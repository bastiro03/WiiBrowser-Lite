/****************************************************************************
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
 * ZipFile Class
 * for WiiXplorer 2010
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>

#include "ZipFile.h"
#include "../fileop.h"
#include "../common.h"

ZipFile::ZipFile(const char *filepath, short mode)
{
    if(!filepath)
        return;

    ZipFilePath = filepath;
    OpenMode = mode;
    zFile = 0;
    uzFile = 0;

    if(OpenMode == ZipFile::OPEN || OpenMode == ZipFile::APPEND)
    {
        uzFile = unzOpen(ZipFilePath.c_str());
        if(uzFile)
            this->LoadList();
    }
}

ZipFile::~ZipFile()
{
    ClearList();
    if(uzFile)
        unzClose(uzFile);
    if(zFile)
        zipClose(zFile, NULL);
}

bool ZipFile::SwitchMode(short mode)
{
    if(mode == ZipFile::OPEN)
    {
        if(zFile)
        {
            zipClose(zFile, NULL);
            zFile = 0;
        }

        if(!uzFile)
            uzFile = unzOpen(ZipFilePath.c_str());

        return (uzFile != 0);
    }
    else if(mode == ZipFile::CREATE || mode == ZipFile::APPEND)
    {
        if(uzFile)
        {
            unzClose(uzFile);
            uzFile = 0;
        }

        if(!zFile)
            zFile = zipOpen(ZipFilePath.c_str(), mode);

        return (zFile != 0);
    }

    return false;
}

void ZipFile::ClearList()
{
    for(u32 i = 0; i < ZipStructure.size(); i++)
    {
        if(ZipStructure[i]->filename)
            delete [] ZipStructure[i]->filename;

        if(ZipStructure[i])
            delete ZipStructure[i];
    }

    ZipStructure.clear();
    std::vector<ArchiveFileStruct *>().swap(ZipStructure);
}

bool ZipFile::LoadList()
{
    ClearList();

    if(!SwitchMode(ZipFile::OPEN))
        return false;

    int ret = unzGoToFirstFile(uzFile);
    if(ret != UNZ_OK)
        return false;

    char filename[1024];
    unz_file_info cur_file_info;
    RealArchiveItemCount = 0;

    do
    {
        if(unzGetCurrentFileInfo(uzFile, &cur_file_info, filename, sizeof(filename), NULL, 0, NULL, 0) == UNZ_OK)
        {
            bool isDir = false;
            if(filename[strlen(filename)-1] == '/')
            {
                isDir = true;
                filename[strlen(filename)-1] = '\0';
            }

            int strlength = strlen(filename)+1;

            ArchiveFileStruct * CurArcFile = new ArchiveFileStruct;
            CurArcFile->filename = new char[strlength];
            strcpy(CurArcFile->filename, filename);
            CurArcFile->length = cur_file_info.uncompressed_size;
            CurArcFile->comp_length = cur_file_info.compressed_size;
            CurArcFile->isdir = isDir;
            CurArcFile->fileindex = RealArchiveItemCount;
            CurArcFile->ModTime = (u64) cur_file_info.dosDate;
            CurArcFile->archiveType = ZIP;

            ZipStructure.push_back(CurArcFile);
        }
        ++RealArchiveItemCount;
    }
    while(unzGoToNextFile(uzFile) == UNZ_OK);

    PathControl();

    return true;
}

ArchiveFileStruct * ZipFile::GetFileStruct(int ind)
{
    if(ind < 0 || ind >= (int) ZipStructure.size())
        return NULL;

    return ZipStructure[ind];
}

bool ZipFile::SeekFile(int ind)
{
    if(ind < 0 || ind >= (int) ZipStructure.size())
        return false;

    if(!SwitchMode(ZipFile::OPEN))
        return false;

    int ret = unzGoToFirstFile(uzFile);
    if(ret != UNZ_OK)
        return false;

    while(ind > 0)
    {
        if(unzGoToNextFile(uzFile) != UNZ_OK)
            return false;

        --ind;
    }

    return true;
}

void ZipFile::CheckMissingPath(const char * path)
{
    if(!path)
        return;

    u32 i = 0;
    for(i = 0; i < ZipStructure.size(); i++)
    {
        if(strcasecmp(ZipStructure[i]->filename, path) == 0)
            break;
    }

    if(i == ZipStructure.size())
    {
        int strlength = strlen(path)+1;
        ArchiveFileStruct * CurArcFile = new ArchiveFileStruct;
        CurArcFile->filename = new char[strlength];
        strcpy(CurArcFile->filename, path);
        CurArcFile->length = 0;
        CurArcFile->comp_length = 0;
        CurArcFile->isdir = true;
        CurArcFile->fileindex = ZipStructure.size();
        CurArcFile->ModTime = 0;
        CurArcFile->archiveType = ZIP;

        ZipStructure.push_back(CurArcFile);
    }
}

void ZipFile::PathControl()
{
    char missingpath[1024];

    for(u32 n = 0; n < ZipStructure.size(); n++)
    {
        const char * filepath = ZipStructure[n]->filename;
        int strlength = strlen(filepath);

        for(int i = 0; i < strlength; i++)
        {
            if(filepath[i] == '/')
                CheckMissingPath(missingpath);

            missingpath[i] = filepath[i];
            missingpath[i+1] = '\0';
        }
    }
}

int ZipFile::AddFile(const char *filepath, const char *destfilepath, int compresslevel, bool RefreshList)
{
    if(!destfilepath)
        return -1;

    if(OpenMode == ZipFile::OPEN)
    {
        if(!SwitchMode(ZipFile::APPEND))
            return -2;
    }
    else if(!SwitchMode(OpenMode))
        return -3;

    zip_fileinfo file_info;
    memset(&file_info, 0, sizeof(zip_fileinfo));

    if(destfilepath[strlen(destfilepath)-1] == '/')
    {
        int ret = zipOpenNewFileInZip(zFile, destfilepath, &file_info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, compresslevel);
        if(ret != ZIP_OK)
            return -4;

        zipCloseFileInZip(zFile);
        return 1;
    }

    if(!filepath)
        return -5;


    struct stat filestat;
    memset(&filestat, 0, sizeof(struct stat));
    stat(filepath, &filestat);

    u64 filesize = filestat.st_size;

    //! Set up the modified time
    struct tm * ModTime = localtime(&filestat.st_mtime);
    file_info.tmz_date.tm_sec = ModTime->tm_sec;
    file_info.tmz_date.tm_min = ModTime->tm_min;
    file_info.tmz_date.tm_hour = ModTime->tm_hour;
    file_info.tmz_date.tm_mday = ModTime->tm_mday;
    file_info.tmz_date.tm_mon = ModTime->tm_mon;
    file_info.tmz_date.tm_year = ModTime->tm_year;

    FILE * sourceFile = fopen(filepath, "rb");
    if(!sourceFile)
        return -6;

    u32 blocksize = 1024*70;
    u8 * buffer = (u8 *) malloc(blocksize);
    if(!buffer)
    {
        fclose(sourceFile);
        return -7;
    }

    int ret = zipOpenNewFileInZip(zFile, destfilepath, &file_info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, compresslevel);
    if(ret != ZIP_OK)
    {
        free(buffer);
        fclose(sourceFile);
        return -8;
    }

	const char * RealFilename = strrchr(destfilepath, '/');
	if(RealFilename)
        RealFilename += 1;
    else
        RealFilename = destfilepath;

    int res = 0;
    u64 done = 0;

    while(done < filesize)
    {
        if(filesize - done < blocksize)
            blocksize = filesize - done;

        ret = fread(buffer, 1, blocksize, sourceFile);
        if(ret <= 0)
            break; //done

        res = zipWriteInFileInZip(zFile, buffer, ret);
        if(res != ZIP_OK)
            break;

        done += ret;
    }

    free(buffer);
    fclose(sourceFile);
    zipCloseFileInZip(zFile);

    if(RefreshList)
        LoadList();

    //! File is now created the next files need to be appended
    OpenMode = ZipFile::APPEND;

    return (done == filesize) ? 1 : -12;
}


int ZipFile::AddDirectory(const char *dirpath, const char *destfilepath, int compresslevel)
{
	if(!dirpath || !destfilepath)
		return -1;

    int ret = 1;
    DIR* dir = NULL;

    dir = opendir(dirpath);
    if(dir == NULL)
        return -1;

    char *filename = (char *) malloc(MAXPATHLEN);
    if(!filename)
    {
        closedir(dir);
        return -2;
    }

    std::vector<std::string> DirList;
    struct dirent *entry;

    while((entry = readdir(dir)))
	{
        if(entry->d_type == DT_DIR)
        {
            if(strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0)
            {
                if(DirList.capacity()-DirList.size() == 0)
                    DirList.reserve(DirList.size()+100);
                DirList.push_back(std::string(entry->d_name));
            }
        }
        else
        {
            std::string newpath(dirpath);
            if(dirpath[strlen(dirpath)-1] != '/')
                newpath += '/';
            newpath += filename;

            std::string newdestpath(destfilepath);
            if(destfilepath[strlen(destfilepath)-1] != '/')
                newdestpath += '/';
            newdestpath += filename;

            ret = AddFile(newpath.c_str(), newdestpath.c_str(), compresslevel, false);
            if(ret < 0)
                break;
        }
	}

	while(!DirList.empty() && !(ret < 0))
	{
        std::string newpath(dirpath);
        if(dirpath[strlen(dirpath)-1] != '/')
            newpath += '/';
		newpath += DirList[0];

        std::string newdestpath(destfilepath);
        if(destfilepath[strlen(destfilepath)-1] != '/')
            newdestpath += '/';
		newdestpath += DirList[0];

        ret = AddDirectory(newpath.c_str(), newdestpath.c_str(), compresslevel);

        DirList.erase(DirList.begin());
	}

	return ret;
}

int ZipFile::ExtractFile(int ind, const char *dest, bool withpath)
{
    if(!SwitchMode(OPEN))
        return -1;

    if(!SeekFile(ind) && ind < RealArchiveItemCount)
        return -1;

    ArchiveFileStruct * CurArcFile = GetFileStruct(ind);

    u32 done = 0;

	char * RealFilename = strrchr(CurArcFile->filename, '/');
	if(RealFilename)
        RealFilename += 1;
    else
        RealFilename = CurArcFile->filename;

	char writepath[1024];
	if(withpath)
        snprintf(writepath, sizeof(writepath), "%s/%s", dest, CurArcFile->filename);
    else
        snprintf(writepath, sizeof(writepath), "%s/%s", dest, RealFilename);

	u32 filesize = CurArcFile->length;

    if(CurArcFile->isdir)
    {
        strncat(writepath, "/", sizeof(writepath));
        CreateSubfolder(writepath);
        return 1;
    }

    int ret = unzOpenCurrentFile(uzFile);

    if(ret != UNZ_OK)
        return -2;

    char * temppath = strdup(writepath);
    char * pointer = strrchr(temppath, '/');
    if(pointer)
    {
        pointer += 1;
        pointer[0] = '\0';
    }

    CreateSubfolder(temppath);

    free(temppath);
    temppath = NULL;

    u32 blocksize = 1024*50;
    void *buffer = malloc(blocksize);

    FILE *pfile = fopen(writepath, "wb");
    if(!pfile)
    {
        unzCloseCurrentFile(uzFile);
        free(buffer);
        fclose(pfile);
        WindowPrompt(("Could not extract file:"), CurArcFile->filename, "OK", NULL);
        return -3;
    }

    do
    {
        if(filesize - done < blocksize)
            blocksize = filesize - done;

        ret = unzReadCurrentFile(uzFile, buffer, blocksize);
        if(ret < 0)
        {
            free(buffer);
            fclose(pfile);
            unzCloseCurrentFile(uzFile);
            return -4;
        }

        fwrite(buffer, 1, blocksize, pfile);

        done += ret;

    } while(done < filesize);

    fclose(pfile);
    unzCloseCurrentFile(uzFile);

    free(buffer);

    return 1;
}

int ZipFile::ExtractAll(const char *dest)
{
    if(!SwitchMode(OPEN))
        return -1;

    bool Stop = false;

    u32 blocksize = 1024*70;
    void *buffer = malloc(blocksize);

    if(!buffer)
        return -5;

    char writepath[1024];
    char filename[1024];
    memset(writepath, 0, sizeof(writepath));
    memset(filename, 0, sizeof(filename));

    unz_file_info cur_file_info;

    int ret = unzGoToFirstFile(uzFile);
    if(ret != UNZ_OK)
    {
        free(buffer);
        return -6;
    }

    while(!Stop)
    {
        if(unzGetCurrentFileInfo(uzFile, &cur_file_info, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK)
        {
            Stop = true;
        }

        if(!Stop && filename[strlen(filename)-1] != '/')
        {
            u64 uncompressed_size = cur_file_info.uncompressed_size;

            u64 done = 0;
            char *pointer = NULL;

            ret = unzOpenCurrentFile(uzFile);

            snprintf(writepath, sizeof(writepath), "%s/%s", dest, filename);

            pointer = strrchr(writepath, '/');
            int position = pointer-writepath+2;

            char temppath[strlen(writepath)];
            snprintf(temppath, position, "%s", writepath);

            CreateSubfolder(temppath);

            if(ret == UNZ_OK)
            {
                FILE *pfile = fopen(writepath, "wb");
                if(!pfile)
                {
                    free(buffer);
                    fclose(pfile);
                    unzCloseCurrentFile(uzFile);
                    return -8;
                }

                do
                {
                    if(uncompressed_size - done < blocksize)
                        blocksize = uncompressed_size - done;

                    ret = unzReadCurrentFile(uzFile, buffer, blocksize);

                    if(ret == 0)
                        break;

                    fwrite(buffer, 1, blocksize, pfile);

                    done += ret;

                } while(done < uncompressed_size);

                fclose(pfile);
                unzCloseCurrentFile(uzFile);
            }
        }
        if(unzGoToNextFile(uzFile) != UNZ_OK)
        {
            Stop = true;
        }
    }

    free(buffer);
    buffer = NULL;

    return 1;
}
