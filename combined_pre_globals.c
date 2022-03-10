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
/* Derived by Tursi from code by:
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

#include <string.h>		/* for memory funcs  */
#include <malloc.h>     /* for malloc in gsm_create */
#include "gsm_short.h"

/* private.h */
typedef short				word;		/* 16 bit signed int	*/
typedef long				longword;	/* 32 bit signed int	*/

typedef unsigned short		uword;		/* unsigned word	*/
typedef unsigned long		ulongword;	/* unsigned longword	*/

struct gsm_state {
	word		dp0[ 280 ];
	word		LARpp[2][8]; 	/*                              */
	word		j;				/*                              */
	word		nrp; /* 40 */	/* long_term.c, synthesis	*/
	word		v[9];			/* short_term.c, synthesis	*/
	word		msr;			/* decoder.c,	Postprocessing	*/
} sGSM;

#define	MIN_WORD	(-32767 - 1)
#define	MAX_WORD	  32767

#define	MIN_LONGWORD	(-2147483647 - 1)
#define	MAX_LONGWORD	  2147483647

#define	SASR(x, by)	((x) >> (by))

#define GSM_MULT_R(a, b) /* word a, word b, !(a == b == MIN_WORD) */	\
	(SASR( ((longword)(a) * (longword)(b) + 16384), 15 ))

#define	GSM_ADD(a, b)	\
	((ulongword)((ltmp = (longword)(a) + (longword)(b)) - MIN_WORD) > \
		MAX_WORD - MIN_WORD ? (ltmp > 0 ? MAX_WORD : MIN_WORD) : ltmp)

# define GSM_SUB(a, b)	\
	((ltmp = (longword)(a) - (longword)(b)) >= MAX_WORD \
	? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)

/*
 * Adapted from add.c gsm_asl and it's call to gsm_asr (Tursi)
 * gsm_asr becomes a straight shift because we've already rangechecked 0-15
 * validated with truth table, n={-20, -16, -8, 0, 8, 16, 20}, for +a and -a
 */
#define GSM_ASL(a, n) \
	((n) >= 16 ? 0 : (n) >= 0 ? ((a)<<(n)) : (n) <= (-16) ? (-((a)<0)) : ((a)>>(-(n))) )
/* GSM_ASR is much the same, but the other way */
#define GSM_ASR(a, n) \
	( (n) >= 16 ? (-((a)<0)) : (n) >= 0 ? ((a)>>(n)) : (n) <= (-16) ? 0 : ((a)<<(-(n))) )

void Gsm_Decoder(word * LARcr, word * Ncr, word * bcr, word * Mcr, word * xmaxcr, word * xMcr, word *s, unsigned char *f);
void Gsm_Long_Term_Synthesis_Filtering(word Ncr, word bcr, word *erp, word *drp);
void Gsm_RPE_Decoding(word xmaxcr, word Mcr, word *xMcr, word *erp);
void Gsm_Short_Term_Synthesis_Filter(word *LARcr, word *drp, word *s);
void Postprocessing(register word *s, unsigned char *f);
void RPE_grid_positioning(word Mc, register word *xMp, register word *ep);
void APCM_quantization_xmaxc_to_exp_mant(word xmaxc, word *exp_out, word *mant_out);
void APCM_inverse_quantization(register word *xMc, word mant, word exp, register word *xMp);
void Decoding_of_the_coded_Log_Area_Ratios(word *LARc, word *LARpp);
void Coefficients_0_12(register word *LARpp_j_1, register word *LARpp_j, register word *LARp);
void Coefficients_13_26(register word *LARpp_j_1, register word *LARpp_j, register word *LARp);
void Coefficients_27_39(register word *LARpp_j_1, register word *LARpp_j, register word *LARp);
void Coefficients_40_159(register word *LARpp_j, register word * LARp);
void LARp_to_rp(register word * LARp);
void Short_term_synthesis_filtering(register word *rrp, register int k, register word *wt, register word *sr);

word  		LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];
gsm_signal	work[160];

/* ************ Tables ****************** */
/*  Table 4.1  Quantization of the Log.-Area Ratios
 */
/* i 		     1      2      3        4      5      6        7       8 */
const word gsm_A[8]   = {20480, 20480, 20480,  20480,  13964,  15360,   8534,  9036};
const word gsm_B[8]   = {    0,     0,  2048,  -2560,     94,  -1792,   -341, -1144};
const word gsm_MIC[8] = { -32,   -32,   -16,    -16,     -8,     -8,     -4,    -4 };
const word gsm_MAC[8] = {  31,    31,    15,     15,      7,      7,      3,     3 };

/*  Table 4.2  Tabulation  of 1/A[1..8]
 */
const word gsm_INVA[8]={ 13107, 13107,  13107, 13107,  19223, 17476,  31454, 29708 };

/*   Table 4.3a  Decision level of the LTP gain quantizer
 */
/*  bc		      0	        1	  2	     3			*/
const word gsm_DLB[4] = {  6554,    16384,	26214,	   32767	};

/*   Table 4.3b   Quantization levels of the LTP gain quantizer
 */
/* bc		      0          1        2          3			*/
const word gsm_QLB[4] = {  3277,    11469,	21299,	   32767	};

/*   Table 4.4	 Coefficients of the weighting filter
 */
/* i		    0      1   2    3   4      5      6     7   8   9    10  */
const word gsm_H[11] = {-134, -374, 0, 2054, 5741, 8192, 5741, 2054, 0, -374, -134 };

/*   Table 4.5 	 Normalized inverse mantissa used to compute xM/xmax 
 */
/* i		 	0        1    2      3      4      5     6      7   */
const word gsm_NRFAC[8] = { 29128, 26215, 23832, 21846, 20165, 18725, 17476, 16384 };

/*   Table 4.6	 Normalized direct mantissa used to compute xM/xmax
 */
/* i                  0      1       2      3      4      5      6      7   */
const word gsm_FAC[8]	= { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };

/* ************* Implementation ***************** */
gsm gsm_init ()
{
	memset((char *)&sGSM, 0, sizeof(sGSM));
	sGSM.nrp = 40;
	return &sGSM;
}

int gsm_decode (gsm_byte * c, unsigned char * target)
{
	if (((*c >> 4) & 0x0F) != GSM_MAGIC) return -1;

	LARc[0]  = (*c++ & 0xF) << 2;		/* 1 */
	LARc[0] |= (*c >> 6) & 0x3;
	LARc[1]  = *c++ & 0x3F;
	LARc[2]  = (*c >> 3) & 0x1F;
	LARc[3]  = (*c++ & 0x7) << 2;
	LARc[3] |= (*c >> 6) & 0x3;
	LARc[4]  = (*c >> 2) & 0xF;
	LARc[5]  = (*c++ & 0x3) << 2;
	LARc[5] |= (*c >> 6) & 0x3;
	LARc[6]  = (*c >> 3) & 0x7;
	LARc[7]  = *c++ & 0x7;
	Nc[0]  = (*c >> 1) & 0x7F;
	bc[0]  = (*c++ & 0x1) << 1;
	bc[0] |= (*c >> 7) & 0x1;
	Mc[0]  = (*c >> 5) & 0x3;
	xmaxc[0]  = (*c++ & 0x1F) << 1;
	xmaxc[0] |= (*c >> 7) & 0x1;
	xmc[0]  = (*c >> 4) & 0x7;
	xmc[1]  = (*c >> 1) & 0x7;
	xmc[2]  = (*c++ & 0x1) << 2;
	xmc[2] |= (*c >> 6) & 0x3;
	xmc[3]  = (*c >> 3) & 0x7;
	xmc[4]  = *c++ & 0x7;
	xmc[5]  = (*c >> 5) & 0x7;
	xmc[6]  = (*c >> 2) & 0x7;
	xmc[7]  = (*c++ & 0x3) << 1;		/* 10 */
	xmc[7] |= (*c >> 7) & 0x1;
	xmc[8]  = (*c >> 4) & 0x7;
	xmc[9]  = (*c >> 1) & 0x7;
	xmc[10]  = (*c++ & 0x1) << 2;
	xmc[10] |= (*c >> 6) & 0x3;
	xmc[11]  = (*c >> 3) & 0x7;
	xmc[12]  = *c++ & 0x7;
	Nc[1]  = (*c >> 1) & 0x7F;
	bc[1]  = (*c++ & 0x1) << 1;
	bc[1] |= (*c >> 7) & 0x1;
	Mc[1]  = (*c >> 5) & 0x3;
	xmaxc[1]  = (*c++ & 0x1F) << 1;
	xmaxc[1] |= (*c >> 7) & 0x1;
	xmc[13]  = (*c >> 4) & 0x7;
	xmc[14]  = (*c >> 1) & 0x7;
	xmc[15]  = (*c++ & 0x1) << 2;
	xmc[15] |= (*c >> 6) & 0x3;
	xmc[16]  = (*c >> 3) & 0x7;
	xmc[17]  = *c++ & 0x7;
	xmc[18]  = (*c >> 5) & 0x7;
	xmc[19]  = (*c >> 2) & 0x7;
	xmc[20]  = (*c++ & 0x3) << 1;
	xmc[20] |= (*c >> 7) & 0x1;
	xmc[21]  = (*c >> 4) & 0x7;
	xmc[22]  = (*c >> 1) & 0x7;
	xmc[23]  = (*c++ & 0x1) << 2;
	xmc[23] |= (*c >> 6) & 0x3;
	xmc[24]  = (*c >> 3) & 0x7;
	xmc[25]  = *c++ & 0x7;
	Nc[2]  = (*c >> 1) & 0x7F;
	bc[2]  = (*c++ & 0x1) << 1;		/* 20 */
	bc[2] |= (*c >> 7) & 0x1;
	Mc[2]  = (*c >> 5) & 0x3;
	xmaxc[2]  = (*c++ & 0x1F) << 1;
	xmaxc[2] |= (*c >> 7) & 0x1;
	xmc[26]  = (*c >> 4) & 0x7;
	xmc[27]  = (*c >> 1) & 0x7;
	xmc[28]  = (*c++ & 0x1) << 2;
	xmc[28] |= (*c >> 6) & 0x3;
	xmc[29]  = (*c >> 3) & 0x7;
	xmc[30]  = *c++ & 0x7;
	xmc[31]  = (*c >> 5) & 0x7;
	xmc[32]  = (*c >> 2) & 0x7;
	xmc[33]  = (*c++ & 0x3) << 1;
	xmc[33] |= (*c >> 7) & 0x1;
	xmc[34]  = (*c >> 4) & 0x7;
	xmc[35]  = (*c >> 1) & 0x7;
	xmc[36]  = (*c++ & 0x1) << 2;
	xmc[36] |= (*c >> 6) & 0x3;
	xmc[37]  = (*c >> 3) & 0x7;
	xmc[38]  = *c++ & 0x7;
	Nc[3]  = (*c >> 1) & 0x7F;
	bc[3]  = (*c++ & 0x1) << 1;
	bc[3] |= (*c >> 7) & 0x1;
	Mc[3]  = (*c >> 5) & 0x3;
	xmaxc[3]  = (*c++ & 0x1F) << 1;
	xmaxc[3] |= (*c >> 7) & 0x1;
	xmc[39]  = (*c >> 4) & 0x7;
	xmc[40]  = (*c >> 1) & 0x7;
	xmc[41]  = (*c++ & 0x1) << 2;
	xmc[41] |= (*c >> 6) & 0x3;
	xmc[42]  = (*c >> 3) & 0x7;
	xmc[43]  = *c++ & 0x7;			/* 30  */
	xmc[44]  = (*c >> 5) & 0x7;
	xmc[45]  = (*c >> 2) & 0x7;
	xmc[46]  = (*c++ & 0x3) << 1;
	xmc[46] |= (*c >> 7) & 0x1;
	xmc[47]  = (*c >> 4) & 0x7;
	xmc[48]  = (*c >> 1) & 0x7;
	xmc[49]  = (*c++ & 0x1) << 2;
	xmc[49] |= (*c >> 6) & 0x3;
	xmc[50]  = (*c >> 3) & 0x7;
	xmc[51]  = *c & 0x7;			/* 33 */

	Gsm_Decoder(LARc, Nc, bc, Mc, xmaxc, xmc, work, target);

	return 0;
}

void Gsm_Decoder(	word		* LARcr,	/* [0..7]		IN	*/
					word		* Ncr,		/* [0..3] 		IN 	*/
					word		* bcr,		/* [0..3]		IN	*/
					word		* Mcr,		/* [0..3] 		IN 	*/
					word		* xmaxcr,	/* [0..3]		IN 	*/
					word		* xMcr,		/* [0..13*4]	IN	*/
					word		* s,		/* [0..159]		TEMP*/
					unsigned char *f		/* [0..159]     OUT */
				)
{
	int		j, k, idx;
	word	erp[40], wt[160];
	word*	drp = sGSM.dp0 + 120;

	/* Calculate j offset outside of this loop */
	idx = 0;
	for (j=0; j <= 3; j++, xmaxcr++, bcr++, Ncr++, Mcr++, xMcr += 13) {

		Gsm_RPE_Decoding(*xmaxcr, *Mcr, xMcr, erp );
		Gsm_Long_Term_Synthesis_Filtering(*Ncr, *bcr, erp, drp );

		for (k = 0; k <= 39; k++) wt[ idx++ ] =  drp[ k ];
	}

	Gsm_Short_Term_Synthesis_Filter(LARcr, wt, s );
	Postprocessing(s, f);
}

void Gsm_RPE_Decoding(	word 		xmaxcr,
						word		Mcr,
						word		* xMcr,		/* [0..12], 3 bits 	IN	*/
						word		* erp		/* [0..39]			OUT */
					)
{
	word	exp, mant;
	word	xMp[ 13 ];

	APCM_quantization_xmaxc_to_exp_mant( xmaxcr, &exp, &mant );
	APCM_inverse_quantization( xMcr, mant, exp, xMp );
	RPE_grid_positioning( Mcr, xMp, erp );
}

void APCM_quantization_xmaxc_to_exp_mant (	word	xmaxc,		/* IN 	*/
											word	* exp_out,	/* OUT	*/
											word	* mant_out 	/* OUT  */
										)
{
	word	exp, mant;

	/* Compute exponent and mantissa of the decoded version of xmaxc
	 */

	exp = 0;
	if (xmaxc > 15) exp = SASR(xmaxc, 3) - 1;
	mant = xmaxc - (exp << 3);

	if (mant == 0) {
		exp  = -4;
		mant = 7;
	}
	else {
		while (mant <= 7) {
			mant = mant << 1 | 1;
			exp--;
		}
		mant -= 8;
	}

	*exp_out  = exp;
	*mant_out = mant;
}

void APCM_inverse_quantization (	register word	* xMc,	/* [0..12]			IN 	*/
									word		mant,
									word		exp,
									register word	* xMp	/* [0..12]			OUT 	*/
								)
/* 
 *  This part is for decoding the RPE sequence of coded xMc[0..12]
 *  samples to obtain the xMp[0..12] array.  Table 4.6 is used to get
 *  the mantissa of xmaxc (FAC[0..7]).
 */
{
	int	i;
	word	temp, temp1, temp2, temp3, temp4;
	longword	ltmp;	/* for GSM_SUB */

	temp1 = gsm_FAC[ mant ];	/* see 4.2-15 for mant */
	temp2 = GSM_SUB( 6, exp );	/* see 4.2-15 for exp  */
	temp4 = GSM_SUB( temp2, 1 );	/* broken out by Tursi for the macro */
	temp3 = GSM_ASL( 1, temp4 );
//	temp3 = gsm_asl( 1, gsm_sub( temp2, 1 ));

	for (i = 13; i--;) {
		temp = (*xMc++ << 1) - 7;	/* restore sign   */

		temp <<= 12;				/* 16 bit signed  */
		temp = GSM_MULT_R( temp1, temp );
		temp = GSM_ADD( temp, temp3 );
		*xMp++ = GSM_ASR( temp, temp2 );
//		*xMp++ = gsm_asr( temp, temp2 );
	}
}

void RPE_grid_positioning	(	word			Mc,		/* grid position	IN	*/
								register word	* xMp,	/* [0..12]			IN	*/
								register word	* ep	/* [0..39]			OUT	*/
							)
/*
 *  This procedure computes the reconstructed long term residual signal
 *  ep[0..39] for the LTP analysis filter.  The inputs are the Mc
 *  which is the grid position selection and the xMp[0..12] decoded
 *  RPE samples which are upsampled by a factor of 3 by inserting zero
 *  values.
 */
{
	/* Tursi: This is friggin' **SCARY** code. On entry, the switch does just what you'd    */
	/* expect. The 'do' is basically just a label for the 'while' to jump to, so it doesn't */
	/* matter whether it's executed or not, the while still correctly branches, in the      */
	/* middle of the switch. Not how Jim Kirk would've done it.                             */

	/* Thus, Mc decides what to do. It ranges 0-3 (verified by assert): */
	/* 0: copy one byte from xMp, then loop 12 more times, each setting a three byte pattern */
	/*    of 0, 0, *xMp. This writes 37 bytes to ep.                                         */
	/* 1: set one zero, then copy one byte, then loop 12 times. This writes 38 bytes.        */
	/* 2: write 13 full sets of 0, 0, *xMp. This writes 39 bytes.                            */
	/* 3: write one zero, then 13 sets of 0, 0, *xMp. This writes 40 bytes.                  */


	int	i = 13;

	switch (Mc) {
            case 3: *ep++ = 0;
            case 2:  do {
                            *ep++ = 0;
            case 1:         *ep++ = 0;
            case 0:         *ep++ = *xMp++;
                        } while (--i);
    }

#if 0
    int i;
	/* Tursi's version - simpler code, maybe easier to optimize? */
	switch (Mc) {
		case 3: *ep++ = 0;
		case 2: *ep++ = 0;
		case 1: *ep++ = 0;
		case 0: *ep++ = *xMp++;
	}
	for (i=12; i; i--) {
		*ep++ = 0;
		*ep++ = 0;
		*ep++ = *xMp++;
	}
	/* End Tursi's version */
#endif
	/* This makes sure we write out however many padding bytes we need to reach 40 */
	while (++Mc < 4) *ep++ = 0;
}

void Gsm_Long_Term_Synthesis_Filtering	(	word				Ncr,
											word				bcr,
											register word		* erp,	   /* [0..39]		  	 IN */
											register word		* drp	   /* [-120..-1] IN, [-120..40] OUT */
										)
/*
 *  This procedure uses the bcr and Ncr parameter to realize the
 *  long term synthesis filtering.  The decoding of bcr needs
 *  table 4.3b.
 */
{
	register longword	ltmp;	/* for ADD */
	register int 		k;
	word				brp, drpp, Nr;

	/*  Check the limits of Nr.
	 */
	Nr = Ncr < 40 || Ncr > 120 ? sGSM.nrp : Ncr;
	sGSM.nrp = Nr;

	/*  Decoding of the LTP gain bcr
	 */
	brp = gsm_QLB[ bcr ];

	/*  Computation of the reconstructed short term residual 
	 *  signal drp[0..39]
	 */
	for (k = 0; k <= 39; k++) {
		drpp   = GSM_MULT_R( brp, drp[ k - Nr ] );
		drp[k] = GSM_ADD( erp[k], drpp );
	}

	/*
	 *  Update of the reconstructed short term residual signal
	 *  drp[ -1..-120 ]
	 */
	for (k = 0; k <= 119; k++) drp[ -120 + k ] = drp[ -80 + k ];
}

void Gsm_Short_Term_Synthesis_Filter	(	word	* LARcr,	/* received log area ratios [0..7]	IN  */
											word	* wt,		/* received d [0..159]				IN  */
											word	* s			/* signal   s [0..159]				OUT  */
										)
{
	word		* LARpp_j	= sGSM.LARpp[ sGSM.j     ];
	word		* LARpp_j_1	= sGSM.LARpp[ sGSM.j ^=1 ];
	word		LARp[8];

	Decoding_of_the_coded_Log_Area_Ratios( LARcr, LARpp_j );

	Coefficients_0_12( LARpp_j_1, LARpp_j, LARp );
	LARp_to_rp( LARp );
	Short_term_synthesis_filtering( LARp, 13, wt, s );

	Coefficients_13_26( LARpp_j_1, LARpp_j, LARp);
	LARp_to_rp( LARp );
	Short_term_synthesis_filtering(LARp, 14, wt + 13, s + 13 );

	Coefficients_27_39( LARpp_j_1, LARpp_j, LARp);
	LARp_to_rp( LARp );
	Short_term_synthesis_filtering(LARp, 13, wt + 27, s + 27 );

	Coefficients_40_159( LARpp_j, LARp );
	LARp_to_rp( LARp );
	Short_term_synthesis_filtering(LARp, 120, wt + 40, s + 40);
}

void Decoding_of_the_coded_Log_Area_Ratios	(	word	* LARc,		/* coded log area ratio	[0..7] 	IN	*/
														word	* LARpp		/* out: decoded ..			*/
													)
{
	register word	temp1;
	register long	ltmp;	/* for GSM_ADD */

	/*  This procedure requires for efficient implementation
	 *  two tables.
 	 *
	 *  INVA[1..8] = integer( (32768 * 8) / real_A[1..8])
	 *  MIC[1..8]  = minimum value of the LARc[1..8]
	 */

	/*  Compute the LARpp[1..8]
	 */

#define	STEP( B, MIC, INVA )	\
		temp1    = GSM_ADD( *LARc++, MIC ) << 10;	\
		temp1    = GSM_SUB( temp1, B << 1 );		\
		temp1    = GSM_MULT_R( INVA, temp1 );		\
		*LARpp++ = GSM_ADD( temp1, temp1 );

	STEP(      0,  -32,  13107 );
	STEP(      0,  -32,  13107 );
	STEP(   2048,  -16,  13107 );
	STEP(  -2560,  -16,  13107 );

	STEP(     94,   -8,  19223 );
	STEP(  -1792,   -8,  17476 );
	STEP(   -341,   -4,  31454 );
	STEP(  -1144,   -4,  29708 );

	/* NOTE: the addition of *MIC is used to restore
	 * 	 the sign of *LARc.
	 */
}

/*
 *  Within each frame of 160 analyzed speech samples the short term
 *  analysis and synthesis filters operate with four different sets of
 *  coefficients, derived from the previous set of decoded LARs(LARpp(j-1))
 *  and the actual set of decoded LARs (LARpp(j))
 *
 * (Initial value: LARpp(j-1)[1..8] = 0.)
 */

void Coefficients_0_12	(	register word * LARpp_j_1,
							register word * LARpp_j,
							register word * LARp
						)
{
	register int 	i;
	register longword ltmp;

	for (i = 1; i <= 8; i++, LARp++, LARpp_j_1++, LARpp_j++) {
		*LARp = GSM_ADD( SASR( *LARpp_j_1, 2 ), SASR( *LARpp_j, 2 ));
		*LARp = GSM_ADD( *LARp,  SASR( *LARpp_j_1, 1));
	}
}

void Coefficients_13_26	(	register word * LARpp_j_1,
							register word * LARpp_j,
							register word * LARp
						)
{
	register int i;
	register longword ltmp;
	for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
		*LARp = GSM_ADD( SASR( *LARpp_j_1, 1), SASR( *LARpp_j, 1 ));
	}
}

void Coefficients_27_39	(	register word * LARpp_j_1,
							register word * LARpp_j,
							register word * LARp
						)
{
	register int i;
	register longword ltmp;

	for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
		*LARp = GSM_ADD( SASR( *LARpp_j_1, 2 ), SASR( *LARpp_j, 2 ));
		*LARp = GSM_ADD( *LARp, SASR( *LARpp_j, 1 ));
	}
}

void Coefficients_40_159	(	register word * LARpp_j,
								register word * LARp
							)
{
	register int i;

	for (i = 1; i <= 8; i++, LARp++, LARpp_j++)
		*LARp = *LARpp_j;
}

void LARp_to_rp(register word * LARp)	/* [0..7] IN/OUT  */
/*
 *  The input of this procedure is the interpolated LARp[0..7] array.
 *  The reflection coefficients, rp[i], are used in the analysis
 *  filter and in the synthesis filter.
 */
{
	register int 		i;
	register word		temp;
	register longword	ltmp;

	for (i = 1; i <= 8; i++, LARp++) {
		if (*LARp < 0) {
			temp = *LARp == MIN_WORD ? MAX_WORD : -(*LARp);
			*LARp = - ((temp < 11059) ? temp << 1
				: ((temp < 20070) ? temp + 11059
				:  GSM_ADD( temp >> 2, 26112 )));
		} else {
			temp  = *LARp;
			*LARp =    (temp < 11059) ? temp << 1
				: ((temp < 20070) ? temp + 11059
				:  GSM_ADD( temp >> 2, 26112 ));
		}
	}
}

void Short_term_synthesis_filtering  (  register word	* rrp,   /* [0..7]	    IN	*/
                                       	register int	k,	     /* k_end - k_start	*/
                                       	register word	* wt,	 /* [0..k-1]	IN	*/
                                       	register word	* sr	 /* [0..k-1]	OUT	*/
                                     )
{
	register word		* v = sGSM.v;
	register int		i;
	register word		sri, tmp1, tmp2;
	register longword	ltmp;	/* for GSM_ADD  & GSM_SUB */

	while (k--) {
		sri = *wt++;
		for (i = 8; i--;) {
			tmp1 = rrp[i];
			tmp2 = v[i];
			tmp2 =  ( tmp1 == MIN_WORD && tmp2 == MIN_WORD
				? MAX_WORD
				: 0x0FFFF & (( (longword)tmp1 * (longword)tmp2
					     + 16384) >> 15)) ;

			sri  = GSM_SUB( sri, tmp2 );

			tmp1  = ( tmp1 == MIN_WORD && sri == MIN_WORD
				? MAX_WORD
				: 0x0FFFF & (( (longword)tmp1 * (longword)sri
					     + 16384) >> 15)) ;

			v[i+1] = GSM_ADD( v[i], tmp1);
		}
		*sr++ = v[0] = sri;
	}
}

void Postprocessing	(	register word 		* s,
						unsigned char       * f
					)
{
	register int		k;
	register word		msr = sGSM.msr;
	register longword	ltmp;	/* for GSM_ADD */
	register word		tmp;

	for (k = 160; k--; ) {
		tmp = GSM_MULT_R( msr, 28180 );
		msr = GSM_ADD(*(s++), tmp);  	   /* Deemphasis 	     */
		*(f++) = (unsigned char)(GSM_ADD(msr, msr) >> 8);  /* Truncation & Upscaling, cut to 8 bit */
	}
	sGSM.msr = msr;
}

