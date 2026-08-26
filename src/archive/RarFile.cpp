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
 * RarFile.cpp
 *
 * RarFile Class
 * for WiiXplorer 2009
 *
 * NOTE: RAR support is currently disabled.  The UnRAR SDK bundled with
 * this project (external/portlibs) does not match the header set that
 * the original implementation was written against, and the source for
 * the matching UnRAR version is no longer bundled with the project.
 * To restore RAR extraction, vendor a self-consistent UnRAR 3.x SDK
 * (headers + library) and replace this file with the original
 * implementation.
 ***************************************************************************/
#include <stdio.h>
#include <string.h>

#include "RarFile.h"

RarFile::RarFile(const char *filepath)
{
    (void)filepath;
}

RarFile::~RarFile()
{
    ClearList();
}

void RarFile::ClearList()
{
    for(u32 i = 0; i < RarStructure.size(); i++)
    {
        if(RarStructure.at(i)->filename != NULL)
        {
            delete [] RarStructure.at(i)->filename;
            RarStructure.at(i)->filename = NULL;
        }
        if(RarStructure.at(i) != NULL)
        {
            delete RarStructure.at(i);
            RarStructure.at(i) = NULL;
        }
    }

    RarStructure.clear();
}

ArchiveFileStruct * RarFile::GetFileStruct(int ind)
{
    if(ind < 0 || ind >= (int) RarStructure.size())
        return NULL;

    return RarStructure.at(ind);
}

u32 RarFile::GetItemCount()
{
    return RarStructure.size();
}

int RarFile::ExtractFile(int fileindex, const char * outpath, bool withpath)
{
    (void)fileindex;
    (void)outpath;
    (void)withpath;
    return -1;
}

int RarFile::ExtractAll(const char * destpath)
{
    (void)destpath;
    return 1;
}