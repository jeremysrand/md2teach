/*
 *  io.h
 *  md2teach
 *
 *  Created by Jeremy Rand on 2021-04-24.
 * 
 */

#ifndef _GUARD_PROJECTmd2teach_FILEio_
#define _GUARD_PROJECTmd2teach_FILEio_


#include "md4c.h"


extern int openOutputFile(const char * filename);
extern void writeChar(MD_CHAR ch);
extern void writeString(const MD_CHAR * str, MD_SIZE size);
extern MD_SIZE outputPos(void);
extern void closeOutputFile(void);

extern const MD_CHAR * readInputFile(const char * filename, MD_SIZE * bufferSize);
extern void releaseInputBuffer(const MD_CHAR * inputBuffer);


#endif /* define _GUARD_PROJECTmd2teach_FILEio_ */
