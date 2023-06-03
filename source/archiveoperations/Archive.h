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
 * Archive Class
 *
 * Virtual Class to inherit the Archives from and enable
 *
 * for WiiXplorer 2009
 ***************************************************************************/
#ifndef ARCHIVE_H_
#define ARCHIVE_H_

#include <gctypes.h>

#include "7ZipFile.h"
#include "ZipFile.h"
#include "RarFile.h"

class ArchiveHandle
{
public:
	//!Constructor
	ArchiveHandle(const char* filepath);
	//!Destructor
	~ArchiveHandle();
	//!Get the archive file structure
	ArchiveFileStruct* GetFileStruct(int fileIndx);
	//!Extract a files from a zip file to a path
	int ExtractFile(int ind, const char* destpath, bool withpath = false);
	//!Extract all files from a zip file to a directory
	int ExtractAll(const char* destpath);
	//!Reload archive list
	bool ReloadList();
	//!Get the total amount of items inside the archive
	u32 GetItemCount();

private:
	//!Check what kind of archive it is
	bool IsZipFile(const char* buffer);
	bool Is7ZipFile(const char* buffer);
	bool IsRarFile(const char* buffer);

	SzFile* szFile;
	ZipFile* zipFile;
	RarFile* rarFile;
};

#endif //ARCHIVE_BROWSER_H_
