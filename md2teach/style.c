/*
 *  style.c
 *  md2teach
 *
 * Created by Jeremy Rand on 2021-04-24.
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include <font.h>
#include <memory.h>
#include <textedit.h>

#include "io.h"
#include "main.h"
#include "style.h"


// Defines

#define NUM_HEADER_SIZES 6

// This is plain, emphasized, strong or strong+empasized
#define NUM_TEXT_FORMATS 4

#define NUM_HEADER_STYLES (NUM_HEADER_SIZES * NUM_TEXT_FORMATS)
#define NUM_TEXT_STYLES NUM_TEXT_FORMATS
#define NUM_QUOTE_STYLES NUM_TEXT_FORMATS
#define NUM_CODE_STYLES 1

#define TOTAL_STYLES (NUM_HEADER_STYLES + NUM_TEXT_STYLES + NUM_QUOTE_STYLES + NUM_CODE_STYLES)

#define STARTING_STYLE_ITEMS 32


// Typedefs

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


// Globals

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

static Handle formatHandle = NULL;
static uint32_t allocStyleItems = 0;
static MD_SIZE styleChangedAt = 0;


// Implementation

static tFormat * growStyleItems(void)
{
    uint32_t newAllocStyleItems = 2 * allocStyleItems;
    
    HUnlock(formatHandle);
    SetHandleSize(sizeof(tFormatHeader) + newAllocStyleItems * sizeof(StyleItem), formatHandle);
    if (toolerror()) {
        fprintf(stderr, "%s: Out of memory, toolerror=0x%x\n", commandName, toolerror());
        exit(1);
    }
    
    allocStyleItems = newAllocStyleItems;
    HLock(formatHandle);
    return (tFormat *)(*formatHandle);
}


int addStyle(int styleListNum, uint16_t fontFamily, uint8_t fontSize, uint8_t fontStyle, uint16_t backgroundColour)
{
    tFormat * formatPtr = (tFormat *)(*formatHandle);
    formatPtr->header.styleList[styleListNum].styleFontID.fidRec.famNum = fontFamily;
    formatPtr->header.styleList[styleListNum].styleFontID.fidRec.fontSize = fontSize;
    formatPtr->header.styleList[styleListNum].styleFontID.fidRec.fontStyle = fontStyle;
    formatPtr->header.styleList[styleListNum].foreColor = 0x00;
    formatPtr->header.styleList[styleListNum].backColor = backgroundColour;
    formatPtr->header.styleList[styleListNum].userData = 0x00;
    
    return styleListNum + 1;
}

int styleInit(void)
{
    int styleListNum;
    int headerSize;
    tFormat * formatPtr;
    
    formatHandle = NewHandle(sizeof(formatPtr->header) + STARTING_STYLE_ITEMS * sizeof(StyleItem), userid(), attrNoPurge, NULL);
    if (toolerror()) {
        fprintf(stderr, "%s: Out of memory, toolerror=0x%x\n", commandName, toolerror());
        return 1;
    }
    HLock(formatHandle);
    formatPtr = (tFormat *)(*formatHandle);
    allocStyleItems = STARTING_STYLE_ITEMS;
    
    formatPtr->header.version = 0x0000;
    
    formatPtr->header.rulerSize = sizeof(formatPtr->header.ruler);
    formatPtr->header.ruler.leftMargin = 0x00;
    formatPtr->header.ruler.leftIndent = 0x00;
    formatPtr->header.ruler.rightMargin = 0x0221;
    formatPtr->header.ruler.just = leftJust;
    formatPtr->header.ruler.extraLS = 0x00;
    formatPtr->header.ruler.flags = 0x00;
    formatPtr->header.ruler.userData = 0x00;
    formatPtr->header.ruler.tabType = stdTabs;
    formatPtr->header.ruler.tabTerminator = 0x40;
    
    formatPtr->header.styleListLength = sizeof(formatPtr->header.styleList);
    
    styleListNum = 0;
    
    // Add header styles
    for (headerSize = 0; headerSize < NUM_HEADER_SIZES; headerSize++) {
        styleListNum = addStyle(styleListNum, helvetica, headerFontSizes[headerSize], plainMask, 0xffff);
        styleListNum = addStyle(styleListNum, helvetica, headerFontSizes[headerSize], boldMask, 0xffff);
        styleListNum = addStyle(styleListNum, helvetica, headerFontSizes[headerSize], italicMask, 0xffff);
        styleListNum = addStyle(styleListNum, helvetica, headerFontSizes[headerSize], boldMask | italicMask, 0xffff);
    }
    
    // Add test styles - Also default the first text format to plain.
    formatPtr->header.numberOfStyles = 1;
    formatPtr->styleItems[0].dataOffset = 0;
    formatPtr->styleItems[0].dataOffset = styleListNum * sizeof(formatPtr->header.styleList[0]);
    
    styleListNum = addStyle(styleListNum, helvetica, 12, plainMask, 0xffff);
    styleListNum = addStyle(styleListNum, helvetica, 12, boldMask, 0xffff);
    styleListNum = addStyle(styleListNum, helvetica, 12, italicMask, 0xffff);
    styleListNum = addStyle(styleListNum, helvetica, 12, boldMask | italicMask, 0xffff);
    
    // Add quote styles
    styleListNum = addStyle(styleListNum, helvetica, 12, plainMask, 0xeeee);
    styleListNum = addStyle(styleListNum, helvetica, 12, boldMask, 0xeeee);
    styleListNum = addStyle(styleListNum, helvetica, 12, italicMask, 0xeeee);
    styleListNum = addStyle(styleListNum, helvetica, 12, boldMask | italicMask, 0xeeee);
    
    // Add code style
    styleListNum = addStyle(styleListNum, courier, 12, plainMask, 0xffff);
    
    HUnlock(formatHandle);
    
    if (styleListNum != TOTAL_STYLES)
    {
        fprintf(stderr, "%s: Expected %d styles but created %d styles.\n", commandName);
        return 1;
    }
    
    return 0;
}


void setStyle(tStyleType styleType, uint16_t textMask, uint16_t headerSize)
{
    int32_t styleOffset;
    MD_SIZE currentPos;
    int lastStyleIndex;
    tFormat * formatPtr;
    
    HLock(formatHandle);
    formatPtr = (tFormat *)(*formatHandle);
    
    lastStyleIndex = formatPtr->header.numberOfStyles - 1;
    
    switch (styleType) {
        case STYLE_TYPE_HEADER:
            styleOffset = ((headerSize - 1) * NUM_TEXT_FORMATS) + textMask;
            break;
            
        case STYLE_TYPE_TEXT:
            styleOffset = NUM_HEADER_STYLES + textMask;
            break;
            
        case STYLE_TYPE_QUOTE:
            styleOffset = NUM_HEADER_STYLES + NUM_TEXT_STYLES + textMask;
            break;
            
        case STYLE_TYPE_CODE:
            styleOffset = NUM_HEADER_STYLES + NUM_TEXT_STYLES + NUM_QUOTE_STYLES;
            break;
            
        default:
            fprintf(stderr, "%s: Unexpected style type (%u)\n", commandName, (uint16_t)styleType);
    }
    
    styleOffset *= sizeof(formatPtr->header.styleList[0]);
    
    // If the offset requested is the same as the one we already have, then just return.
    // Nothing has changed.
    if (formatPtr->styleItems[lastStyleIndex].dataOffset == styleOffset)
    {
        HUnlock(formatHandle);
        return;
    }
    
    // Check to see if the previous style actually emitted any characters and if not,
    // then just overwrite it with this new style.
    currentPos = outputPos();
    
    if (debugEnabled)
        fprintf(stderr, "%*ssetStyle(%u,%u,%u) @ offset %lu\n", debugIndentLevel, "", (uint16_t)styleType, textMask, headerSize, currentPos);
    
    if (styleChangedAt == currentPos) {
        formatPtr->styleItems[lastStyleIndex].dataOffset = styleOffset;
        HUnlock(formatHandle);
        return;
    }
    
    formatPtr->styleItems[lastStyleIndex].dataLength = currentPos - styleChangedAt;
    styleChangedAt = currentPos;
    
    if (formatPtr->header.numberOfStyles == allocStyleItems)
        formatPtr = growStyleItems();
    
    lastStyleIndex++;
    formatPtr->header.numberOfStyles++;
    formatPtr->styleItems[lastStyleIndex].dataOffset = styleOffset;
    HUnlock(formatHandle);
}

void closeStyle(void)
{
    int lastStyleIndex;
    tFormat * formatPtr;
    uint32_t formatSize;
    MD_SIZE currentPos = outputPos();
    
    HLock(formatHandle);
    formatPtr = (tFormat *)(*formatHandle);
    
    lastStyleIndex = formatPtr->header.numberOfStyles - 1;
    
    // If the final style was not used, then remove it.  Otherwise, update the length of the
    // final style.
    if (styleChangedAt == currentPos) {
        formatPtr->header.numberOfStyles--;
    } else {
        formatPtr->styleItems[lastStyleIndex]. dataLength = currentPos - styleChangedAt;
    }
    
    formatSize = sizeof(formatPtr->header) + (sizeof(formatPtr->styleItems) * formatPtr->header.numberOfStyles);
    
    HUnlock(formatHandle);
    if (GetHandleSize(formatHandle) != formatSize)
        SetHandleSize(formatSize, formatHandle);
}

Handle styleHandle(void)
{
    Handle result = formatHandle;
    formatHandle = NULL;
    return result;
}

uint8_t * stylePtr(void)
{
    HLock(formatHandle);
    return (uint8_t *)(*formatHandle);
}

uint32_t styleSize(void)
{
    return GetHandleSize(formatHandle);
}

void styleShutdown(void)
{
    if (formatHandle != NULL)
        DisposeHandle(formatHandle);
}
