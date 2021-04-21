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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "md4c.h"

#pragma memorymodel 1

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

// Typedefs

typedef struct tBlockListItem
{
    MD_BLOCKTYPE type;
    union {
        MD_BLOCK_UL_DETAIL ulDetail;
        MD_BLOCK_OL_DETAIL olDetail;
        MD_BLOCK_H_DETAIL hDetail;
        MD_BLOCK_CODE_DETAIL codeDetail;
    } u;
    int numTabs;
    
    struct tBlockListItem * next;
} tBlockListItem;

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
int debugIndentLevel = 0;
int debugEnabled = 0;
int isFirstNonDocumentBlock = 1;

tBlockListItem * blockList = NULL;

FILE * outputFile;


// Implementation

static int enterBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    tBlockListItem * newBlock = malloc(sizeof(tBlockListItem));
    
    if (newBlock == NULL) {
        fprintf(stderr, "%s: Out of memory", commandName);
        return 1;
    }
    
    newBlock->type = type;
    if (blockList == NULL)
        newBlock->numTabs = 0;
    else
        newBlock->numTabs = blockList->numTabs;
    newBlock->next = blockList;
    blockList = newBlock;
    
    if ((detail != NULL) &&
        (detail < lowestStackSeen))
        lowestStackSeen = detail;
    
    switch (type) {
        case MD_BLOCK_DOC:
            if (debugEnabled)
                fprintf(stderr, "%*sDOC {\n", debugIndentLevel, "");
            break;
            
        case MD_BLOCK_QUOTE:
            if (debugEnabled)
                fprintf(stderr, "%*sQUOTE {\n", debugIndentLevel, "");
            
            break;
            
        case MD_BLOCK_UL: {
            MD_BLOCK_UL_DETAIL * ulDetail = (MD_BLOCK_UL_DETAIL *)detail;
            if (debugEnabled)
                fprintf(stderr, "%*sUL (is_tight=%d, mark=%c) {\n", debugIndentLevel, "", ulDetail->is_tight, ulDetail->mark);
            
            memcpy(&(newBlock->u.ulDetail), ulDetail, sizeof(*ulDetail));
            newBlock->numTabs++;
            
            if (!isFirstNonDocumentBlock)
                fputc('\r', outputFile);
            break;
        }
            
        case MD_BLOCK_OL: {
            MD_BLOCK_OL_DETAIL * olDetail = (MD_BLOCK_OL_DETAIL *)detail;
            if (debugEnabled)
                fprintf(stderr, "%*sOL (start=%u, is_tight=%d, mark_delimiter=%c) {\n", debugIndentLevel, "", olDetail->start, olDetail->is_tight, olDetail->mark_delimiter);
            
            memcpy(&(newBlock->u.olDetail), olDetail, sizeof(*olDetail));
            newBlock->numTabs++;
            
            if (!isFirstNonDocumentBlock)
                fputc('\r', outputFile);
            break;
        }
            
        case MD_BLOCK_LI: {
            int i;
            tBlockListItem * enclosingBlock = newBlock->next;
            int isNumbered = 0;
            
            if (debugEnabled)
                fprintf(stderr, "%*sLI {\n", debugIndentLevel, "");
            
            if (enclosingBlock == NULL) {
                fprintf(stderr, "%s: Got a list item block without an enclosing block\n", commandName);
                return 1;
            }
            
            if (enclosingBlock->type == MD_BLOCK_OL) {
                isNumbered = 1;
                if ((!enclosingBlock->u.olDetail.is_tight) &&
                    (!isFirstNonDocumentBlock))
                    fputc('\r', outputFile);
            } else if (enclosingBlock->type == MD_BLOCK_UL) {
                if ((!enclosingBlock->u.ulDetail.is_tight) &&
                    (!isFirstNonDocumentBlock))
                    fputc('\r', outputFile);
            }
            
            for (i = 0; i < newBlock->numTabs; i++)
                fputc('\t', outputFile);
            
            if (isNumbered) {
                fprintf(outputFile, "%u%c ", enclosingBlock->u.olDetail.start, enclosingBlock->u.olDetail.mark_delimiter);
                enclosingBlock->u.olDetail.start++;
            } else {
                fprintf(outputFile, "%c ", 0xd7);    // 0xd7 is a diamond looking character which is good for a bullet
            }
            
            break;
        }
            
        case MD_BLOCK_HR: {
            int i;
            
            if (debugEnabled)
                fprintf(stderr, "%*sHR {\n", debugIndentLevel, "");
            
            if (!isFirstNonDocumentBlock)
                fputc('\r', outputFile);
            
            for (i = 0; i < 30; i++)
                fputc('_', outputFile);    // 0xd1 is a horizontal line
            break;
        }
            
        case MD_BLOCK_H: {
            MD_BLOCK_H_DETAIL * hDetail = (MD_BLOCK_H_DETAIL *)detail;
            if (debugEnabled)
                fprintf(stderr, "%*sH (level=%u) {\n", debugIndentLevel, "", hDetail->level);
            
            memcpy(&(newBlock->u.hDetail), hDetail, sizeof(*hDetail));
            
            if (!isFirstNonDocumentBlock)
                fputc('\r', outputFile);
            break;
        }
            
        case MD_BLOCK_CODE: {
            MD_BLOCK_CODE_DETAIL * codeDetail = (MD_BLOCK_CODE_DETAIL *)detail;
            if (debugEnabled) {
                fprintf(stderr, "%*sCODE ", debugIndentLevel, "");
                if (codeDetail->fence_char != '\0') {
                    fprintf(stderr, "(fence_char=%c) ", codeDetail->fence_char);
                }
                fprintf(stderr, "{\n");
            }
            
            memcpy(&(newBlock->u.codeDetail), codeDetail, sizeof(*codeDetail));
            
            if (!isFirstNonDocumentBlock)
                fputc('\r', outputFile);
            break;
        }
            
        case MD_BLOCK_P:
            if (debugEnabled)
                fprintf(stderr, "%*sP {\n", debugIndentLevel, "");
            
            if (!isFirstNonDocumentBlock)
                fputc('\r', outputFile);
            break;
            
        default:
            fprintf(stderr, "%s: Invalid block type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    
    if (type != MD_BLOCK_DOC)
        isFirstNonDocumentBlock = 0;
    
    debugIndentLevel+=2;
    return 0;
}


static int leaveBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    tBlockListItem * oldBlock = blockList;
    
    if (oldBlock == NULL) {
        fprintf(stderr, "%s: Block list is empty but leaving block of type %d\n", commandName, (int)type);
        return 1;
    }
    
    if (oldBlock->type != type) {
        fprintf(stderr, "%s: Expected to leave block of type %d but got type %d\n", commandName, (int)oldBlock->type, (int)type);
        return 1;
    }
    
    blockList = oldBlock->next;
    free(oldBlock);
    
    if ((detail != NULL) &&
        (detail < lowestStackSeen))
        lowestStackSeen = detail;
    
    switch (type) {
        case MD_BLOCK_DOC:
            break;
            
        case MD_BLOCK_QUOTE:
            break;
            
        case MD_BLOCK_UL:
            fputc('\r', outputFile);
            break;
            
        case MD_BLOCK_OL:
            fputc('\r', outputFile);
            break;
            
        case MD_BLOCK_LI:
            fputc('\r', outputFile);
            break;
            
        case MD_BLOCK_HR:
            fputc('\r', outputFile);
            break;
            
        case MD_BLOCK_H:
            fputc('\r', outputFile);
            break;
            
        case MD_BLOCK_CODE:
            fputc('\r', outputFile);
            break;
            
        case MD_BLOCK_P:
            fputc('\r', outputFile);
            break;
            
        default:
            fprintf(stderr, "%s: Invalid block type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    debugIndentLevel-=2;
    if (debugEnabled)
        fprintf(stderr, "%*s}\n", debugIndentLevel, "");
    
    return 0;
}


static int enterSpanHook(MD_SPANTYPE type, void * detail, void * userdata)
{
    if ((detail != NULL) &&
        (detail < lowestStackSeen))
        lowestStackSeen = detail;
    
    switch (type) {
        case MD_SPAN_EM:
            if (debugEnabled)
                fprintf(stderr, "%*sEM {\n", debugIndentLevel, "");
            break;
            
        case MD_SPAN_STRONG:
            if (debugEnabled)
                fprintf(stderr, "%*sSTRONG {\n", debugIndentLevel, "");
            break;
            
        case MD_SPAN_A:
            if (debugEnabled)
                fprintf(stderr, "%*sA {\n", debugIndentLevel, "");
            break;
            
        case MD_SPAN_IMG:
            if (debugEnabled)
                fprintf(stderr, "%*sIMG {\n", debugIndentLevel, "");
            break;
            
        case MD_SPAN_CODE:
            if (debugEnabled)
                fprintf(stderr, "%*sCODE {\n", debugIndentLevel, "");
            break;
            
        default:
            fprintf(stderr, "%s: Invalid span type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    debugIndentLevel+=2;
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
    
    debugIndentLevel-=2;
    if (debugEnabled)
        fprintf(stderr, "%*s}\n", debugIndentLevel, "");
    
    return 0;
}


static int textHook(MD_TEXTTYPE type, const MD_CHAR * text, MD_SIZE size, void * userdata)
{
    switch (type) {
        case MD_TEXT_NORMAL:
            if (debugEnabled)
                fprintf(stderr, "%*sText: \"", debugIndentLevel, "");
            break;
        
        case MD_TEXT_NULLCHAR:
            fprintf(stderr, "%s: Null character encountered on input\n", commandName);
            return 1;
            
        case MD_TEXT_BR:
            if (debugEnabled)
                fprintf(stderr, "%*sBR\n", debugIndentLevel, "");
            putchar('\n');
            return 0;
            
        case MD_TEXT_SOFTBR:
            if (debugEnabled)
                fprintf(stderr, "%*sSOFT BR\n", debugIndentLevel, "");
            return 0;
            
        case MD_TEXT_ENTITY:
            if (debugEnabled)
                fprintf(stderr, "%*sEntity: \"", debugIndentLevel, "");
            
            // GS_TODO - For now, just skip printing anything but it would be best to look
            // at the extended character map and do the "right thing" for special characters
            // like the copyright symbol.
            text = "";
            size = 0;
            break;
            
        case MD_TEXT_CODE:
            if (debugEnabled)
                fprintf(stderr, "%*sCode: \"", debugIndentLevel, "");
            break;
            
        default:
            fprintf(stderr, "%s: Invalid text type (%d)\n", commandName, (int)type);
            return 1;
            break;
    }
    
    if (debugEnabled) {
        fwrite(text, sizeof(MD_CHAR), size, stderr);
        fprintf(stderr, "\"\n");
    }
    
    if (size > 0)
        fwrite(text, sizeof(MD_CHAR), size, outputFile);
    
    return 0;
}


static void debugLogHook(const char * message, void * userdata)
{
    if (debugEnabled)
        fprintf(stderr, "DEBUG: %s\n", message);
}


static void printUsage(void)
{
    fprintf(stderr, "USAGE: %s [ -d ] inputfile outputfile\n", commandName);
    exit(1);
}


static int parseArgs(int argc, char * argv[])
{
    commandName = argv[0];
    
    static int index;
    static int charOffset;
    static int optionLen;
    
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
                    break;
            }
        }
    }
    
    if (index + 2 != argc) {
        printUsage();
    }
    
    return index;
}


int main(int argc, char * argv[])
{
    int result;
    
    static char * inputFileName;
    static FILE * inputFile;
    static long inputFileLen;
    static char * inputBuffer;
    
    static char * outputFileName;
    
    static int index;
    
    lowestStackSeen = &result;
    
    index = parseArgs(argc, argv);
    inputFileName = argv[index];
    outputFileName = argv[index + 1];
    
    outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "%s: Unable to open output file %s, %s\n", commandName, outputFileName, strerror(errno));
        exit(1);
    }
    
    inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "%s: Unable to open input file %s, %s\n", commandName, inputFileName, strerror(errno));
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
    
    putchar('\n');
    
    fclose(inputFile);
    fclose(outputFile);
    
    if (debugEnabled) {
        fprintf(stderr, "Parser result: %d\n", result);
        fprintf(stderr, "Most stack used: %lu\n", ((unsigned long)&result) - ((unsigned long)lowestStackSeen));
    }
    
    return 0;
}
