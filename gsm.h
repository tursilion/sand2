//
// (C) 2004 Mike Brent aka Tursi aka HarmlessLion.com
// This software is provided AS-IS. No warranty
// express or implied is provided.
//
// This notice defines the entire license for this code.
// All rights not explicity granted here are reserved by the
// author.
//
// You may redistribute this software provided the original
// archive is UNCHANGED and a link back to my web page,
// http://harmlesslion.com, is provided as the author's site.
// It is acceptable to link directly to a subpage at harmlesslion.com
// provided that page offers a URL for that purpose
//
// Source code, if available, is provided for educational purposes
// only. You are welcome to read it, learn from it, mock
// it, and hack it up - for your own use only.
//
// Please contact me before distributing derived works or
// ports so that we may work out terms. I don't mind people
// using my code but it's been outright stolen before. In all
// cases the code must maintain credit to the original author(s).
//
// -COMMERCIAL USE- Contact me first. I didn't make
// any money off it - why should you? ;) If you just learned
// something from this, then go ahead. If you just pinched
// a routine or two, let me know, I'll probably just ask
// for credit. If you want to derive a commercial tool
// or use large portions, we need to talk. ;)
//
// If this, itself, is a derived work from someone else's code,
// then their original copyrights and licenses are left intact
// and in full force.
//
// http://harmlesslion.com - visit the web page for contact info
//
/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /home/kbs/jutta/src/gsm/gsm-1.0/inc/RCS/gsm.h,v 1.11 1996/07/05 18:02:56 jutta Exp $*/

#ifndef	GSM_H
#define	GSM_H

#ifdef __cplusplus
#	define	NeedFunctionPrototypes	1
#endif

#if __STDC__
#	define	NeedFunctionPrototypes	1
#endif

#ifdef _NO_PROTO
#	undef	NeedFunctionPrototypes
#endif

#ifdef NeedFunctionPrototypes
#   include	<stdio.h>		/* for FILE * 	*/
#endif

#undef GSM_P
#if NeedFunctionPrototypes
#	define	GSM_P( protos )	protos
#else
#	define  GSM_P( protos )	( /* protos */ )
#endif

/*
 *	Interface
 */

typedef struct gsm_state*	gsm;
typedef short		   	gsm_signal;		/* signed 16 bit */
typedef unsigned char	gsm_byte;
typedef gsm_byte 		gsm_frame[33];		/* 33 * 8 bits	 */

#define	GSM_MAGIC		0xD		  	/* 13 kbit/s RPE-LTP */

#define	GSM_PATCHLEVEL	10
#define	GSM_MINOR		0
#define	GSM_MAJOR		1

#define	GSM_OPT_VERBOSE		1
#define	GSM_OPT_FAST		2
#define	GSM_OPT_LTP_CUT		3
#define	GSM_OPT_WAV49		4
#define	GSM_OPT_FRAME_INDEX	5
#define	GSM_OPT_FRAME_CHAIN	6

extern gsm  gsm_create 	GSM_P((void));
extern void gsm_destroy GSM_P((gsm));	

extern int  gsm_print   GSM_P((FILE *, gsm, gsm_byte  *));
extern int  gsm_option  GSM_P((gsm, int, int *));

extern void gsm_encode  GSM_P((gsm, gsm_signal *, gsm_byte  *));
/*extern int  gsm_decode  GSM_P((gsm, gsm_byte   *, gsm_signal *));*/
extern int  gsm_decode  GSM_P((gsm, gsm_byte   *, unsigned char *));

extern int  gsm_explode GSM_P((gsm, gsm_byte   *, gsm_signal *));
extern void gsm_implode GSM_P((gsm, gsm_signal *, gsm_byte   *));

#undef	GSM_P

#endif	/* GSM_H */
