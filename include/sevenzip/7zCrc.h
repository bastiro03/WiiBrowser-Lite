/* 7zCrc.h -- CRC32 calculation
2008-11-23 : Igor Pavlov : Public domain */

#ifndef __7Z_CRC_H
#define __7Z_CRC_H

#include "7zTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

extern UInt32 g_CrcTable[];

void CrcGenerateTable(void);

#define CRC_INIT_VAL 0xFFFFFFFF
#define CRC_GET_DIGEST(crc) ((crc) ^ CRC_INIT_VAL)
#define CRC_UPDATE_BYTE(crc, b) (g_CrcTable[((crc) ^ (b)) & 0xFF] ^ ((crc) >> 8))

UInt32 CrcUpdate(UInt32 crc, const void *data, size_t size);
UInt32 CrcCalc(const void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
