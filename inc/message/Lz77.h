/*
 * Lz77.h
 *
 *  Created on: 2017��10��9��
 *      Author: ada
 */

#ifndef LZ77_H_
#define LZ77_H_

#include <assert.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "general.h"

#define OFFSET_CODING_LENGTH    (10)
#define MAX_WND_SIZE            1024
#define OFFSET_MASK_CODE        (MAX_WND_SIZE-1)
static  const ulong     m=3;

void Write1ToBitStream(u8 *  pBuffer, ulong   ulBitOffset);
void Write0ToBitStream(u8 *  pBuffer,ulong   ulBitOffset);
ulong ReadBitFromBitStream(u8 *  pBuffer,ulong   ulBitOffset);
ulong WriteGolombCode(ulong   x,u8 *  pBuffer,ulong   ulBitOffset);
ulong ReadGolombCode(ulong *  pulCodingLength,u8 *  pBuffer,ulong   ulBitOffset);
ulong CompareStrings(u8 *  string1,u8 *  string2,ulong   length);
void FindLongestSubstring(u8 * pSourceString,u8 * pString,ulong ulSourceStringLength,ulong * pulSubstringOffset,ulong * pulSubstringLength);
void WriteBits(u8 *  pDataBuffer,ulong ulOffsetToWrite,ulong ulBits,ulong ulBitLength);
void ReadBits(u8 *  pDataBuffer,ulong ulOffsetToRead,ulong * pulBits);
extern void lz77compress(u8 * pDataBuffer,ulong ulDataLength,u8 * pOutputBuffer,ulong * pulNumberOfBits);
extern void lz77decompress(u8 * pDataBuffer,ulong ulNumberOfBits,u8 * pOutputBuffer,ulong * pulNumberOfBytes);

#endif /* LZ77_H_ */
