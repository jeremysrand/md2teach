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
#include <orca.h>
#include <resources.h>

#include "io.h"
#include "main.h"
#include "style.h"


// Defines

// This enables a workaround for creating the resources by writing a .rez file rather than
// writing it directly.  At the moment, Golden Gate does not support writing resources
// but this will let me test it without that capability.
#define RESOURCE_WORKAROUND

#define TEACH_FILE_TYPE 0x50
#define TEACH_AUX_TYPE 0x5445

#define R_WINDOW_POSITION 0x7001
#define WINDOW_POSITION_NUM 1

#define STYLE_BLOCK_NUM 1



// Typedefs

typedef struct tWindowPos
{
    int16_t height;
    int16_t width;
    int16_t top;
    int16_t left;
    int32_t version;
} tWindowPos;


// Globals

static GSString255 outputFileName;
static IORecGS writeRec;
static char writeBuffer[4096];
static int32_t writeBufferOffset = 0;
static MD_SIZE writePos = 0;

static tWindowPos windowPos = {
    0xad,   // height
    0x27c,  // width
    0x1a,   // top
    0x02,   // left
    0x0     // version
};


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
    createRec.fileType = TEACH_FILE_TYPE;
    createRec.auxType = TEACH_AUX_TYPE;
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


static int writeResources(void)
{
#ifndef RESOURCE_WORKAROUND
    int shutdownResources = 0;
    Word writeResId;
    
    if (!ResourceStatus()) {
        ResourceStartUp(userid());
        shutdownResources = 1;
    }
    
    CreateResourceFile(TEACH_AUX_TYPE, TEACH_FILE_TYPE, destroyEnable | renameEnable | readWriteEnable, (Pointer)outputFileName);
    if (toolerror()) {
        fprintf(stderr, "%s: Unable to create resources of file %s, toolerror=0x%x\n", commandName, outputFileName.text, toolerror());
        return 1;
    }
    writeResId = OpenResourceFile(0x8000 | readWriteEnable, NULL, (Pointer)outputFileName);
    if (toolerror()) {
        fprintf(stderr, "%s: Unable to open resources of file %s, toolerror=0x%x\n", commandName, outputFileName.text, toolerror());
        return 1;
    }
    
    CloseResourceFile(writeResId);
    
    // Implement more of this...
    
    if (shutdownResources)
        ResourceShutDown();
#else
    FILE * rezFile;
    uint8_t * ptr;
    uint32_t size;
    uint32_t i;
    
    strcat(outputFileName.text, ".rez");
    rezFile = fopen(outputFileName.text, "w");
    if (rezFile == NULL) {
        fprintf(stderr, "%s: Unable to open resource file %s, %s\n", commandName, outputFileName.text, strerror(errno));
        return 1;
    }
    
    fprintf(rezFile,
"#define rStyleBlock 0x%x\n"
"#define rWindowPosition 0x%x\n"
"\n"
"type rStyleBlock {\n"
"    hex string;\n"
"};\n"
"\n"
"type rWindowPosition {\n"
"    hex string;\n"
"};\n"
"\n"
"resource rStyleBlock (%u) {\n"
"    $\"",
            rStyleBlock, R_WINDOW_POSITION, STYLE_BLOCK_NUM
            );
    
    ptr = stylePtr();
    size = styleSize();
    for (i = 0; i < size; i++) {
        if ((i > 0) &&
            ((i % 32) == 0)) {
            fprintf(rezFile, "\"\n    $\"");
        }
        fprintf(rezFile, "%02x", (uint16_t)*ptr);
        ptr++;
    }
    
    fprintf(rezFile, "\"\n"
"};\n"
"\n"
"resource rWindowPosition (%u) {\n"
"    $\"",
            WINDOW_POSITION_NUM);
    
    ptr = (uint8_t *)(&windowPos);
    size = sizeof(windowPos);
    for (i = 0; i < size; i++) {
        if ((i > 0) &&
            ((i % 32) == 0)) {
            fprintf(rezFile, "\"\n    $\"");
        }
        fprintf(rezFile, "%02x", (uint16_t)*ptr);
        ptr++;
    }
    fprintf(rezFile, "\"\n"
"};\n"
"\n");
    
    fclose(rezFile);
#endif
    
    return 0;
}


int closeOutputFile(void)
{
    RefNumRecGS closeRec;
    
    if (writeBufferOffset > 0)
        flushBuffer();
    closeRec.pCount = 1;
    closeRec.refNum = writeRec.refNum;
    CloseGS(&closeRec);
    
    return writeResources();
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
