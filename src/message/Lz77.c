/*
 * Lz77.c
 *
 *  Created on: 2017��10��9��
 *      Author: guo
 */
#include "Lz77.h"



////////////////////////////////////////////////////////////////////////////////
void Write1ToBitStream(u8 *  pBuffer, ulong   ulBitOffset)
{
   ulong   ulByteBoundary;
   ulong   ulOffsetInByte;
   ulByteBoundary = ulBitOffset>>3 ;
   ulOffsetInByte = ulBitOffset&7;
   *(pBuffer+ulByteBoundary) |= (1<<ulOffsetInByte);
}
void Write0ToBitStream(u8 *  pBuffer,ulong   ulBitOffset)
{
   ulong   ulByteBoundary;
   ulong   ulOffsetInByte;
   ulByteBoundary = ulBitOffset>>3 ;
   ulOffsetInByte = ulBitOffset&7;
   *(pBuffer+ulByteBoundary) &= (~(1<<ulOffsetInByte));
}
ulong ReadBitFromBitStream(u8 *  pBuffer,ulong   ulBitOffset)
{
   ulong   ulByteBoundary;
   ulong   ulOffsetInByte;
   ulByteBoundary = ulBitOffset>>3 ;
   ulOffsetInByte = ulBitOffset&7;
   return ((*(ulong *)(pBuffer+ulByteBoundary))>>ulOffsetInByte)&1 ;
}

ulong WriteGolombCode(ulong   x,u8 *  pBuffer,ulong   ulBitOffset)
{
   ulong           q, r;
   int             i;
   q = (x-1)>>m;
   r = x-(q<<m)-1;
   for(i=0; (ulong)i<q; i++, ulBitOffset++)
   {
       Write1ToBitStream(pBuffer, ulBitOffset);
   }
   Write0ToBitStream(pBuffer, ulBitOffset);
   ulBitOffset++;
   for(i=0; i<m; i++, ulBitOffset++)
   {
       if( (r>>i)&1 )
       {
           Write1ToBitStream(pBuffer, ulBitOffset);
       }
       else
       {
           Write0ToBitStream(pBuffer, ulBitOffset);
       }
   }
   return m+q+1;
}

ulong ReadGolombCode(ulong *  pulCodingLength,u8 *  pBuffer,ulong   ulBitOffset)
{
   ulong   q, r;
   ulong   bit;
   int i;
   for(q=0; ;q++)
   {
       bit = (ulong)ReadBitFromBitStream(pBuffer, ulBitOffset);
       ulBitOffset++;
       if( !bit )
       {
           break;
       }
   }

   for(i=0, r=0; (ulong)i<m; i++, ulBitOffset++)
   {
       bit = (ulong)ReadBitFromBitStream(pBuffer, ulBitOffset);
       bit <<= i;
       r |= bit;
   }
   *pulCodingLength = m + q + 1;
   return r+(q<<m)+1;
}

ulong CompareStrings(u8 *  string1,u8 *  string2,ulong   length)
{
   ulong       i;
   u8 * p1;
   u8 *p2;
   p1 = string1;
   p2 = string2;
   for(i=0; i<length; i++)
   {
       if( *p1==*p2 )
       {
           p1++;
           p2++;
       }
       else
       {
           break;
       }
   }
   return p1-string1;
}

void FindLongestSubstring(u8 * pSourceString,u8 * pString,ulong ulSourceStringLength,
		ulong * pulSubstringOffset,ulong * pulSubstringLength)
{
   u8 *  pSrc;
   ulong   offset, length;
   ulong   ulMaxLength;

   *pulSubstringOffset = offset = 0;
   *pulSubstringLength = 0;
   if( NULL==pSourceString || NULL==pString )
   {
       return;
   }
   ulMaxLength = ulSourceStringLength;
   pSrc = pSourceString;
   while( ulMaxLength>0 )
   {
       length = CompareStrings(pSrc, pString, ulMaxLength);
       if( length>*pulSubstringLength )
       {
           *pulSubstringLength = length;
           *pulSubstringOffset = offset;
       }
       pSrc++;
       offset++;
       ulMaxLength--;
   }
}

void WriteBits(u8 *  pDataBuffer,ulong ulOffsetToWrite,ulong ulBits,ulong ulBitLength)
{
   ulong   ulDwordsOffset;
   ulong   ulBitsOffset, ulBitsRemained;
   ulDwordsOffset = ulOffsetToWrite>>5;
   ulBitsOffset = ulOffsetToWrite&31;
   ulBitsRemained = 32 - ulBitsOffset;
   if( 0==ulBitsOffset )
   {
       *((ulong *)pDataBuffer+ulDwordsOffset) = ulBits;
   }
   else if( ulBitsRemained>=ulBitLength )
   {
       *((ulong *)pDataBuffer+ulDwordsOffset) |= (ulBits<<ulBitsOffset);
   }
   else
   {
       *((ulong *)pDataBuffer+ulDwordsOffset) |= (ulBits<<ulBitsOffset);
       *((ulong *)pDataBuffer+ulDwordsOffset+1) = ulBits>>ulBitsRemained;
   }
}
void ReadBits(u8 *  pDataBuffer,ulong ulOffsetToRead,ulong * pulBits)
{
   ulong   ulDwordsOffset;
   ulong   ulBitsOffset, ulBitsLength;
   ulDwordsOffset = ulOffsetToRead>>5;
   ulBitsOffset = ulOffsetToRead&31;
   ulBitsLength = 32 - ulBitsOffset;

   *pulBits = *((ulong *)pDataBuffer+ulDwordsOffset);
   if( 0!=ulBitsOffset )
   {
       (*pulBits) >>= ulBitsOffset;
       (*pulBits) |= (*((ulong *)pDataBuffer+ulDwordsOffset+1))<<ulBitsLength;
   }
}
void lz77compress(u8 * pDataBuffer,ulong ulDataLength,u8 * pOutputBuffer,ulong * pulNumberOfBits)
{
   long        iSlideWindowPtr;
   ulong       ulBytesCoded;
   ulong       ulMaxlength;
   u8 *      pSlideWindowPtr;
   u8 *      pUnprocessedDataPtr;
   ulong   offset;
   ulong   length;
   ulong   ulCodingLength;
   ulong   ulBitOffset;
   u8   cc;
   int     i;
   iSlideWindowPtr = -MAX_WND_SIZE;
   pSlideWindowPtr = NULL;
   ulBitOffset = 0;
   ulBytesCoded = 0;
   while( ulBytesCoded<ulDataLength )
   {
       if( iSlideWindowPtr>=0 )
       {
           pSlideWindowPtr = pDataBuffer+iSlideWindowPtr;
           ulMaxlength = MAX_WND_SIZE;
       }
       else if( iSlideWindowPtr>=-MAX_WND_SIZE )
       {
           pSlideWindowPtr = pDataBuffer;
           ulMaxlength = MAX_WND_SIZE + iSlideWindowPtr;
       }
       else
       {
           pSlideWindowPtr = NULL;
           ulMaxlength = 0;
       }
       pUnprocessedDataPtr = pDataBuffer + ulBytesCoded;
       if( ulMaxlength>ulDataLength-ulBytesCoded )
       {
           ulMaxlength = ulDataLength-ulBytesCoded;
       }
       FindLongestSubstring(
           pSlideWindowPtr,
           pUnprocessedDataPtr,
           ulMaxlength,
           &offset,
           &length
           );
       assert( length<=MAX_WND_SIZE );
       assert( offset<MAX_WND_SIZE );
       if(length>1)
       {
           Write1ToBitStream(pOutputBuffer, ulBitOffset);
           ulBitOffset++;
           for(i=0; i<OFFSET_CODING_LENGTH; i++, ulBitOffset++)
           {
               if( (offset>>i)&1 )
               {
                   Write1ToBitStream(pOutputBuffer, ulBitOffset);
               }
               else
               {
                   Write0ToBitStream(pOutputBuffer, ulBitOffset);
               }
           }
           ulCodingLength = WriteGolombCode(length, pOutputBuffer, ulBitOffset);
           ulBitOffset += ulCodingLength;
           iSlideWindowPtr += length;
           ulBytesCoded += length;
       }
       else
       {
           Write0ToBitStream(pOutputBuffer, ulBitOffset);
           ulBitOffset++;
           cc = (*pUnprocessedDataPtr);
           for(i=0; i<8; i++, ulBitOffset++)
           {
               if( (cc>>i)&1 )
               {
                   Write1ToBitStream(pOutputBuffer, ulBitOffset);
               }
               else
               {
                   Write0ToBitStream(pOutputBuffer, ulBitOffset);
               }
           }
           iSlideWindowPtr++;
           ulBytesCoded++;
       }
   }
   if( ulBytesCoded!=ulDataLength )
   {
       assert(ulBytesCoded==ulDataLength);
   }
   *pulNumberOfBits = ulBitOffset;
}

void lz77decompress(u8 * pDataBuffer,ulong ulNumberOfBits,u8 * pOutputBuffer,ulong * pulNumberOfBytes)
{
   long        iSlideWindowPtr;
   u8 *      pSlideWindowPtr;
   ulong   length, offset;
   ulong   bit;
   u8   cc;
   int     i;
   ulong   ulBytesDecoded;
   ulong   ulBitOffset;
   ulong   ulCodingLength;
   u8 *  pWrite;
   iSlideWindowPtr = -MAX_WND_SIZE;
   pWrite = (u8 *)pOutputBuffer;
   ulBitOffset = 0;
   ulBytesDecoded = 0;

   while( ulBitOffset<ulNumberOfBits )
   {
       bit = ReadBitFromBitStream(pDataBuffer, ulBitOffset);
       ulBitOffset++;
       if( bit )
       {
           if( iSlideWindowPtr>=0 )
           {
               pSlideWindowPtr = pOutputBuffer + iSlideWindowPtr;
           }
           else if( iSlideWindowPtr>=-MAX_WND_SIZE )
           {
               pSlideWindowPtr = pOutputBuffer;
           }
           else
           {
               pSlideWindowPtr = NULL;
           }

           for(i=0, offset=0; i<OFFSET_CODING_LENGTH; i++, ulBitOffset++)
           {
               bit = ReadBitFromBitStream(pDataBuffer, ulBitOffset);
               offset |= (bit<<i);
           }
           length= ReadGolombCode(&ulCodingLength, pDataBuffer, ulBitOffset);
           assert(offset<MAX_WND_SIZE);
           if( length>MAX_WND_SIZE )
           {
               assert(length<=MAX_WND_SIZE);
           }
           ulBitOffset += ulCodingLength;
           memcpy(pWrite, pSlideWindowPtr+offset, length);
           pWrite+=length;
           iSlideWindowPtr+=length;
           ulBytesDecoded+=length;
       }
       else
       {
           for(i=0, cc=0; i<8 ; i++, ulBitOffset++)
           {
               bit = ReadBitFromBitStream(pDataBuffer, ulBitOffset);
               cc |= ((u8)bit<<i);
           }
           *pWrite++ = cc;
           iSlideWindowPtr++;
           ulBytesDecoded++;
       }
   }
   *pulNumberOfBytes = ulBytesDecoded;
}


