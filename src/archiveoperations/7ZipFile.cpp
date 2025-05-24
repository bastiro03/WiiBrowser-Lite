/****************************************************************************
 * Copyright (C) 2009
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * ZipFile.cpp
 *
 * ZipFile Class
 * for WiiXplorer 2009
 ***************************************************************************/
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>

#include "7ZipFile.h"
#include "../common.h"
#include "../fileop.h"

#define MAXPATHLEN 512

// 7zip error list
static const char* szerrormsg[10] = {
	("File is corrupt."), // 7z: Data error
	("Not enough memory."), // 7z: Out of memory
	("File is corrupt (CRC mismatch)."), // 7z: CRC Error
	("File uses unsupported compression settings."), // 7z: Not implemented
	("File is corrupt."), // 7z: Fail
	("Failed to read file data."), // 7z: Data read failure
	("File is corrupt."), // 7z: Archive error
	("File uses too high of compression settings (dictionary size is too large)."), // 7z: Dictionary too large
	("Can't open file."),
	("Process canceled."),
};

SzFile::SzFile(const char* filepath)
{
	if (InFile_Open(&archiveStream.file, filepath))
	{
		SzResult = 9;
		return;
	}

	FileInStream_CreateVTable(&archiveStream);
	LookToRead_CreateVTable(&lookStream, False);

	lookStream.realStream = &archiveStream.s;
	LookToRead_Init(&lookStream);

	// set default 7Zip SDK handlers for allocation and freeing memory
	SzAllocImp.Alloc = SzAlloc;
	SzAllocImp.Free = SzFree;
	SzAllocTempImp.Alloc = SzAllocTemp;
	SzAllocTempImp.Free = SzFreeTemp;

	// prepare CRC and 7Zip database structures
	CrcGenerateTable();
	SzArEx_Init(&SzArchiveDb);

	// open the archive
	SzResult = SzArEx_Open(&SzArchiveDb, &lookStream.s, &SzAllocImp, &SzAllocTempImp);

	if (SzResult != SZ_OK)
	{
		DisplayError(SzResult);
	}
}

SzFile::~SzFile()
{
	SzArEx_Free(&SzArchiveDb, &SzAllocImp);

	File_Close(&archiveStream.file);
}

bool SzFile::Is7ZipFile(const char* buffer)
{
	unsigned int* check;
	check = (unsigned int*)buffer;

	// 7z signature
	int i;
	for (i = 0; i < 6; i++)
		if (buffer[i] != k7zSignature[i])
			return false;

	return true; // 7z archive found
}

ArchiveFileStruct* SzFile::GetFileStruct(int ind)
{
	if (ind > static_cast<int>(SzArchiveDb.db.NumFiles) || ind < 0)
		return nullptr;

	CSzFileItem* SzFileItem = SzArchiveDb.db.Files + ind;

	CurArcFile.filename = SzFileItem->Name;
	CurArcFile.length = SzFileItem->Size;
	CurArcFile.comp_length = 0;
	CurArcFile.isdir = SzFileItem->IsDir;
	CurArcFile.fileindex = ind;
	if (SzFileItem->MTimeDefined)
		CurArcFile.ModTime = (u64)(SzFileItem->MTime.Low | (static_cast<u64>(SzFileItem->MTime.High) << 32));
	else
		CurArcFile.ModTime = 0;
	CurArcFile.archiveType = SZIP;

	return &CurArcFile;
}

void SzFile::DisplayError(SRes res)
{
	WindowPrompt(("7z decompression failed:"), szerrormsg[(res - 1)], ("OK"), nullptr);
}

u32 SzFile::GetItemCount()
{
	if (SzResult != SZ_OK)
		return 0;

	return SzArchiveDb.db.NumFiles;
}

int SzFile::ExtractFile(int fileindex, const char* outpath, bool withpath)
{
	if (SzResult != SZ_OK)
		return -1;

	if (!GetFileStruct(fileindex))
		return -2;

	// reset variables
	UInt32 SzBlockIndex = 0xFFFFFFFF;
	size_t SzOffset = 0;
	size_t SzSizeProcessed = 0;
	Byte* outBuffer = nullptr;
	size_t outBufferSize = 0;

	char* RealFilename = strrchr(CurArcFile.filename, '/');

	char writepath[MAXPATHLEN];
	if (!RealFilename || withpath)
		snprintf(writepath, sizeof(writepath), "%s/%s", outpath, CurArcFile.filename);
	else
		snprintf(writepath, sizeof(writepath), "%s/%s", outpath, RealFilename + 1);

	if (CurArcFile.isdir)
	{
		strncat(writepath, "/", sizeof(writepath));
		CreateSubfolder(writepath);
		return 1;
	}

	char* temppath = strdup(writepath);
	char* pointer = strrchr(temppath, '/');
	if (pointer)
	{
		pointer += 1;
		pointer[0] = '\0';
	}
	CreateSubfolder(temppath);

	free(temppath);
	temppath = nullptr;

	// Extract
	SzResult = SzAr_Extract(&SzArchiveDb, &lookStream.s, fileindex,
	                        &SzBlockIndex, &outBuffer, &outBufferSize,
	                        &SzOffset, &SzSizeProcessed,
	                        &SzAllocImp, &SzAllocTempImp);

	if (SzResult == SZ_OK)
	{
		FILE* wFile = fopen(writepath, "wb");

		//not quite right and needs to be changed
		u32 done = 0;
		if (!wFile)
			done = CurArcFile.length;

		do
		{
			int wrote = fwrite(outBuffer, 1, 51200, wFile);
			done += wrote;

			if (wrote == 0)
				break;
		}
		while (done < CurArcFile.length);

		fclose(wFile);
	}
	if (outBuffer)
	{
		IAlloc_Free(&SzAllocImp, outBuffer);
		outBuffer = NULL;
	}

	// check for errors
	if (SzResult != SZ_OK)
	{
		// display error message
		DisplayError(SzResult);
		return -4;
	}

	return 1;
}

int SzFile::ExtractAll(const char* destpath)
{
	if (!destpath)
		return -5;

	for (u32 i = 0; i < SzArchiveDb.db.NumFiles; i++)
	{
		CSzFileItem* SzFileItem = SzArchiveDb.db.Files + i;

		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/%s", destpath, SzFileItem->Name);
		u32 filesize = SzFileItem->Size;

		if (!filesize)
			continue;

		char* pointer = strrchr(path, '/') + 1;

		if (pointer)
		{
			//cut off filename
			pointer[0] = '\0';
		}
		else
			continue; //shouldn't ever happen but to be sure, skip the file if it does

		CreateSubfolder(path);

		ExtractFile(i, path);
	}

	return 1;
}
