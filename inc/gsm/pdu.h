/*
 * pdu.h
 *
 *  Created on: 2018年1月10日
 *      Author: tykj
 */

#ifndef PDU_H_
#define PDU_H_

#include "general.h"

extern int PDU_DecodeMessage(const char* pSrc, PDU_PARAM* pDst);
extern int PDU_String2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);
extern int PDU_SerializeNumbers(const char* pSrc, char* pDst, int nSrcLength);
extern  const char GSM_M3M_CENTER1[];

#endif /* PDU_H_ */
