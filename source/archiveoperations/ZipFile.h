/***************************************************************************
 * Copyright (C) 2010
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
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef _ZIPFILE_H_
#define _ZIPFILE_H_

#include <vector>
#include <string>
#include <zip/zip.h>
#include <zip/unzip.h>
#include "7ZipFile.h"

class ZipFile
{
    public:
		//!Constructor
        ZipFile(const char *filepath, short mode = ZipFile::OPEN);
		//!Destructor
		~ZipFile();
		//!Get the archive file structure
        ArchiveFileStruct * GetFileStruct(int fileIndx);
		//!Add a file to the zip file
		int AddFile(const char *filepath, const char *destfilepath, int compresslevel = Z_DEFAULT_COMPRESSION, bool RefreshList = true);
        //!Add a directory to the zip file with all of it's content
        int AddDirectory(const char *dirpath, const char *destfilepath, int compresslevel = Z_DEFAULT_COMPRESSION);
		//!Extract a files from a zip file to a path
		int ExtractFile(int ind, const char *dest, bool withpath = false);
		//!Extract all files from a zip file to a directory
		int ExtractAll(const char *dest);
		//!Get the total amount of items inside the archive
        u32 GetItemCount() { return ZipStructure.size(); };
        //!Load/Reload of the full item list in the zip
        bool LoadList();
        //!Enum for opening modes
        enum
        {
            CREATE = 0,
            OPEN,
            APPEND,
        };
    private:
        bool SeekFile(int ind);
        void PathControl();
        void CheckMissingPath(const char * path);
        void ClearList();
        bool SwitchMode(short mode);

        zipFile zFile;
        unzFile uzFile;
        short OpenMode;
        int RealArchiveItemCount;
        std::string ZipFilePath;
        std::vector<ArchiveFileStruct *> ZipStructure;
};

#endif
