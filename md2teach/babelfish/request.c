/*
 * init.c
 * md2teach
 *
 * Created by Jeremy Rand on 2021-05-12.
 * Copyright (c) 2021 Jeremy Rand. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <font.h>
#include <locator.h>
#include <memory.h>

#include "babelfish/babelfish_defs.h"
#include "babelfish/babelfish_types.h"
#include "md4c.h"


#pragma memorymodel 1


static uint16_t myUserId;
static TrTextDataRecord textRecord;


void DoStartUp(TrStartUpDataIn * dataIn, TrStartUpDataOut * dataOut)
{
    static recvCount = 0;
    TrTransferRecord * xferRecPtr = dataIn->xferRecPtr;
    
    asm {
        stz |lb1
    lb1: nop
        nop
    };
    
    xferRecPtr->dataKinds[0] = TrFormatText;
    xferRecPtr->dataKinds[1] = TrFormatNone;
    xferRecPtr->dataKinds[2] = TrFormatNone;
    xferRecPtr->dataKinds[3] = TrFormatNone;
    xferRecPtr->dataKinds[4] = TrFormatNone;
    xferRecPtr->dataKinds[5] = TrFormatNone;
    xferRecPtr->dataKinds[6] = TrFormatNone;
    xferRecPtr->dataKinds[7] = TrFormatNone;
    xferRecPtr->fullTherm = 512;
    xferRecPtr->currentTherm = 0;
    xferRecPtr->dataRecordPtr = (TrDataRecord *)&textRecord;
    
    textRecord.parmCount = 10;
    textRecord.actionCode = bfTextGetSettings;
    textRecord.responseCode = 0;
    textRecord.textStreamLength = 0;
    textRecord.textStreamPtr = NULL;
    textRecord.textStreamHandle = NULL;
    textRecord.familyId = helvetica;
    textRecord.fontSize = 12;
    textRecord.foreColor = 0x0000;
    textRecord.backColor = 0xffff;
    textRecord.position = 0;
    
    recvCount++;
    dataOut->recvCount = recvCount;
    dataOut->trResult = bfContinue;
}


void DoShutDown(TrShutDownDataIn * dataIn, TrShutDownDataOut * dataOut)
{
    static recvCount = 0;
    
    recvCount++;
    dataOut->recvCount = recvCount;
    dataOut->trResult = bfContinue;
}


void DoRead(TrReadDataIn * dataIn, TrReadDataOut * dataOut)
{
    static recvCount = 0;
    
    recvCount++;
    dataOut->recvCount = recvCount;
    dataOut->trResult = bfContinue;
}


#pragma databank 1
#pragma toolparms 1

pascal unsigned MyRequestProc(uint16_t request, uint32_t dataIn, uint32_t dataOut)
{
    switch (request) {
        case TrStartUp:
            DoStartUp((TrStartUpDataIn *)dataIn, (TrStartUpDataOut *)dataOut);
            break;
            
        case TrShutDown:
            DoShutDown((TrShutDownDataIn *)dataIn, (TrShutDownDataOut *)dataOut);
            break;
            
        case TrRead:
            DoRead((TrReadDataIn *)dataIn, (TrReadDataOut *)dataOut);
            break;
            
        default:
            break;
    }
    
	return 0;
}

#pragma toolparms 0
#pragma databank 0

void setup(void)
{
    static char myName[48];
    
	myUserId = MMStartUp();
    sprintf(myName+1, "Babelfish~Jeremy~MarkdownTrans%04x", myUserId);
    *myName = strlen(myName+1);
    
    AcceptRequests(myName, myUserId, MyRequestProc);

	// return 0;
}
