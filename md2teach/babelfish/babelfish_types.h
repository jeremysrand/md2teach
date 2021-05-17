/*
 *  babelfish_types.h
 *  md2teach
 *
 *  Created by Jeremy Rand on 2021-05-16.
 * 
 */

#ifndef _GUARD_PROJECTmd2teach_FILEbabelfish_types_
#define _GUARD_PROJECTmd2teach_FILEbabelfish_types_

#include <Types.h>


// The actual data record varies based on the file format.
typedef struct TrDataRecord
{
    Word parmCount;
} TrDataRecord;


typedef struct TrTextDataRecord
{
    Word parmCount;
    Word actionCode;
    Word responseCode;
    Long textStreamLength;
    char * textStreamPtr;
    Handle textStreamHandle;
    Word familyId;
    Word fontSize;
    Word fontStyle;
    Word foreColor;
    Word backColor;
    Word position;
    Word charSpacing;
    Word lineSpacing;
    Word spaceBefore;
    Word spaceAfter;
    Word firstIndent;
    Word leftIndent;
    Word rightIndent;
    Word justification;
    unsigned char tabArray[64];
    Word options;
    Long border;
    Word pageLength;
    Word pageWidth;
    Rect sectionRect;
    Word columns;
    Word gutter;
    Handle picHandle;
} TrTextDataRecord;


typedef struct TrTransferRecord
{
    Word parmCount;
    Word status;
    Word miscFlags;
    unsigned char dataKinds[8];
    Word transNum;
    Word userId;
    Word progressAction;
    Word fullTherm;
    Word currentTherm;
    char * msgPtr;
    TrDataRecord * dataRecordPtr;
    char * filePathPtr;     /* C string*/
    char * fileNamePtr;     /* P string */
} TrTransferRecord;


typedef struct TrStartUpDataIn
{
    TrTransferRecord * xferRecPtr;
} TrStartUpDataIn;


typedef struct TrStartUpDataOut
{
    Word recvCount;
    Word trResult;
} TrStartUpDataOut;


typedef struct TrShutDownDataIn
{
    TrTransferRecord * xferRecPtr;
} TrShutDownDataIn;


typedef struct TrShutDownDataOut
{
    Word recvCount;
    Word trResult;
} TrShutDownDataOut;


typedef struct TrReadDataIn
{
    TrTransferRecord * xferRecPtr;
} TrReadDataIn;


typedef struct TrReadDataOut
{
    Word recvCount;
    Word trResult;
} TrReadDataOut;


#endif /* define _GUARD_PROJECTmd2teach_FILEbabelfish_types_ */
