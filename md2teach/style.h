/*
 *  style.h
 *  md2teach
 *
 *  Created by Jeremy Rand on 2021-04-24.
 * 
 */

#ifndef _GUARD_PROJECTmd2teach_FILEstyle_
#define _GUARD_PROJECTmd2teach_FILEstyle_

// Defines

#define NUM_HEADER_SIZES 6

// This is plain, emphasized, strong or strong+empasized
#define NUM_TEXT_FORMATS 4

#define NUM_HEADER_STYLES (NUM_HEADER_SIZES * NUM_TEXT_FORMATS)
#define NUM_CODE_STYLES 1
#define NUM_TEXT_STYLES NUM_TEXT_FORMATS
#define NUM_QUOTE_STYLES NUM_TEXT_FORMATS

#define TOTAL_STYLES (NUM_HEADER_STYLES + NUM_CODE_STYLES + NUM_TEXT_STYLES + NUM_QUOTE_STYLES)


#endif /* define _GUARD_PROJECTmd2teach_FILEstyle_ */
