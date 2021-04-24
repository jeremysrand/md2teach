/*
 *  style.c
 *  md2teach
 *
 * Created by Jeremy Rand on 2021-04-24.
 * 
 */

#include <stdint.h>

#include <textedit.h>

#include "style.h"


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

static tWindowPos windowPos = {
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
static uint8_t headerFontSizes[NUM_HEADER_SIZES] = {
    36,
    30,
    27,
    24,
    20,
    18
};

static tFormat * formatPtr = NULL;
