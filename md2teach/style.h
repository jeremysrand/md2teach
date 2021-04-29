/*
 *  style.h
 *  md2teach
 *
 *  Created by Jeremy Rand on 2021-04-24.
 * 
 */

#ifndef _GUARD_PROJECTmd2teach_FILEstyle_
#define _GUARD_PROJECTmd2teach_FILEstyle_

#include "md4c.h"


// Defines

#define STYLE_TEXT_PLAIN 0u
#define STYLE_TEXT_MASK_STRONG 1u
#define STYLE_TEXT_MASK_EMPHASIZED 2u


// Typedefs

typedef enum tStyleType {
    STYLE_TYPE_HEADER,
    STYLE_TYPE_TEXT,
    STYLE_TYPE_QUOTE,
    STYLE_TYPE_CODE
} tStyleType;


// API

extern int styleInit(void);
extern void setStyle(tStyleType styleType, uint16_t textMask, uint16_t headerSize);
extern void closeStyle(void);

uint8_t * stylePtr(void);
uint32_t styleSize(void);


#endif /* define _GUARD_PROJECTmd2teach_FILEstyle_ */
