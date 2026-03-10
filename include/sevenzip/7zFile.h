/* 7zFile.h -- File IO
2008-11-23 : Igor Pavlov : Public domain */

#ifndef __7Z_FILE_H
#define __7Z_FILE_H

#include <stdio.h>
#include "7zTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  FILE *file;
} CSzFile;

void File_Construct(CSzFile *p);
WRes InFile_Open(CSzFile *p, const char *name);
WRes OutFile_Open(CSzFile *p, const char *name);
WRes File_Close(CSzFile *p);
WRes File_Read(CSzFile *p, void *data, size_t *size);
WRes File_Write(CSzFile *p, const void *data, size_t *size);
WRes File_Seek(CSzFile *p, Int64 *pos, ESzSeek origin);
WRes File_GetLength(CSzFile *p, UInt64 *length);

typedef struct
{
  ISeqInStream s;
  CSzFile file;
} CFileSeqInStream;

void FileSeqInStream_CreateVTable(CFileSeqInStream *p);

typedef struct
{
  ISeekInStream s;
  CSzFile file;
} CFileInStream;

void FileInStream_CreateVTable(CFileInStream *p);

typedef struct
{
  ISeqOutStream s;
  CSzFile file;
} CFileOutStream;

void FileOutStream_CreateVTable(CFileOutStream *p);

#ifdef __cplusplus
}
#endif

#endif
