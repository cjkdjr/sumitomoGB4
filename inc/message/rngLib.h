/* rngLib.h - ring buffer subroutine library header */

/*
modification history
--------------------
^
^
|
|
01a,28jan16,sgt  created.
*/

#ifndef __RNGLIB_H
#define __RNGLIB_H

//#include "kernel\port.h"
	
/* HIDDEN */

/* typedefs */

typedef struct	 /* RING - ring buffer */
	{
    int pToBuf;	 /* offset from start of buffer where to write next */
    int pFromBuf;	/* offset from start of buffer where to read next */
    unsigned long bufSize;	/* size of ring in bytes */
    char *buf;	 /* pointer to start of buffer */
	} RING;

/* END_HIDDEN */
		
typedef RING *RING_ID;
	
		
/*
 * The following macros are designed to do various operations on
 * the RING object.  By using them, users can avoid having to know
 * the structure of RING.  However they are intended to be very
 * efficient and this is why they are macros in the first place.
 * In general the parameters to them should be register variables
 * for maximum efficiency.
 */

/*******************************************************************************
*
* RNG_ELEM_GET - get one character from a ring buffer
*
* This macro gets a single character from the specified ring buffer.
* Must supply temporary variable (register int) 'fromP'.
*
* RETURNS: 1 if there was a char in the buffer to return, 0 otherwise
*
* NOMANUAL
*/
#define RNG_ELEM_GET(ringId, pCh, fromP)	\
(						\
		fromP = (ringId)->pFromBuf,						\
		((ringId)->pToBuf == fromP) ?					\
		0				\
		:				\
		(				\
				*pCh = (ringId)->buf[fromP],			\
				(ringId)->pFromBuf = ((++fromP == (ringId)->bufSize) ? 0 : fromP), 			\
				1																	\
		)				\
)
/*******************************************************************************
*
* RNG_ELEM_PUT - put one character into a ring buffer
*
* This macro puts a single character into the specified ring buffer.
* Must supply temporary variable (register int) 'toP'.
*
* RETURNS: 1 if there was room in the buffer for the char, 0 otherwise
*
* NOMANUAL
*/
#define RNG_ELEM_PUT(ringId, ch, toP)		\
(						\
    toP = (ringId)->pToBuf,							\
    (toP == (ringId)->pFromBuf - 1) ?		\
		0 			\
    :				\
		(				\
				(toP == (ringId)->bufSize - 1) ?			\
				(							\
						((ringId)->pFromBuf == 0) ?				\
						0					\
						:					\
						(					\
								(ringId)->buf[toP] = ch,			\
								(ringId)->pToBuf = 0,					\
								1			\
						)					\
				)							\
				:							\
				(							\
						(ringId)->buf[toP] = ch,					\
						(ringId)->pToBuf++,								\
						1					\
				)							\
		)									\
)
/*********************************************************************************************************
  function declarations	函数声明
*********************************************************************************************************/

extern int 	rngIsEmpty (RING_ID ringId);
extern int 	rngIsFull (RING_ID ringId);
extern RING_ID 	rngCreate (int nbytes);
extern int 	rngBufGet (RING_ID rngId, char *buffer, int maxbytes);
extern int 	rngBufGetOnly (RING_ID rngId, char *buffer, int maxbytes);
extern int 	rngBufPut (RING_ID rngId, char *buffer, int nbytes);
extern int 	rngFreeBytes (RING_ID ringId);
extern int 	rngNBytes (RING_ID ringId);
extern void 	rngDelete (RING_ID ringId);
extern void 	rngFlush (RING_ID ringId);
extern void 	rngMoveAhead (RING_ID ringId, int n);
extern void 	rngPutAhead (RING_ID ringId, char byte, int offset);
extern void	rngDelet (RING_ID ringId, int n);

#endif
