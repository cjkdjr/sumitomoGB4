/*
 * pdu.c
 *
 *  Created on: 2018年1月10日
 *      Author: tykj
 */

#include <string.h>

#include "pdu.h"
#include "sms.h"
#include "general.h"
/*-----------------------------------------------------
Name    :PDU_Decode8bit()
Funciton:8-bit decode
Input   :*pSrc
		 *pDst
		 *nSrcLength
Output  :nDst
Author  :lmm-2014/02/10
Modify  :[<name>-<data>]
------------------------------------------------------*/

int PDU_Decode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
     memcpy(pDst, pSrc, nSrcLength);
 	 //*pDst = '\0';
    return nSrcLength;
}

/*-----------------------------------------------------
Name    :PDU_String2Bytes()
Funciton:"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
Input   :*pSrc
		 *pDst
		 *nSrcLength
Output  :nDst
Author  :lmm-2014/02/10
Modify  :[<name>-<data>]
------------------------------------------------------*/
int PDU_String2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int i=0;
    for(i=0; i<nSrcLength; i+=2)
    {
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }
        pSrc++;
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
            *pDst |= *pSrc - 'A' + 10;
        }
        pSrc++;
        pDst++;
    }
    return (nSrcLength+1)/2;
}

/*-----------------------------------------------------
Name    :PDU_SerializeNumbers()
Funciton:converts :"683158812764F8" --> "8613851872468"
Input   :*pSrc:
	 *pDst:
	 *nSrcLength:
Output  :none
Author  :lmm-2014/02/10
Modify  :[<name>-<data>]
------------------------------------------------------*/
int PDU_SerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;
    char ch;
    int i=0;
    nDstLength = nSrcLength;

    for(i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;
        *pDst++ = *pSrc++;
        *pDst++ = ch;
    }

    if(*(pDst-1) == 'F')
    {
        pDst--;
        nDstLength--;
    }

    *pDst = '\0';

    return nDstLength;
}

/*-----------------------------------------------------
Name    :PDU_DecodeMessage
Funciton:string pdu decode
Input   :*pSrc:
	 	 SMS_PARAM*pDst:
	 	 *nSrcLength:
Output  :nDstLength
Author  :lmm-2014/02/10
Modify  :[<name>-<data>]
------------------------------------------------------*/
int PDU_DecodeMessage(const char* pSrc, PDU_PARAM* pDst)
{
    int nDstLength = 0;
    unsigned char tmp;
    unsigned char buf[256];
//    unsigned char buf_liantong_num[32];
//    unsigned char *buf_liantong_num_pin;
//    unsigned char buf_liantong[256];

    /*SMSC address message */
    PDU_String2Bytes(pSrc, &tmp, 2);
    tmp = (tmp - 1) * 2;
    pSrc += 4;
    PDU_SerializeNumbers(pSrc, pDst->SCA, tmp);
    pSrc += tmp;
    /*TP-DA basic paramater*/
    PDU_String2Bytes(pSrc, &tmp, 2);
    pSrc += 2;
    /*Get target number*/
    PDU_String2Bytes(pSrc, &tmp, 2); /*target number len*/
    if(tmp & 1) tmp += 1;            /*judge odd or even*/
    pSrc += 4;
    printf_gsm("pDst->TPA len =%d\n",tmp);
    PDU_SerializeNumbers(pSrc, pDst->TPA, tmp);/*get target number*/
    printf_gsm("pDst->TPA=%s\n",pDst->TPA);
    pSrc += tmp;

    /*TPDU*/
    PDU_String2Bytes(pSrc, (unsigned char*)&pDst->TP_PID, 2);/*TP-PID*/
    pSrc += 2;
    PDU_String2Bytes(pSrc, (unsigned char*)&pDst->TP_DCS, 2); /*TP-DCS*/
    pSrc += 2;
    PDU_SerializeNumbers(pSrc, pDst->TP_SCTS, 14);/*timap*/

#if 0
    if (0 == memcmp(pDst->TPA, GSM_M3M_CENTER1,strlen(GSM_M3M_CENTER1)))
    {
       pSrc += 16;
       api_AscToHex(pSrc, buf_liantong_num, 8);/*message len*/
//      DBG0_PR("buf_liantong_num=%s\n",buf_liantong_num);

     //  buf_liantong_num=buf_liantong_num +2;
       buf_liantong_num_pin = buf_liantong_num;
       buf_liantong_num_pin+= 2;
       PDU_String2Bytes(buf_liantong_num_pin, &tmp, 2);/*message len*/
       tmp=tmp+2;//       加上校验码和尾码
//       DBG0_PR("temp=%d\n",tmp);
       //pSrc += 4;
   //    if(pDst->TP_DCS == 0xF5)
   //    {
   //        nDstLength = PDU_String2Bytes(pSrc, buf, tmp * 2);
   //        nDstLength = PDU_Decode8bit(buf, pDst->TP_UD, nDstLength);
   //
   //    }

       nDstLength = api_AscToHex(pSrc, buf_liantong, tmp * 4);
       nDstLength = api_AscToHex(buf_liantong, buf, tmp*2);
       nDstLength = PDU_Decode8bit(buf, pDst->TP_UD, nDstLength);

       return nDstLength;

    }
#endif
    pSrc += 14;
    PDU_String2Bytes(pSrc, &tmp, 2);/*message len*/
    printf_gsm("temp=%d\n",tmp);
    pSrc += 2;
//    if(pDst->TP_DCS == 0xF5)
//    {
//        nDstLength = PDU_String2Bytes(pSrc, buf, tmp * 2);
//        nDstLength = PDU_Decode8bit(buf, pDst->TP_UD, nDstLength);
//
//    }
    nDstLength = PDU_String2Bytes(pSrc, buf, tmp * 2);
    nDstLength = PDU_Decode8bit(buf, pDst->TP_UD, nDstLength);

    return nDstLength;
}

