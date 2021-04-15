/*
 * main.c
 * md2teach
 *
 * Created by Jeremy Rand on 2021-04-13.
 * Copyright (c) 2021 Jeremy Rand. All rights reserved.
 *
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "md4c.h"

#pragma memorymodel 1

// GS_TODO - Scan all of the code looking at uses of int and unsigned and
// consider changing them to long and unsigned long if they need to use
// numbers > 64K.

// GS_TODO - How big does the stack need to be?  In looking over the code,
// I don't see massive stack frames due to large globals (other than the
// context which I made static).  But I do see lots of arguments and if
// the call stack gets deep enough, we could have a problem.
//
// Testing looks pretty good though with a trivial input.  The stack seems
// to be less that 256 bytes deep when the hook functions are called.  I
// suspect things can get much worse with a complex document but this
// approach should let me measure the worst case stack with a complex
// document.
//
// Leaving the stack very big for now at 32K.
#pragma stacksize 32768


// Forward declarations

static int enterBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata);
static int leaveBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata);
static int enterSpanHook(MD_SPANTYPE type, void * detail, void * userdata);
static int leaveSpanHook(MD_SPANTYPE type, void * detail, void * userdata);
static int textHook(MD_TEXTTYPE type, const MD_CHAR * text, MD_SIZE size, void * userdata);
static void debugLogHook(const char * message, void * userdata);


// Globals

MD_PARSER parser = {
    0, // abi_version
    MD_FLAG_NOHTMLBLOCKS | MD_FLAG_NOHTMLSPANS, // flags
    enterBlockHook,
    leaveBlockHook,
    enterSpanHook,
    leaveSpanHook,
    textHook,
    debugLogHook,
    NULL // syntax
};

void * lowestStackSeen;
char * commandName;
int indentLevel = 0;


// Implementation

static int enterBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    if ((detail != NULL) &&
        (detail < lowestStackSeen))
        lowestStackSeen = detail;
    
    switch (type) {
        case MD_BLOCK_DOC:
            printf("%*sDOC {\n", indentLevel, "");
            break;
            
        case MD_BLOCK_QUOTE:
            printf("%*sQUOTE {\n", indentLevel, "");
            break;
            
        case MD_BLOCK_UL: {
            MD_BLOCK_UL_DETAIL * ulDetail = (MD_BLOCK_UL_DETAIL *)detail;
            printf("%*sUL (is_tight=%d, mark=%c) {\n", indentLevel, "", ulDetail->is_tight, ulDetail->mark);
            break;
        }
            
        case MD_BLOCK_OL: {
            MD_BLOCK_OL_DETAIL * olDetail = (MD_BLOCK_OL_DETAIL *)detail;
            printf("%*sOL (start=%u, is_tight=%d, mark_delimiter=%c) {\n", indentLevel, "", olDetail->start, olDetail->is_tight, olDetail->mark_delimiter);
            break;
        }
            
        case MD_BLOCK_LI:
            printf("%*sLI {\n", indentLevel, "");
            break;
            
        case MD_BLOCK_HR:
            printf("%*sHR {\n", indentLevel, "");
            break;
            
        case MD_BLOCK_H: {
            MD_BLOCK_H_DETAIL * hDetail = (MD_BLOCK_H_DETAIL *)detail;
            printf("%*sH (level=%u) {\n", indentLevel, "", hDetail->level);
            break;
        }
            
        case MD_BLOCK_CODE: {
            MD_BLOCK_CODE_DETAIL * codeDetail = (MD_BLOCK_CODE_DETAIL *)detail;
            printf("%*sCODE ", indentLevel, "");
            if (codeDetail->fence_char != '\0') {
                printf("(fence_char=%c) ", codeDetail->fence_char);
            }
            printf("{\n");
            break;
        }
            
        case MD_BLOCK_P:
            printf("%*sP {\n", indentLevel, "");
            break;
            
        default:
            fprintf(stderr, "%s: Invalid block type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    indentLevel+=2;
    return 0;
}


static int leaveBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    if ((detail != NULL) &&
        (detail < lowestStackSeen))
        lowestStackSeen = detail;
    
    switch (type) {
        case MD_BLOCK_DOC:
            break;
            
        case MD_BLOCK_QUOTE:
            break;
            
        case MD_BLOCK_UL:
            break;
            
        case MD_BLOCK_OL:
            break;
            
        case MD_BLOCK_LI:
            break;
            
        case MD_BLOCK_HR:
            break;
            
        case MD_BLOCK_H:
            break;
            
        case MD_BLOCK_CODE:
            break;
            
        case MD_BLOCK_P:
            break;
            
        default:
            fprintf(stderr, "%s: Invalid block type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    indentLevel-=2;
    printf("%*s}\n", indentLevel, "");
    
    return 0;
}


static int enterSpanHook(MD_SPANTYPE type, void * detail, void * userdata)
{
    if ((detail != NULL) &&
        (detail < lowestStackSeen))
        lowestStackSeen = detail;
    
    switch (type) {
        case MD_SPAN_EM:
            printf("%*sEM {\n", indentLevel, "");
            break;
            
        case MD_SPAN_STRONG:
            printf("%*sSTRONG {\n", indentLevel, "");
            break;
            
        case MD_SPAN_A:
            printf("%*sA {\n", indentLevel, "");
            break;
            
        case MD_SPAN_IMG:
            printf("%*sIMG {\n", indentLevel, "");
            break;
            
        case MD_SPAN_CODE:
            printf("%*sCODE {\n", indentLevel, "");
            break;
            
        default:
            fprintf(stderr, "%s: Invalid span type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    indentLevel+=2;
    return 0;
}


static int leaveSpanHook(MD_SPANTYPE type, void * detail, void * userdata)
{
    if ((detail != NULL) &&
        (detail < lowestStackSeen))
        lowestStackSeen = detail;
    
    switch (type) {
        case MD_SPAN_EM:
            break;
            
        case MD_SPAN_STRONG:
            break;
            
        case MD_SPAN_A:
            break;
            
        case MD_SPAN_IMG:
            break;
            
        case MD_SPAN_CODE:
            break;
            
        default:
            fprintf(stderr, "%s: Invalid span type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    indentLevel-=2;
    printf("%*s}\n", indentLevel, "");
    
    return 0;
}


static int textHook(MD_TEXTTYPE type, const MD_CHAR * text, MD_SIZE size, void * userdata)
{
    switch (type) {
        case MD_TEXT_NORMAL:
            printf("%*sText: \"", indentLevel, "");
            break;
        
        case MD_TEXT_NULLCHAR:
            fprintf(stderr, "%s: Null character encountered on input\n", commandName);
            return 1;
            
        case MD_TEXT_BR:
        case MD_TEXT_SOFTBR:
            return 0;
            
        case MD_TEXT_ENTITY:
            printf("%*sEntity: \"", indentLevel, "");
            break;
            
        case MD_TEXT_CODE:
            printf("%*sCode: \"", indentLevel, "");
            break;
            
        default:
            fprintf(stderr, "%s: Invalid text type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    fwrite(text, sizeof(MD_CHAR), size, stdout);
    printf("\"\n");
    
    return 0;
}


static void debugLogHook(const char * message, void * userdata)
{
    printf("DEBUG: %s\n", message);
}


int main(int argc, char * argv[])
{
    int result;
    
    static char * inputFileName;
    static FILE * inputFile;
    static long inputFileLen;
    static char * inputBuffer;
    
    static char * outputFileName;
    
    lowestStackSeen = &result;
    
    if (argc != 3) {
        fprintf(stderr, "USAGE: %s inputfile outputfile\n", argv[0]);
        exit(1);
    }
    
    commandName = argv[0];
    inputFileName = argv[1];
    outputFileName = argv[2];
    
    inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "%s: Unable to open file %s, %s\n", commandName, inputFileName, strerror(errno));
        exit(1);
    }
    
    if (fseek(inputFile, 0l, SEEK_END) != 0) {
        fprintf(stderr, "%s: Unable to seek to the end of file %s, %s\n", commandName, inputFileName, strerror(errno));
        fclose(inputFile);
        exit(1);
    }
    
    inputFileLen = ftell(inputFile);
    if (inputFileLen < 0) {
        fprintf(stderr, "%s: Unable to get size of file %s, %s\n", commandName, inputFileName, strerror(errno));
        fclose(inputFile);
        exit(1);
    }
    
    inputBuffer = malloc(inputFileLen);
    if (inputBuffer == NULL) {
        fprintf(stderr, "%s: Unable to allocate %ld bytes for input buffer\n", commandName, inputFileLen);
        fclose(inputFile);
        exit(1);
    }
    
    if (fseek(inputFile, 0l, SEEK_SET) != 0) {
        fprintf(stderr, "%s: Unable to seek to the beginning of file %s, %s\n", commandName, inputFileName, strerror(errno));
        free(inputBuffer);
        fclose(inputFile);
        exit(1);
    }
    
    if (fread(inputBuffer, 1, inputFileLen, inputFile) != inputFileLen) {
        fprintf(stderr, "%s: Unable to read all of file %s, %s\n", commandName, inputFileName, strerror(errno));
        free(inputBuffer);
        fclose(inputFile);
        exit(1);
    }
    
    result = md_parse(inputBuffer, inputFileLen, &parser, NULL);
    printf("Parser result: %d\n", result);
    printf("Most stack used: %lu\n", ((unsigned long)&result) - ((unsigned long)lowestStackSeen));
    
    return 0;
}
