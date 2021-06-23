/*
 * main.c
 * md2teach
 *
 * Created by Jeremy Rand on 2021-04-13.
 * Copyright (c) 2021 Jeremy Rand. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "main.h"
#include "translate.h"


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
// Leaving the stack very big for now at 8K.
#pragma stacksize 8192


// Globals

char * commandName;
int debugEnabled = 0;
int debugIndentLevel = 0;
int generateRez = 0;


// Implementation

static void printUsage(void)
{
    fprintf(stderr, "USAGE: %s [ -d ] [ -r ] inputfile outputfile\n", commandName);
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
                    
                case 'r':
                    generateRez = 1;
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

    releaseInputBuffer(inputBuffer);
    
    if (debugEnabled) {
        fprintf(stderr, "Parser result: %d\n", result);
    }
    
    if (result != 0)
        fprintf(stderr, "%s: Parser failed (%d)\n", commandName, result);
    
    if (closeOutputFile() != 0)
        result = 1;
    
    if (result != 0)
        remove(argv[index + 1]);
    
    putchar('\n');
    
    styleShutdown();
    
    return result;
}
