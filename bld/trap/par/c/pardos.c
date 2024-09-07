/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2002-2024 The Open Watcom Contributors. All Rights Reserved.
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include <dos.h>
#include "dosequip.h"
#include "parlink.h"
#include "realmod.h"


static  unsigned long __far *BiosTime;

char *InitSys( void )
{
    BiosTime = _MK_FP( BDATA_SEG, BDATA_SYSTEM_CLOCK );
    return( 0 );
}

void FiniSys( void )
{
}

/* value of Ticks is incremented approx every 1/10 th of a second */

unsigned long Ticks( void )
{
    return( *BiosTime >> 1 );
}


int NumPrinters( void )
{
    return( Equipment().num_printers );
}


unsigned PrnAddress( int printer )
{
    return( *(unsigned short __far *)_MK_FP( BDATA_SEG, BDATA_PRINTER_BASE + printer * 2 ) );
}

void FreePorts( unsigned first, unsigned last )
{
    /* unused parameters */ (void)first; (void)last;
}

bool AccessPorts( unsigned first, unsigned last )
{
    /* unused parameters */ (void)first; (void)last;

    return( true );
}
