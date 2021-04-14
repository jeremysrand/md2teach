/*
 * main.c
 * md2teach
 *
 * Created by Jeremy Rand on 2021-04-13.
 * Copyright (c) 2021 Jeremy Rand. All rights reserved.
 *
 */


#include <stdio.h>
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
    0, // flags
    enterBlockHook,
    leaveBlockHook,
    enterSpanHook,
    leaveSpanHook,
    textHook,
    debugLogHook,
    NULL // syntax
};

char *testMarkdown = "Hello, world!\n\nMore text\n";


// Implementation

static int enterBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    printf("Enter Block: type = %d, detail = %p\n", (int)type, detail);
    return 0;
}


static int leaveBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    printf("Leave Block: type = %d, detail = %p\n", (int)type, detail);
    return 0;
}


static int enterSpanHook(MD_SPANTYPE type, void * detail, void * userdata)
{
    printf("Enter Span: type = %d, detail = %p\n", (int)type, detail);
    return 0;
}


static int leaveSpanHook(MD_SPANTYPE type, void * detail, void * userdata)
{
    printf("Leave Span: type = %d, detail = %p\n", (int)type, detail);
    return 0;
}


static int textHook(MD_TEXTTYPE type, const MD_CHAR * text, MD_SIZE size, void * userdata)
{
    printf("Text: type = %d, size=%lu, text=", (int)type, size);
    fwrite(text, sizeof(MD_CHAR), size, stdout);
    putchar('\n');
    return 0;
}


static void debugLogHook(const char * message, void * userdata)
{
    printf("DEBUG: %s\n", message);
}


int main(int argc, char * argv[])
{
    int result;
    printf("Stack start: %p\n", &result);
    
    result = md_parse(testMarkdown, strlen(testMarkdown), &parser, NULL);
    printf("Parser result: %d\n", result);
    
    return 0;
}
