//
// (C) 2003 Mike Brent aka Tursi aka HarmlessLion.com
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
#include "tursigb.h"
#include "gsm_short.h"
#include "sanddemo.h"
#include "random.h"

extern u16* VideoBuf;
extern volatile u32 nFrame;
extern const char *pDesert;
extern u32 nDesertOff;
extern struct _stars Star[];
extern const u16 sand2_gfx[];
extern const u16 sand2_pal[];
extern const u16 font2_gfx[];
extern const u16 font2_pal[];
extern const signed short SINE[];

void srand(unsigned int newseed);

/* data (not in RAM) */
const struct imgdata SandLogo[] = {
#include "dos\sand.txt"
	{ -1,-1, 0 }
};
const char Story[]={
    "                "
    "HEY, TURSI HERE, WITH SAND II! OKAY, THERES NO PICTURES LIKE LAST TIME ON THE PSX, "
    "AND MAYBE THEY LOOK A LOT LIKE STARS, BUT DAMN IT, I WAS INSPIRED BY SAND, SO SAND I"
    "T IS! IF YOURE RUNNING THIS IN AN EMULATOR, IT WILL PROBABLY SUCK. THIS DEMO USES IN"
    "TERRUPTS HEAVILY, AND EMULATORS STRUGGLE TO GET THE TIMING RIGHT. ESPECIALLY, THE TI"
    "TLE PAGE MOST LIKELY DIDNT LOOK RIGHT UNLESS YOU CAN RUN FULL SPEED WITH NO FRAMESKI"
    "P.  NOW, ALL ABOUT THE DEMO. THE TURSI LOGO WAS DONE BY THE AWESOME VIVO, WHO I WAS "
    "VISITING IN AUSTRALIA WHEN I TOOK THE MORETON ISLAND PIC. THE PIC IS MODE 3 15-BIT C"
    "OLOR AND PIXEL SCROLLED USING STANDARD DMA. THE AUDIO IS PROVIDED BY MY HEAVILY MODI"
    "FIED LION DECODER, WHICH PROVIDES 5 TO 1 COMPRESSION AT 8 BITS AND DECENT PLAYBACK. "
    "THIS IS THE ENTIRE RADIO MIX OF SANDSTORM, AT 16KHZ, IN 734K AND LESS THAN 30 PERCEN"
    "T CPU AND ENTIRELY INTERRUPT DRIVEN. THE STARFI... I MEAN SAND STORM, IS BASED ON MY"
    " PC FUNKYSTARS CODE, OPTIMIZED AND SCALED DOWN A BIT FOR INTEGER MATH. SCROLL TEXT I"
    "S DRIVEN ENTIRELY BY TIMER INTERRUPT. I USED THE VISUAL HAM IDE, THOUGH NOT HAM ITSE"
    "LF. ALL MY OWN CODE EXCEPT FOR J.FROHWEINS FAST DIVIDE AND STARTUP CODE. USED PANORA"
    "MA FACTORY FOR THE PIC AND BIN2INC BY VAN HELSING FOR THE DATA. THANKS TO RABITGUY. "
    "HI TO FOXXFIRE, MARJAN, RIMMY, LOSER, BINKY, AND FLIPPER! BYE NOW!    "//41--
    "                 "
    "\x80"     // triggers the stars to end
    "                "
    "\x81"     // second one exits altogether
};
const char Font[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!?., ";
const int nPulsedData[7]= { 0x2455, 0x2501, 0x2537, 0x2584, 0x25b6, 0x271A, 0 };

#define STARSPAUSE (0x21f1-100)
#define STARSRESUME 0x3A39

/* vars */
signed int sprX[32], sprY[32];
const char *pStory=Story;
int sprstart=234;       // counter times the appearance of the scrolltext

// FastRAM version of strchr()
signed int CODE_IN_IWRAM StrChr(const char *p, char x) {
    const char *t;

    t=p;
    while ((*t)&&(*t != x)) t++;
    if (*t) {
        return t-p;
    } else {
        return -1;
    }
}

// Interrupt called - move the scrolltext
void CODE_IN_IWRAM DoScrollText() {
    int idx;
    signed int tx,ty;
    static int sinpos=0, tmppos=0;
    
    // move the scrolltext sprites
    for (idx=0; idx<32; idx++) {
        sprX[idx]=sprX[idx]-4;
        if (sprX[idx]<0) {
            sprX[idx]=sprX[idx]+512;
        } else {
            if (sprX[idx]==496) {
                tx=*(pStory++);
                if (tx&0x80) {
                    tx=40;
                    pStory--;       // Hold on the last char
                } else {
                    tx=StrChr(Font, tx);
                    if (tx == -1) {
                        tx=40;
                    }
                }
                ty=(tx>>4)<<4;
                SPR_SETTILE(idx, (ty<<2)+((tx-ty)<<1)+512);
            }
        }
        SPR_SETX(idx, sprX[idx]);
        sprY[idx]=(SINE[(sinpos+(sprX[idx]<<2))&0x3ff]>>9)+72;
        SPR_SETY(idx, sprY[idx]);
        tmppos++;
        if (tmppos==6) {
            tmppos=0;
            sinpos++;
        }
    }
}

// Do an integer division using GBA BIOS fast division SWI code.
//  by Jeff F., 2002-Apr-27
// May not work correctly with vars on the stack?
void CODE_IN_IWRAM FastIntDivide (s32 Numer, s32 Denom, s32 *Result, s32 *Remainder)
{
   asm volatile
      (
      " mov   r0,%2   \n"
      " mov   r1,%3   \n"
      " swi   0x60000       \n"     // NOTE!!!!! Put 6 here for Thumb C Compiler mode.
      " ldr   r2,%0   \n"           //           Put 0x60000 there for ARM C Compiler mode.
      " str   r0,[r2] \n"
      " ldr   r2,%1   \n"
      " str   r1,[r2] \n"
      : "=m" (Result), "=m" (Remainder) // Outputs
      : "r" (Numer), "r" (Denom)        // Inputs
      : "r0","r1","r2","r3"             // Regs crushed & smushed
      );
}

// Handle a simple interrupt
void CODE_IN_IWRAM InterruptProcess(void) {
    register u32 tmp=REG_IF;

    // disable interrupts
    REG_IME = 0;

    if (tmp & INT_VBLANK) {
        nFrame++;
   	}
   	
   	if ((tmp & INT_TIMER3)&&(0 == sprstart)) {
   	    DoScrollText();
    }
   	
    /* acknowledge to BIOS, for IntrWait() */
//    *(volatile unsigned short *)0x03fffff8 |= tmp;

    // clear *all* flagged ints except timer 1, which we check below (writing a 1 clears that bit)
    // When we clear the bits, those ints are allowed to trigger again   	
    REG_IF = ~INT_TIMER1;

    // Re-enable interrupts
    REG_IME = 1;

    // This one can take a while - we'll allow other ints now
	if (tmp & INT_TIMER1) {
        process_decode();
        // clear the interrupt only after we're done. If we were too slow,
        // we may have missed interrupts!
        REG_IF=INT_TIMER1;  // writing zeros is ignored
    }
}

void CODE_IN_IWRAM DrawFrame() {
    register int dest;
    register const char *src;

    // Never do non-aligned DMA - can cause hangs under load (doesn't work right anyway)
    if (nDesertOff&1) return;

    src=&pDesert[nDesertOff];

    // Do a scroll (can't use auto DMA cause of the offset)
    for (dest=0; dest<38400; dest+=240) {
        // Write in 16-bit chunks to permit single-pixel scrolling
        fastcopy16(&VideoBuf[dest], (void*)src, 480);
        src+=2048;
    }
}

// Chosen because this is 32*96, and 96 is our palette size, used for the color calc
#define UNISIZE 3072
// Set 8-bit mode, set up sand, fade in from white, do a twisting sandstorm
signed int tx, ty, tt;
void CODE_IN_IWRAM doStorm(void) {
    signed int fade, title, framecount, curframe;
    signed int sinr, cosr;
    signed int idx;
    signed int rs=157, tmprs=0;
    int nPulsedIdx=0;       // used to handle the pulsing
    int nMode=0;

    // Hardware setup //
    // Set the video to mode 4, bg 2 enabled (8-bit bitmap) & sprites
    REG_DISPCNT= MODE4 | BG2_ENABLE | SPRITE_ENABLE | SPRITES_2D;
    REG_BLDMOD= SOURCE_BG2 | LIGHTEN | TARGET_BG2;

    // Clear the screen (240x160x8)
    // Can only do 32k at a time with DMA, so do 2 DMAs
    fastset(VideoBuf, 0x01010101, 19200);
    fastset(&VideoBuf[9600], 0x01010101, 19200);     /* VideoBuf is a u16*, so 1/2 the number of words */

    // Setup stars
    for (idx=0; idx<MAX_SAND; idx++) {
        Star[idx].x=rand()%UNISIZE-(UNISIZE/2);
        Star[idx].y=rand()%UNISIZE-(UNISIZE/2);
        Star[idx].z=rand()%UNISIZE;
        Star[idx].oldoffset=60+80*120;  // the compiler solves this one, it's a const
        Star[idx].targetx=-1;
    }

    // Setup a nice palette
    for (idx=0; idx<32; idx++) {        // first gray 
        BG_PALETTE[idx]=((idx)<<10)|((idx)<<5)|(idx);
    }

    // Reverse palette - bright -> dark
    // Fade from full red to full green
    for (idx=0; idx<32; idx++) {
        BG_PALETTE[idx+32]=(0<<10)|(idx<<5)|(31-idx);
    }
    // Fade green most of the way to blue
    for (idx=0; idx<24; idx++) {
        BG_PALETTE[idx+64]=(((idx))<<10)|((31-idx)<<5)|((idx)>>2);
    }
    // Finish fading in blue, but hold the green and red steady (for the bright blue)
    for (idx=24; idx<32; idx++) {
        BG_PALETTE[idx+64]=(((idx))<<10)|(7<<5)|((idx)>>2);
    }
    // Fade from bright blue down to black
    for (idx=0; idx<32; idx++) {
        BG_PALETTE[idx+96]=((31-idx)<<10)|(((31-idx)>>2)<<5)|((31-idx)>>2);
    }

    // copy the title bmp to the second video buffer - we do it byte by byte
    // We can afford the time as the screen is white, and this way we don't need
    // to preprocess the bitmap to live in the top half of the palette, which none
    // of the tools can do.
    for (idx=0; idx<19200; idx++) {
        *(((u16*)(0x600a000))+idx) = sand2_gfx[idx]|0x8080;
    }

    // Copy the 128 color palette into the top half of the palette range
    fastcopy16(0x5000000+(128*2), sand2_pal, 128*2);
    
    // Finally, get the text font into place in the sprite table
    // the second gfx screen overlaps a bit, so we use the tiles from 512 up
    fastcopy16(0x06010000+(512*32), font2_gfx, 3072*2);
    
    // And it's palette into the first sprite palette
    fastcopy16(SPR_PALETTE, font2_pal, 16*2);

    // bring it on!
    nPulsedIdx=0;
    sprstart=234;
    fade=16;            // counter fades from white to black
    framecount=47;      // handles appearance of title
    title=141;          // counter times the disappearance of the title
    curframe=nFrame;
    pStory=Story;
    
    while (nMode < 9) {
        // Fade down from white to normal (happens several times)
        if (fade>0) {
           fade--;
           REG_COLEY=fade;
        }
        
        switch (nMode) {
            case 0:     // fading
                if (fade == 0) {
                    nMode=1;
                }
                break;

            case 1:     // Counting down framecount (flicker starts after this)
                if (framecount>0) {
                    framecount--;
                } else {
                    nMode=2;
                }
                break;
                
            case 2:     // counting down title (title ends after this)
                if (title>0) {
                    title--;
                } else {
                    // all done?
                    REG_DISPCNT &= ~PAGE2;
                    framecount=-1;
                    nMode=3;
                }
                break;
                
            case 3:     // counting down sprstart
                if (sprstart > 0) {
                    sprstart--;
                    if (sprstart == 0) {
                        // start the scrolltext sprites (32 x 16 = 512)
                        // Normally start text with at least 16 spaces!
                        for (idx=0; idx<32; idx++) {
                            sprX[idx]=idx<<4;
                            sprY[idx]=80;
                            SPR_SETY(idx, sprY[idx]);
                            SPR_ROTSCALEOFF(idx);
                            SPR_DOUBLEOFF(idx);
                            SPR_NORMAL(idx);
                            SPR_MOSAICOFF(idx);
                            SPR_16COLOR(idx);
                            SPR_SETSIZE(idx, SPR_16x16);    // 16x16
                            SPR_SETX(idx, sprX[idx]);
                            SPR_NOFLIPH(idx);
                            SPR_NOFLIPV(idx);
                            tx=*(pStory++);
                            if (tx=='*') {
                                tx=' ';
                                pStory=Story;
                            }
                            tx=StrChr(Font, tx);
                            if (tx == -1) {
                                tx=47;
                            }
                            ty=(tx>>4)<<4;
                            SPR_SETTILE(idx, (ty<<2)+((tx-ty)<<1)+512);
                            SPR_SETPRIORITY(idx, 0);
                            SPR_SETPALETTE(idx, 0);
                        }
                        // Set Timer3 which controls the scrolltext
                        REG_TM3D=0xffff-546;        // approx 30fps
                        REG_TM3CNT=TIMER_16KHZ | TIMER_INTERRUPT | TIMER_ENABLED;
                    }
                } else {
                    nMode=4;
                }
                break;
                
            case 4:     // Waiting for pause
                if (nNextBlock > STARSPAUSE) {
                   nMode = 5;
                }
                break;
                
            case 5:     // Stars paused
                if (nNextBlock >= nPulsedData[nPulsedIdx]) {
                    for (idx=0; idx<100; idx++) {
   	      	           	Star[idx].z=rand()%UNISIZE;
      	          	}
                    nPulsedIdx++;
                    if (0 == nPulsedData[nPulsedIdx]) {
                        fade=16;
                        for (idx=0; idx<MAX_SAND; idx++) {
                            Star[idx].z=rand()%UNISIZE;
                        }
                        nMode=6;
                    }
                }
                break;

            case 6:  // Stars streaking
                if (nNextBlock >= STARSRESUME) {
                    fade=16;
                    // Clear the screen (240x160x8)
                    // Can only do 32k at a time with DMA, so do 2 DMAs
                    fastset(VideoBuf, 0x01010101, 19200);
                    fastset(&VideoBuf[9600], 0x01010101, 19200);     /* VideoBuf is a u16*, so 1/2 the number of words */
                    nMode=7;
                }

            case 7:  // resume normal, faster spinning
                tmprs++;
                if (*pStory == 0x80) {
                    pStory++;       // use scrolltext for additional delay
                    nMode=8;
                }
                break;
                
          case 8: // demo over, cleaning up stars and text
                tmprs++;
                if (*pStory == 0x81) {
                    nMode=9;        // breaks out
                }
                break;
        }

    	/* Speed at which rotation changes */
    	tmprs++;
    	if ((tmprs>1)||((SINE[rs]<1500)&&(SINE[rs]>-1500))) {
    		rs++;
    		rs&=0x3ff;
    		if ((SINE[rs]<1500)&&(SINE[rs]>-1500)) {
                rs++;
        		rs&=0x3ff;
    		}
    		tmprs=0;
    	}

    	/* actual amount to rotate this frame */
        tt=(SINE[rs]>>11)&0x3ff;
    	sinr=SINE[tt];
        cosr=SINE[(tt+256)&0x3ff];  // cosine offset is 256

        for (idx=0; idx<MAX_SAND; idx++) {
            switch (nMode) {
                case 0:     // fading
                case 1:     // framecount
                    // Erase old star
                    VideoBuf[Star[idx].oldoffset]=0x0101;
                    // move star
                    if (Star[idx].targetx != 1) {
            	       	Star[idx].z-=175;
        	       	} else {
                        if (Star[idx].z > 540) {
                            Star[idx].z-=60;
                        } else {
                            Star[idx].z=480;
                        }
                    }
                    break;

                case 2:     // title
                    /* check on buffer flicker */
                    if (curframe != nFrame) {
                        curframe=nFrame;
                        if (nFrame & 0x1) {
                           REG_DISPCNT |= PAGE2;
                        } else {
                           REG_DISPCNT &= ~PAGE2;
                        }
                    }
                    // Erase old star
                    VideoBuf[Star[idx].oldoffset]=0x0101;
                    // move star
                    if (Star[idx].targetx != 1) {
            	       	Star[idx].z-=175;
        	       	} else {
                        if (Star[idx].z > 540) {
                            Star[idx].z-=60;
                        } else {
                            Star[idx].z=480;
                        }
                    }
                    break;
                    
                case 3:     // sprstart
                    // Erase old star
                    VideoBuf[Star[idx].oldoffset]=0x0101;
            		// rotate star
                    Star[idx].x=((Star[idx].x*cosr)-(Star[idx].y*sinr))>>15;
                    Star[idx].y=((Star[idx].x*sinr)+(Star[idx].y*cosr))>>15;
                    // move star
                    if (Star[idx].targetx != 1) {
            	       	Star[idx].z-=175;
        	       	} else {
    	       	        Star[idx].z+=120;
    	       	        if (Star[idx].z >= UNISIZE) {
                            Star[idx].z=rand()%UNISIZE;
                            Star[idx].targetx=-1;
                        }
                    }
                    break;
                    
                case 4:     // wait for pause
                case 7:     // resume normal
                    // Erase old star
                    VideoBuf[Star[idx].oldoffset]=0x0101;
            		// rotate star
                    Star[idx].x=((Star[idx].x*cosr)-(Star[idx].y*sinr))>>15;
                    Star[idx].y=((Star[idx].x*sinr)+(Star[idx].y*cosr))>>15;
                    // Move star
        	       	Star[idx].z-=175;
                    break;

                case 5:     // paused
                    // Erase old star
                    VideoBuf[Star[idx].oldoffset]=0x0101;
            		// rotate star
                    Star[idx].x=((Star[idx].x*cosr)-(Star[idx].y*sinr))>>15;
                    Star[idx].y=((Star[idx].x*sinr)+(Star[idx].y*cosr))>>15;
                    // Move star
                    if (Star[idx].z < UNISIZE) {
            	       	Star[idx].z-=175;
        	       	}
                    break;
                    
                case 6:     // streaking
            		// rotate star
                    Star[idx].x=((Star[idx].x*cosr)-(Star[idx].y*sinr))>>15;
                    Star[idx].y=((Star[idx].x*sinr)+(Star[idx].y*cosr))>>15;
                    // Move star
        	       	Star[idx].z-=175;
                    break;
                    
                 case 8:    // wait for end
                    // Erase old star
                    VideoBuf[Star[idx].oldoffset]=0x0101;
            		// rotate star
                    Star[idx].x=((Star[idx].x*cosr)-(Star[idx].y*sinr))>>15;
                    Star[idx].y=((Star[idx].x*sinr)+(Star[idx].y*cosr))>>15;
                    // Move star
                    if (Star[idx].z < UNISIZE) {
            	       	Star[idx].z-=175;
        	       	}
                    break;

            }

    		if (Star[idx].z < 1)
	       	{
	       	    if ((nMode != 5)&&(nMode != 8)) {
                    int v1, v2;
                    static signed int w1=123, w2=6400;

	      	      	Star[idx].z+=UNISIZE;

                    // Lissajous style
                    if ((nFrame&0xff) == 0) {  // 256 frames, change pattern
                        w1=rand();
                        w2=rand();
                    }

                    if ((nMode < 3)&&(idx < (sizeof(SandLogo)/sizeof(struct imgdata)))) {
                        Star[idx].x=((SandLogo[idx].x-120)>>1)*30;   // the '30' is hand-tuned to match the BMP
                        Star[idx].y=(SandLogo[idx].y-80)*30;         // it can't be made '32'.
                        Star[idx].targetx=1;
                    } else {
                        v1=w1*nFrame+1;
                        v1&=0x3ff;
                        v2=w2*nFrame+268;
                        v2&=0x3ff;
                        Star[idx].x=(SINE[v1]*UNISIZE-(UNISIZE/2))>>15;
                        Star[idx].y=(SINE[v2]*UNISIZE-(UNISIZE/2))>>15;
                    }
	          	} else {
        	        // Centered, in the distance, fatter at the center (random dist)
        	        Star[idx].z=UNISIZE;
        	        Star[idx].x=(idx>>1)-(MAX_SAND/4);
        	        if (Star[idx].x < 0) {
        	            FastIntDivide(rand(), ((MAX_SAND/4)+Star[idx].x)|1, &tt, &Star[idx].y);
            	        Star[idx].y-=(((MAX_SAND/4)+Star[idx].x)|1)>>1;
    	            } else {
            	        FastIntDivide(rand(), ((MAX_SAND/4)-Star[idx].x)|1, &tt, &Star[idx].y);
            	        Star[idx].y-=(((MAX_SAND/4)-Star[idx].x)|1)>>1;
        	        }
   	            }
       	    }

//          tx=(((Star[idx].x<<7)/Star[idx].z)>>3)+60;
            FastIntDivide(Star[idx].x<<7 , Star[idx].z, &tx, &tt);
            tx=(tx>>3)+60;

            if (Star[idx].targetx == 1) {
//              ty=(((Star[idx].y<<7)/Star[idx].z)>>3)+80;
                FastIntDivide(Star[idx].y<<7, Star[idx].z, &ty, &tt);
                ty=(ty>>3)+80;
            } else {
//              ty=(((Star[idx].y<<7)/Star[idx].z)>>2)+80;
                FastIntDivide(Star[idx].y<<7, Star[idx].z, &ty, &tt);
                ty=(ty>>2)+80;
            }

            if ((tx>0)&&(tx<120)&&(ty>0)&&(ty<160)) {
      		    tt=((Star[idx].z>>5)+32);
      		    // ty*120 == ((ty<<7)-(ty<<3))
      		    Star[idx].oldoffset=tx+((ty<<7)-(ty<<3));
                VideoBuf[Star[idx].oldoffset]=(tt<<8)|tt;
            } else {
                Star[idx].oldoffset=60+80*120;  // the compiler can solve this one, it's a const
            }
        }
    }
}

