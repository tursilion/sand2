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
// Sand2 GBA demo by Tursi

#include <malloc.h>
#include <string.h>
#include "tursigb.h"
#include "gsm_short.h"
#include "sanddemo.h"
#include "random.h"

// external reference
void ShowTursi();
void srand(unsigned int newseed);

// reference external graphics and sound
extern const signed short SINE[];
extern const char pSample[];
extern const u16 desert24_bit_Map[];
extern const u16 moreton_gfx[];
extern const u16 moreton_pal[];
extern const u16 tangalooma_gfx[];  // shares the moreton palette
extern const u16 atorestart_gfx[];  // shares the moreton palette

#define TANGATILES 9            // Last tangalooma tile
#define MORETONTILES 22         // Last moreton tile
#define ARESTARTTILES 9         // Last tile of 'A To Restart'

#define SCROLLEVENT1 0x5e0
#define SCROLLEVENT2 0x600
#define SCROLLEVENT3 0x8c8
#define SCROLLCOMPLETE 0xb50

/* vars */
const char *pDesert=(const char*)desert24_bit_Map;
u32 nDesertOff=0;
volatile bool fNeedAudio=false;
unsigned int seed;

struct _stars Star[MAX_SAND] DATA_IN_EWRAM;

// functions
void InterruptProcess(void) CODE_IN_IWRAM;
void DrawFrame(void) CODE_IN_IWRAM;
void doStorm(void) CODE_IN_IWRAM;

// defines to improve code a little
#define NEXTSCROLLFRAME                 \
        DrawFrame();                    \
        nDesertOff++;                   \
        WAITSYNC(2);

// Some variables
volatile u32 nFrame=0;          // Video frame count
u16* VideoBuf=(u16*)0x6000000;  // Video address

void CODE_IN_ROM SpriteMosaicIn(int st, int en) {
    int idx;
    
    REG_MOSAIC=0x4400;
    for (idx=st; idx<en; idx++) {
        SPR_MOSAICON(idx);
        SPR_DOUBLEOFF(idx);
    }
    for (idx=0x4400; idx>=0x1100; idx-=0x1100) {
        // Cheap way to get three frames ;)
        NEXTSCROLLFRAME;
        NEXTSCROLLFRAME;
        NEXTSCROLLFRAME;
        REG_MOSAIC=idx;
    }
    for (idx=st; idx<en; idx++) {
        SPR_MOSAICOFF(idx);
    }
}

void CODE_IN_ROM SpriteMosaicOut(int st, int en) {
    int idx;

    REG_MOSAIC=0x1100;
    for (idx=st; idx<en; idx++) {
        SPR_MOSAICON(idx);
    }
    for (idx=0x1100; idx<=0x4400; idx+=0x1100) {
        REG_MOSAIC=idx;
        // Cheap way to get three frames ;)
        NEXTSCROLLFRAME;
        NEXTSCROLLFRAME;
        NEXTSCROLLFRAME;
    }
    for (idx=st; idx<en; idx++) {
        SPR_DOUBLEON(idx);
        SPR_MOSAICOFF(idx);
    }
}

// Scroll the desert, start the music, some pulses, exit on white screen
void CODE_IN_ROM doDesert(void) {
    int idx;
    
    // Reset //
    // Stop interrupts
    REG_IME = 0;

    // Screen setup //
    nDesertOff=0;       // Desert scroll offset

    // Hardware setup //
    // Set the video to mode 3, bg 2 enabled (16-bit bitmap) & sprites
    REG_DISPCNT= MODE3 | BG2_ENABLE | SPRITE_ENABLE | SPRITES_1D;
    // Load the sprite data
    fastcopy16(SPR_PALETTE, moreton_pal, 32);
    // We can't use the first 512 patterns of sprite data because of bitmap mode, so load at #512
    fastcopy16(0x6010000+(512*32), tangalooma_gfx, 288);    // uses 9 tiles: 512-520
    fastcopy16(0x6010000+(521*32), moreton_gfx, 416);       // uses 13 tiles:521-533
    // Clear the sprite table
    fastset(OAM_BASE, 0, 1024);
    // Put the sprites in place. Supposedly, double-sized sprites with scaling disabled are
    // the preferred way to 'hide' a sprite
    for (idx=0; idx<TANGATILES; idx++) {
        SPR_DOUBLEON(idx);
        SPR_SETY(idx, 147);
        SPR_SETX(idx, idx<<3);
        SPR_SETTILE(idx, idx+512);
    }
    for (idx=TANGATILES; idx<MORETONTILES; idx++) {
        SPR_DOUBLEON(idx);
        SPR_SETY(idx, 153);
        SPR_SETX(idx, (idx-9)<<3);
        SPR_SETTILE(idx, idx+512);
    }

    // Audio engine setup
	gsm_init(pSample);      // Prepare the GSM decoder - defaults to 16khz
//	SetAudioFreq(0xFFFF - 332);
    DEEMPHASIS = 25380;
    process_decode();       // load the first buffer
    process_decode();       // load the second buffer

    // Set up the video chip - set vertical blank interrupt
    REG_DISPSTAT = VBLANK_IRQ;
    
    // Set up interrupts
    REG_IE = INT_TIMER1 | INT_VBLANK;
    REG_IME = 1;    // Enable interrupts

    // Allow audio to begin
    AudioOn();

    // fade in
    REG_COLEY=17;
    REG_BLDMOD= SOURCE_BG2 | DARKEN | TARGET_BG2;
    nFrame=0;
    for (idx=32; idx>=0; idx--) {
        REG_COLEY=(u16)idx>>1;
        NEXTSCROLLFRAME;
    }

// Skip desert and first of stars
//nNextBlock=0x1e00;
//pDat=pSample+(nNextBlock*33);
//goto skipdesert;

// Skip to end of desert
//nNextBlock=SCROLLCOMPLETE-200;
//pDat=pSample+(nNextBlock*33);
//goto skipmostofdesert;

    // Brief pause (about 2.5s)
    for (idx=0; idx<150; idx++) {
        NEXTSCROLLFRAME;
    }

    // mosaic in the location text
    SpriteMosaicIn(0, TANGATILES);
    SpriteMosaicIn(TANGATILES, MORETONTILES);
    
    // Wait about 3 seconds
    for (idx=0; idx<180; idx++) {
        NEXTSCROLLFRAME;
    }
    
    // Clear the text out
    SpriteMosaicOut(TANGATILES, MORETONTILES);
    SpriteMosaicOut(0, TANGATILES);

skipmostofdesert:
    // All that needs to be done before the first event
    // Event 1 and 2 just flash the screen - repeated code
    REG_BLDMOD= SOURCE_BG2 | LIGHTEN | TARGET_BG2;

    // The first couple are a double flash
    while (nNextBlock < SCROLLEVENT1) {
        NEXTSCROLLFRAME;
    }
    for (idx=20; idx>=0; idx-=2) {
        REG_COLEY=idx;
        NEXTSCROLLFRAME;
    }
    while (nNextBlock < SCROLLEVENT2) {
        NEXTSCROLLFRAME;
    }
    for (idx=20; idx>=0; idx--) {
        REG_COLEY=idx;
        NEXTSCROLLFRAME;
    }

    // Third is a single
    while (nNextBlock < SCROLLEVENT3) {
        NEXTSCROLLFRAME;
    }
    for (idx=20; idx>=0; idx--) {
        REG_COLEY=idx;
        NEXTSCROLLFRAME;
    }

    // Now wait for the end
    while (nNextBlock < SCROLLCOMPLETE) {
        NEXTSCROLLFRAME;
    }

skipdesert:

    // Fade up to white for the transistion
    for (idx=0; idx<=16; idx++) {
        REG_COLEY=idx;
        NEXTSCROLLFRAME;
    }
    // Desert segment complete
}

// Start here - just allows the demo to loop
int main(void) {
    int idx;

    // turn on the sound chip
    REG_SOUNDCNT_X = SND_ENABLED;
    // we don’t want to mess with sound channels 1-4
    REG_SOUNDCNT_L = 0;

    srand(999);     /* not so random */

    for (;;) {
        // Claim responsibility! ;)
        ShowTursi();
        // This scrolls the desert
        doDesert();
        // This does the first segment - twirly stars
        // it also initializes the screen to 8bit and
        // sets up the sand array
        doStorm();
        
        // Clear the sprite table
        fastset(OAM_BASE, 0, 1024);
        // stop the scrolltext interrupt
        REG_TM3CNT=0;
        
        // Set up the 'A to restart text'
        fastcopy16(SPR_PALETTE, moreton_pal, 32);       // reuse this same palette
        
        // Copy the data into the first sprites
        fastcopy16(0x6010000+(512*32), atorestart_gfx, 288);    // uses 9 tiles: 0-9

        // Clear the sprite table
        fastset(OAM_BASE, 0, 1024);
        // Put the sprites in place. Supposedly, double-sized sprites with scaling disabled are
        // the preferred way to 'hide' a sprite
        for (idx=0; idx<ARESTARTTILES; idx++) {
            SPR_DOUBLEON(idx);
            SPR_SETY(idx, 147);
            SPR_SETX(idx, idx<<3);
            SPR_SETTILE(idx, idx+512);
        }
        // And, slowly mosaic it in (can't use the function call cause the desert isn't up)
        REG_MOSAIC=0x4400;
        for (idx=0; idx<ARESTARTTILES; idx++) {
            SPR_MOSAICON(idx+512);
            SPR_DOUBLEOFF(idx+512);
        }
        for (idx=0x4400; idx>=0x1100; idx-=0x1100) {
            vsync();
            vsync();
            vsync();
            vsync();
            REG_MOSAIC=idx;
        }
        for (idx=0; idx<ARESTARTTILES; idx++) {
            SPR_MOSAICOFF(idx+512);
        }

        // Wait for restart
        for (;;) {
            if (!((REG_KEYINPUT)&BTN_A))   // used to break and reset (for now)
                break;
        }

        // Restarting - make sure interrupts are off
        REG_IME = 0;
        // make sure Timers are off
        REG_TM0CNT = 0;
        REG_TM1CNT = 0;
        REG_TM3CNT = 0;
        // make sure DMA is turned off
        REG_DMA1CNT_H = 0;

        for (idx=0; idx<30; idx++) vsync();   // pause half a second
    }
    return 0;   // never reached
}

