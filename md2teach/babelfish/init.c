/*
 * init.c
 * md2teach
 *
 * Created by Jeremy Rand on 2021-05-12.
 * Copyright (c) 2021 Jeremy Rand. All rights reserved.
 *
 */

#include <stdio.h>

#include <Locator.h>
#include <Memory.h>

#include "babelfish/babelfish_defs.h"
#include "babelfish/babelfish_types.h"
#include "md4c.h"


#pragma memorymodel 1
#pragma rtl


uint16_t myUserId;


void DoStartUp(TrStartUpDataIn * dataIn, TrStartUpDataOut * dataOut)
{
    
}


void DoShutDown(TrShutDownDataIn * dataIn, TrShutDownDataOut * dataOut)
{
    
}


void DoRead(TrReadDataIn * dataIn, TrReadDataOut * dataOut)
{
    
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


int main(void)
{
    static char myName[48];
    
	myUserId = MMStartUp();
    sprintf(myName, "Babelfish~Jeremy~MarkdownTrans%04x", myUserId);
    AcceptRequests(myName, myUserId, MyRequestProc);

	return 0;
}

#pragma databank 0
