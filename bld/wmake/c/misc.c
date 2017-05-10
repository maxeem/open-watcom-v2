/****************************************************************************
*
*                            Open Watcom Project
*
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
* Description:  Miscellaneous wmake helper functions.
*
****************************************************************************/


#if defined( __UNIX__ )
    #include <dirent.h>
#else
    #include <direct.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include "make.h"
#include "mmemory.h"
#include "mmisc.h"
#include "mlex.h"
#include "mrcmsg.h"
#include "msg.h"
#include "pathgrp.h"

#include "clibext.h"


static ENV_TRACKER  *envList;

char *SkipWS( const char *p )
/*****************************
 * p is not const because the return value is usually used to write data.
 */
{
    while( cisws( *p ) ) {
        ++p;
    }
    return( (char *)p );
}

STATIC bool is_ws( char c )
/*************************/
{
    return( cisws( c ) );
}

STATIC bool is_ws_or_equal( char c )
/**********************************/
{
    return( cisws( c ) || c == '=' );
}

STATIC char *FindNextSep( const char *str, bool (*chk_sep)( char ) )
/******************************************************************
 * Finds next free separator character, allowing doublequotes to
 * be used to specify strings with white spaces.
 */
{
    bool    string_open;
    char    c;

    string_open = false;
    while( (c = *str) != NULLCHAR ) {
        if( c == BACKSLASH ) {
            if( string_open && str[1] != NULLCHAR ) {
                ++str;
            }
        } else if( c == DOUBLEQUOTE ) {
            string_open = !string_open;
        } else if( !string_open && chk_sep( c ) ) {
            break;
        }
        str++;
    }

    return( (char *)str );
}

char *FindNextWS( const char *str )
/***********************************
 * Finds next free white space character, allowing doublequotes to
 * be used to specify strings with white spaces.
 */
{
    return( FindNextSep( str, is_ws ) );
}

char *FindNextWSorEqual( const char *str )
/*****************************************
 * Finds next free white space or equal character, allowing doublequotes to
 * be used to specify strings with white spaces.
 */
{
    return( FindNextSep( str, is_ws_or_equal ) );
}

char *RemoveDoubleQuotes( char *dst, size_t maxlen, const char *src )
/************************************************************************
 * Removes doublequote characters from string and copies other content
 * from src to dst. Only maxlen number of characters are copied to dst
 * including terminating NUL character.
 */
{
    char    *orgdst = dst;
    bool    string_open = false;
    size_t  pos = 0;
    char    t;

    assert( maxlen );

    // leave space for NUL terminator
    maxlen--;

    while( pos < maxlen ) {
        t = *src++;

        if( t == NULLCHAR ) {
            break;
        }

        if( t == BACKSLASH ) {
            t = *src++;

            if( t == DOUBLEQUOTE ) {
                *dst++ = DOUBLEQUOTE;
                pos++;
            } else {
                *dst++ = BACKSLASH;
                pos++;

                if( pos < maxlen ) {
                    *dst++ = t;
                    pos++;
                }
            }
        } else {
            if( t == DOUBLEQUOTE ) {
                string_open = !string_open;
            } else {
                if( string_open ) {
                    *dst++ = t;
                    pos++;
                } else if( cisws( t ) ) {
                    break;
                } else {
                    *dst++ = t;
                    pos++;
                }
            }
        }
    }

    *dst = NULLCHAR;

    return( orgdst );
}

char *FixName( char *name )
{
#if defined( __DOS__ )
/*********************************
 * Down case all filenames, converting fwd-slash to back-slash
 */
    char    *ptr;
    char    hold;

    assert( name != NULL );

    for( ptr = name; (hold = *ptr) != NULLCHAR; hold = *++ptr ) {
        if( hold == '/' ) {
            *ptr = '\\';
        } else if( cisalpha( hold ) && hold < 'a') {
            *ptr = hold - 'A' + 'a';
        }
        hold = *++ptr;
        if( hold == NULLCHAR ) {
            break;
        }
        if( hold == '/' ) {
            *ptr = '\\';
        } else if( cisalpha( hold ) && hold < 'a') {
            *ptr = hold - 'A' + 'a';
        }
    }

    return( name );
#elif defined( __OS2__ ) || defined( __NT__ )
/*********************************
 * convert fwd-slash to back-slash
 */
    char    *ptr;
    char    hold;

    assert( name != NULL );

    for( ptr = name; (hold = *ptr) != NULLCHAR; hold = *++ptr ) {
        if( hold == '/' ) {
            *ptr = '\\';
        }
        hold = *++ptr;
        if( hold == NULLCHAR ) {
            break;
        }
        if( hold == '/' ) {
            *ptr = '\\';
        }
    }

    return( name );
#else
    return( name );
#endif
}


bool FNameEq( const char *a, const char *b )
/******************************************/
{
#if defined( __OS2__ ) || defined( __NT__ ) || defined( __DOS__ )
    return( stricmp( a, b ) == 0 );
#else
    return( strcmp( a, b ) == 0 );
#endif
}


static bool FNameChrEq( char a, char b )
/**************************************/
{
#if defined( __OS2__ ) || defined( __NT__ ) || defined( __DOS__ )
    return( ctolower( a ) == ctolower( b ) );
#else
    return( a == b );
#endif
}


#ifdef USE_FAR
bool FarFNameEq( const char FAR *a, const char FAR *b )
/*****************************************************/
{
#if defined( __OS2__ ) || defined( __NT__ ) || defined( __DOS__ )
    return( _fstricmp( a, b ) == 0 );
#else
    return( _fstrcmp( a, b ) == 0 );
#endif
}
#endif


#define IS_WILDCARD_CHAR( x ) ((*x == '*') || (*x == '?'))

static bool __fnmatch( const char *pattern, const char *string )
/***************************************************************
 * OS specific compare function FNameChrEq
 * must be used for file names
 */
{
    const char  *p;
    int         len;
    bool        star_char;
    int         i;

    /*
     * check pattern section with wildcard characters
     */
    star_char = false;
    while( IS_WILDCARD_CHAR( pattern ) ) {
        if( *pattern == '?' ) {
            if( *string == NULLCHAR ) {
                return( false );
            }
            string++;
        } else {
            star_char = true;
        }
        pattern++;
    }
    if( *pattern == NULLCHAR ) {
        if( (*string == NULLCHAR) || star_char ) {
            return( true );
        } else {
            return( false );
        }
    }
    /*
     * check pattern section with exact match
     * ( all characters except wildcards )
     */
    p = pattern;
    len = 0;
    do {
        if( star_char ) {
            if( string[len] == NULLCHAR ) {
                return( false );
            }
            len++;
        } else {
            if( !FNameChrEq( *pattern, *string ) ) {
                return( false );
            }
            string++;
        }
        pattern++;
    } while( *pattern != NULLCHAR && !IS_WILDCARD_CHAR( pattern ) );
    if( !star_char ) {
        /*
         * match is OK, try next pattern section
         */
        return( __fnmatch( pattern, string ) );
    } else {
        /*
         * star pattern section, try locate exact match
         */
        while( *string != NULLCHAR ) {
            if( FNameChrEq( *p, *string ) ) {
                for( i = 1; i < len; i++ ) {
                    if( !FNameChrEq( *(p + i), *(string + i) ) ) {
                        break;
                    }
                }
                if( i == len ) {
                    /*
                     * if rest doesn't match, find next occurence
                     */
                    if( __fnmatch( pattern, string + len ) ) {
                        return( true );
                    }
                }
            }
            string++;
        }
        return( false );
    }
}

/*
 * THIS FUNCTION IS NOT RE-ENTRANT!
 *
 * It returns a pointer to a character string, after doing wildcard
 * substitutions.  It returns NULL when there are no more substitutions
 * possible.
 *
 * DoWildCard behaves similarly to strtok.  You first pass it a pointer
 * to a substitution string.  It checks if the string contains wildcards,
 * and if not it simply returns this string.  If the string contains
 * wildcards, it attempts an opendir with the string.  If that fails it
 * returns the string.
 *
 * If the opendir succeeds, or you pass DoWildCard a NULL pointer, it reads
 * the next normal file from the directory, and returns the filename.
 *
 * If there are no more files in the directory, or no directory is open,
 * DoWildCard returns null.
 *
 */

static DIR  *parent = NULL;  /* we need this across invocations */
static char *path = NULL;
static char *pattern = NULL;

const char *DoWildCard( const char *base )
/***********************************************/
{
    PGROUP          pg;
    struct dirent   *entry;

    if( base != NULL ) {
        /* clean up from previous invocation */
        DoWildCardClose();

        if( strpbrk( base, WILD_METAS ) == NULL ) {
            return( base );
        }
        // create directory name and pattern
        path = MallocSafe( _MAX_PATH );
        pattern = MallocSafe( _MAX_PATH );
        strcpy( path, base );
        FixName( path );
        _splitpath2( path, pg.buffer, &pg.drive, &pg.dir, &pg.fname, &pg.ext );
        _makepath( path, pg.drive, pg.dir, ".", NULL );
        // create file name pattern
        _makepath( pattern, NULL, NULL, pg.fname, pg.ext );

        parent = opendir( path );
        if( parent == NULL ) {
            DoWildCardClose();
            return( base );
        }
    }

    if( parent == NULL ) {
        return( NULL );
    }

    assert( path != NULL && parent != NULL );

    while( (entry = readdir( parent )) != NULL ) {
#ifndef __UNIX__
        if( (entry->d_attr & IGNORE_MASK) == 0 ) {
#endif
            if( __fnmatch( pattern, entry->d_name ) ) {
                break;
            }
#ifndef __UNIX__
        }
#endif
    }
    if( entry == NULL ) {
        DoWildCardClose();
        return( base );
    }

    _splitpath2( path, pg.buffer, &pg.drive, &pg.dir, &pg.fname, &pg.ext );
    _makepath( path, pg.drive, pg.dir, entry->d_name, NULL );

    return( path );
}


void DoWildCardClose( void )
/**************************/
{
    if( path != NULL ) {
        FreeSafe( path );
        path = NULL;
    }
    if( pattern != NULL ) {
        FreeSafe( pattern );
        pattern = NULL;
    }
    if( parent != NULL ) {
        closedir( parent );
        parent = NULL;
    }
}


int KWCompare( const void *p1, const void *p2 )     /* for bsearch */
/*********************************************/
{
    return( stricmp( *(const char **)p1, *(const char **)p2 ) );
}


int PutEnvSafe( ENV_TRACKER *env )
/****************************************
 * This function takes over responsibility for freeing env
 */
{
    char        *p;
    ENV_TRACKER **walk;
    ENV_TRACKER *old;
    int         rc;
    size_t      len;

    p = env->value;
                                // upper case the name
    while( *p != '=' && *p != NULLCHAR ) {
        *p = (char)ctoupper( *p );
        ++p;
    }
    rc = putenv( env->value );  // put into environment
    if( p[0] == '=' && p[1] == NULLCHAR ) {
        rc = 0;                 // we are deleting the envvar, ignore errors
    }
    len = p - env->value + 1;   // len including '='
    for( walk = &envList; *walk != NULL; walk = &(*walk)->next ) {
        if( strncmp( (*walk)->value, env->value, len ) == 0 ) {
            break;
        }
    }
    old = *walk;
    if( old != NULL ) {
        *walk = old->next;      // unlink from chain
        FreeSafe( old );
    }
    if( p[1] != NULLCHAR ) {    // we're giving it a new value
        env->next = envList;    // save the memory since putenv keeps a
        envList = env;          // pointer to it...
    } else {                    // we're deleting an old value
        FreeSafe( env );
    }
    return( rc );
}


#if !defined(NDEBUG) || defined(DEVELOPMENT)
void PutEnvFini( void )
/****************************/
{
    ENV_TRACKER *cur;

    while( envList != NULL ) {
        cur = envList;
        envList = cur->next;
        FreeSafe( cur );
    }
}
#endif
