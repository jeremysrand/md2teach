/*
 *  io.c
 *  md2teach
 *
 * Created by Jeremy Rand on 2021-04-24.
 * 
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsos.h>

#include "io.h"
#include "main.h"


// Globals

static IORecGS writeRec;
static char writeBuffer[4096];
static int32_t writeBufferOffset = 0;
static MD_SIZE writePos = 0;


// Implementation

static void flushBuffer(void)
{
    writeRec.requestCount = writeBufferOffset;
    WriteGS(&writeRec);
    if (toolerror()) {
        fprintf(stderr, "%s: Error writing to output file\n", commandName);
        exit(1);
    }
    writeBufferOffset = 0;
}


int openOutputFile(const char * filename)
{
    GSString255 outputFileName;
    CreateRecGS createRec;
    NameRecGS destroyRec;
    OpenRecGS openRec;
    
    outputFileName.length = strlen(filename);
    if (outputFileName.length >= sizeof(outputFileName.text)) {
        fprintf(stderr, "%s: Output file path too long, %s\n", commandName, outputFileName);
        return 1;
    }
    strcpy(outputFileName.text, filename);
    
    destroyRec.pCount = 1;
    destroyRec.pathname = &outputFileName;
    DestroyGS(&destroyRec);
    
    createRec.pCount = 5;
    createRec.pathname = &outputFileName;
    createRec.access = destroyEnable | renameEnable | readWriteEnable;
    createRec.fileType = 0x50; // Type for Teach file
    createRec.auxType = 0x5445; // Aux type for Teach file
    createRec.storageType = extendedFile;
    CreateGS(&createRec);
    if (toolerror()) {
        fprintf(stderr, "%s: Unable to create output file %s\n", commandName, outputFileName.text);
        return 1;
    }
    
    openRec.pCount = 3;
    openRec.refNum = 0;
    openRec.pathname = &outputFileName;
    openRec.requestAccess = writeEnable;
    OpenGS(&openRec);
    if (toolerror()) {
        fprintf(stderr, "%s: Unable to open output file %s\n", commandName, outputFileName.text);
        return 1;
    }
    
    writeRec.pCount = 4;
    writeRec.refNum = openRec.refNum;
    writeRec.dataBuffer = writeBuffer;
    return 0;
}


void writeChar(MD_CHAR ch)
{
    if (writeBufferOffset == sizeof(writeBuffer))
        flushBuffer();
    
    if (ch == '\n')
        ch = '\r';
    writeBuffer[writeBufferOffset] = ch;
    writeBufferOffset++;
    writePos++;
}


void writeString(const MD_CHAR * str, MD_SIZE size)
{
    MD_SIZE i;
    
    for (i = 0; i < size; i++)
        writeChar(str[i]);
}


MD_SIZE outputPos(void)
{
    return writePos;
}


void closeOutputFile(void)
{
    RefNumRecGS closeRec;
    
    if (writeBufferOffset > 0)
        flushBuffer();
    closeRec.pCount = 1;
    closeRec.refNum = writeRec.refNum;
    CloseGS(&closeRec);
}


const MD_CHAR * readInputFile(const char * filename, MD_SIZE * bufferSize)
{
    FILE * inputFile;
    MD_CHAR * inputBuffer;
    MD_SIZE inputFileLen;
    
    inputFile = fopen(filename, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "%s: Unable to open input file %s, %s\n", commandName, filename, strerror(errno));
        return NULL;
    }
    
    if (fseek(inputFile, 0l, SEEK_END) != 0) {
        fprintf(stderr, "%s: Unable to seek to the end of file %s, %s\n", commandName, filename, strerror(errno));
        fclose(inputFile);
        return NULL;
    }
    
    inputFileLen = ftell(inputFile);
    if (inputFileLen < 0) {
        fprintf(stderr, "%s: Unable to get size of file %s, %s\n", commandName, filename, strerror(errno));
        fclose(inputFile);
        return NULL;
    }
    
    inputBuffer = malloc(inputFileLen);
    if (inputBuffer == NULL) {
        fprintf(stderr, "%s: Unable to allocate %ld bytes for input buffer\n", commandName, inputFileLen);
        fclose(inputFile);
        return NULL;
    }
    
    if (fseek(inputFile, 0l, SEEK_SET) != 0) {
        fprintf(stderr, "%s: Unable to seek to the beginning of file %s, %s\n", commandName, filename, strerror(errno));
        free(inputBuffer);
        fclose(inputFile);
        return NULL;
    }
    
    if (fread(inputBuffer, 1, inputFileLen, inputFile) != inputFileLen) {
        fprintf(stderr, "%s: Unable to read all of file %s, %s\n", commandName, filename, strerror(errno));
        free(inputBuffer);
        fclose(inputFile);
        return NULL;
    }
    
    fclose(inputFile);
    
    *bufferSize = inputFileLen;
    return inputBuffer;
}


void releaseInputBuffer(const MD_CHAR * inputBuffer)
{
    free(inputBuffer);
}
