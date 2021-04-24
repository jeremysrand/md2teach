/*
 * main.c
 * md2teach
 *
 * Created by Jeremy Rand on 2021-04-13.
 * Copyright (c) 2021 Jeremy Rand. All rights reserved.
 *
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <resources.h>
#include <textedit.h>

#include "io.h"
#include "main.h"
#include "translate.h"
#include "md4c.h"

// GS_TODO - How big does the stack need to be?  In looking over the code,
// I don't see massive stack frames due to large globals (other than the
// context which I made static).  But I do see lots of arguments and if
// the call stack gets deep enough, we could have a problem.
//
// Testing looks pretty good though with a trivial input.  The stack seems
// to reach just more than 512 bytes deep when the hook functions are called.
// I suspect things can get much worse with a complex document but this
// approach should let me measure the worst case stack with a complex
// document.
//
// Leaving the stack very big for now at 32K.
#pragma stacksize 32768

// Defines

#define NUM_HEADER_SIZES 6

// This is plain, emphasized, strong or strong+empasized
#define NUM_TEXT_FORMATS 4

#define NUM_HEADER_STYLES (NUM_HEADER_SIZES * NUM_TEXT_FORMATS)
#define NUM_CODE_STYLES 1
#define NUM_TEXT_STYLES NUM_TEXT_FORMATS
#define NUM_QUOTE_STYLES NUM_TEXT_FORMATS

#define TOTAL_STYLES (NUM_HEADER_STYLES + NUM_CODE_STYLES + NUM_TEXT_STYLES + NUM_QUOTE_STYLES)

// Typedefs

typedef struct tWindowPos
{
    int16_t height;
    int16_t width;
    int16_t top;
    int16_t left;
    int32_t version;
} tWindowPos;

// I wish I could use the structure definition from textedit.h but TERuler contains optional
// fields in the definition and Teach isn't expecting them it seems (array of theTabs).  So,
// I need my own struct which omits them.
typedef struct tRuler
{
    int16_t leftMargin;
    int16_t leftIndent;
    int16_t rightMargin;
    int16_t just;
    int16_t extraLS;
    int16_t flags;
    int32_t userData;
    int16_t tabType;
    int16_t tabTerminator;
} tRuler;

typedef struct tFormatHeader
{
    int16_t version;
    int32_t rulerSize;
    tRuler ruler;
    int32_t styleListLength;
    TEStyle styleList[TOTAL_STYLES];
    LongWord numberOfStyles;
} tFormatHeader;

typedef struct tFormat
{
    tFormatHeader header;
    StyleItem styleItems[1];
} tFormat;

//typedef struct tStyle


// Globals

void * lowestStackSeen;
char * commandName;
int debugEnabled = 0;

tWindowPos windowPos = {
    0xad,   // height
    0x27c,  // width
    0x1a,   // top
    0x02,   // left
    0x0     // version
};

// For the 6 header sizes, we are going with:
//      1 -> Helvetica 36
//      2 -> Helvetica 30
//      3 -> Helvetica 27
//      4 -> Helvetica 24
//      5 -> Helvetica 20
//      6 -> Helvetica 18
uint8_t headerFontSizes[NUM_HEADER_SIZES] = {
    36,
    30,
    27,
    24,
    20,
    18
};

tFormat * formatPtr = NULL;

// Implementation


static void printUsage(void)
{
    fprintf(stderr, "USAGE: %s [ -d ] inputfile outputfile\n", commandName);
}


static int parseArgs(int argc, char * argv[])
{
    static int index;
    static int charOffset;
    static int optionLen;
    
    commandName = argv[0];
    
    for (index = 1; index < argc; index++) {
        if (argv[index][0] != '-')
            break;
        
        optionLen = strlen(argv[index]);
        for (charOffset = 1; charOffset < optionLen; charOffset++) {
            switch (argv[index][charOffset]) {
                case 'd':
                    debugEnabled = 1;
                    break;
                
                default:
                    printUsage();
                    return -1;
                    break;
            }
        }
    }
    
    if (index + 2 != argc) {
        printUsage();
        return -1;
    }
    
    return index;
}


int main(int argc, char * argv[])
{
    int result;
    MD_SIZE inputFileLen;
    MD_CHAR * inputBuffer;
    int index;
    
    lowestStackSeen = &result;
    
    index = parseArgs(argc, argv);
    if (index < 0)
        exit(1);
    
    inputBuffer = readInputFile(argv[index], &inputFileLen);
    if (inputBuffer == NULL)
        exit(1);
    
    if (openOutputFile(argv[index + 1]) != 0) {
        releaseInputBuffer(inputBuffer);
        exit(1);
    }
    
    result = parse(inputBuffer, inputFileLen);
    
    closeOutputFile();
    releaseInputBuffer(inputBuffer);
    
    putchar('\n');
    
    if (debugEnabled) {
        fprintf(stderr, "Parser result: %d\n", result);
        fprintf(stderr, "Most stack used: %lu\n", ((unsigned long)&result) - ((unsigned long)lowestStackSeen));
    }
    
    if (result != 0)
        fprintf(stderr, "%s: Parser failed (%d)\n", commandName, result);
    
    return result;
}
