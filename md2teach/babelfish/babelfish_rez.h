/*
 *  babelfish_rez.h
 *  md2teach
 *
 *  Created by Jeremy Rand on 2021-05-10.
 * 
 */

#ifndef _GUARD_PROJECTmd2teach_FILEbabelfish_rez_
#define _GUARD_PROJECTmd2teach_FILEbabelfish_rez_


type rTrData
{
    _mybase_ integer = 0;    //revision
    _mybase_ integer;    //general flags
    _mybase_ integer;    //Import Flags
    array[8]
    {
        hex byte;    //Import Kinds
    };
    _mybase_ integer;    //Export Flags
    array[8]
    {
        hex byte;    //Export Kinds
    };
};                                   


#endif /* define _GUARD_PROJECTmd2teach_FILEbabelfish_rez_ */
