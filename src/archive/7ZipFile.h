/***************************************************************************
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
 * 7ZipFile.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef _7ZIPFILE_H_
#define _7ZIPFILE_H_

#include <gctypes.h>

extern "C" {
#include <sevenzip/7zCrc.h>
#include <sevenzip/7zFile.h>
#include <sevenzip/7zIn.h>
#include <sevenzip/7zExtract.h>
#include <sevenzip/7zAlloc.h>
#include <sevenzip/7zHeader.h>
}

typedef struct
{
	char * filename; // full filename
	size_t length; // uncompressed file length in 64 bytes for sizes higher than 4GB
	size_t comp_length; // compressed file length in 64 bytes for sizes higher than 4GB
	bool isdir; // 0 - file, 1 - directory
	u32 fileindex; // fileindex number
	u64 ModTime; // modification time
	u8 archiveType; // modification time
} ArchiveFileStruct;

enum
{
    UNKNOWN = 1,
    ZIP,
    SZIP,
    RAR,
    U8Arch,
    ArcArch
};


class SzFile
{
    public:
		//!Constructor
        SzFile(const char *filepath);
		//!Destructor
		~SzFile();
		//!Check if it is a 7zip file
        bool Is7ZipFile (const char *buffer);
		//!Get the archive file structure
        ArchiveFileStruct * GetFileStruct(int fileIndx);
		//!Extract file from a 7zip to file
        int ExtractFile(int fileindex, const char * outpath, bool withpath = false);
		//!Extract all files from the 7zip to a path
        int ExtractAll(const char * destpath);
		//!Get the total amount of items inside the archive
        u32 GetItemCount();

    private:
        void DisplayError(SRes res);

        ArchiveFileStruct CurArcFile;
        SRes SzResult;
        CFileInStream archiveStream;
        CLookToRead lookStream;
        CSzArEx SzArchiveDb;
        ISzAlloc SzAllocImp;
        ISzAlloc SzAllocTempImp;
        UInt32 SzBlockIndex;
        CSzFileItem * SzFileItem;
};

#endif
