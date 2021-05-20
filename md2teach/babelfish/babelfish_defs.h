/*
 *  babelfish_defs.h
 *  md2teach
 *
 *  Created by Jeremy Rand on 2021-05-10.
 * 
 */

#ifndef _GUARD_PROJECTmd2teach_FILEbabelfish_defs_
#define _GUARD_PROJECTmd2teach_FILEbabelfish_defs_

#define rTrData         0x5472

// Flags
#define bf320           0x0001
#define bf640           0x0002
#define bfCanImport     0x0001
#define bfCanExport     0x0001
#define bfImportOptions 0x0002
#define bfExportOptions 0x0002

//Translator IDs
#define TrVersion       0x0001    //rVersion
#define TrAbout         0x0001    //rComment
#define TrCantLauch     0x0002

#define TrData          0x0001    //rTrData

#define TrInit          0x0001    //rCodeResource
#define TrImportOptions 0x0002    //rCodeResource
#define TrExportOptions 0x0003
#define TrFilter        0x0004

#define TrImportInfo    0x0002    //rText
#define TrExportInfo    0x0003    //rText

#define TrFormatNone            0x0000
#define TrFormatText            0x0001
#define TrFormatPixelMap        0x0002
#define TrFormatTrueColorImage  0x0003
#define TrFormatQD2Picture      0x0004
#define TrFormatFont            0x0005
#define TrFormatSound           0x0006

#define TrStartUp       0x9101
#define TrShutDown      0x9102
#define TrRead          0x9103

#define bfContinue     0
#define bfDone         0x8000
#define bfUserAbort    0x8001
#define bfBadFileErr   0x8002
#define bfReadErr      0x8003
#define bfWriteErr     0x8004
#define bfMemErr       0x8005

#define bfTextGetSettings   1
#define bfTextBody          8

#endif /* define _GUARD_PROJECTmd2teach_FILEbabelfish_defs_ */
