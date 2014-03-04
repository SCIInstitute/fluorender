//-----------------------------------------------------------------------------
//           Name: bitmap_fonts.h
//         Author: John Fay
//  Last Modified: 01/26/05
//    Description: Data and utility functions for rendering bitmap based fonts. 
//                 This is a modified version of the bitmap based rendering 
//                 system from the open source Freeglut project.
//                 http://freeglut.sourceforge.net/
//-----------------------------------------------------------------------------

#ifndef _BITMAP_FONTS_H_
#define _BITMAP_FONTS_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include OPEN_GL_HEADER 

/*
 * The bitmap font structure
 */

struct BitmapFontData
{
    char*           Name;         /* The source font name             */
    int             Quantity;     /* Number of chars in font          */
    int             Height;       /* Height of the characters         */
    const GLubyte** Characters;   /* The characters mapping           */

    float           xorig, yorig; /* Relative origin of the character */
};

/*
 * Following fonts are defined in this file:
 * 
 * 1. FontFixed8x13 <-misc-fixed-medium-r-normal--13-120-75-75-C-80-iso8859-1>
 * 2. FontFixed9x15 <-misc-fixed-medium-r-normal--15-140-75-75-C-90-iso8859-1>
 * 3. FontHelvetica10 <-adobe-helvetica-medium-r-normal--10-100-75-75-p-56-iso8859-1>
 * 4. FontHelvetica12 <-adobe-helvetica-medium-r-normal--12-120-75-75-p-67-iso8859-1>
 * 5. FontHelvetica18 <-adobe-helvetica-medium-r-normal--18-180-75-75-p-98-iso8859-1>
 * 6. FontTimesRoman10 <-adobe-times-medium-r-normal--10-100-75-75-p-54-iso8859-1>
 * 7. FontTimesRoman24 <-adobe-times-medium-r-normal--24-240-75-75-p-124-iso8859-1>
 */

static const GLubyte Fixed8x13_Character_032[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* blank */
static const GLubyte Fixed8x13_Character_097[] = {  8,  0,  0,116,140,132,124,  4,120,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte Fixed8x13_Character_098[] = {  8,  0,  0,184,196,132,132,196,184,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_099[] = {  8,  0,  0,120,132,128,128,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_100[] = {  8,  0,  0,116,140,132,132,140,116,  4,  4,  4,  0,  0};
static const GLubyte Fixed8x13_Character_101[] = {  8,  0,  0,120,132,128,252,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_102[] = {  8,  0,  0, 64, 64, 64, 64,248, 64, 64, 68, 56,  0,  0};
static const GLubyte Fixed8x13_Character_103[] = {  8,120,132,120,128,112,136,136,116,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_104[] = {  8,  0,  0,132,132,132,132,196,184,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_105[] = {  8,  0,  0,248, 32, 32, 32, 32, 96,  0, 32,  0,  0,  0};
static const GLubyte Fixed8x13_Character_106[] = {  8,112,136,136,  8,  8,  8,  8, 24,  0,  8,  0,  0,  0};
static const GLubyte Fixed8x13_Character_107[] = {  8,  0,  0,132,136,144,224,144,136,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_108[] = {  8,  0,  0,248, 32, 32, 32, 32, 32, 32, 32, 96,  0,  0};
static const GLubyte Fixed8x13_Character_109[] = {  8,  0,  0,130,146,146,146,146,236,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_110[] = {  8,  0,  0,132,132,132,132,196,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_111[] = {  8,  0,  0,120,132,132,132,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_112[] = {  8,128,128,128,184,196,132,196,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_113[] = {  8,  4,  4,  4,116,140,132,140,116,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_114[] = {  8,  0,  0, 64, 64, 64, 64, 68,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_115[] = {  8,  0,  0,120,132, 24, 96,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_116[] = {  8,  0,  0, 56, 68, 64, 64, 64,248, 64, 64,  0,  0,  0};
static const GLubyte Fixed8x13_Character_117[] = {  8,  0,  0,116,136,136,136,136,136,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_119[] = {  8,  0,  0, 68,170,146,146,130,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_118[] = {  8,  0,  0, 32, 80, 80,136,136,136,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_120[] = {  8,  0,  0,132, 72, 48, 48, 72,132,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_121[] = {  8,120,132,  4,116,140,132,132,132,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_122[] = {  8,  0,  0,252, 64, 32, 16,  8,252,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte Fixed8x13_Character_065[] = {  8,  0,  0,132,132,132,252,132,132,132, 72, 48,  0,  0}; /* "A" */
static const GLubyte Fixed8x13_Character_066[] = {  8,  0,  0,252, 66, 66, 66,124, 66, 66, 66,252,  0,  0};
static const GLubyte Fixed8x13_Character_067[] = {  8,  0,  0,120,132,128,128,128,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_068[] = {  8,  0,  0,252, 66, 66, 66, 66, 66, 66, 66,252,  0,  0};
static const GLubyte Fixed8x13_Character_069[] = {  8,  0,  0,252,128,128,128,240,128,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_070[] = {  8,  0,  0,128,128,128,128,240,128,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_071[] = {  8,  0,  0,116,140,132,156,128,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_072[] = {  8,  0,  0,132,132,132,132,252,132,132,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_073[] = {  8,  0,  0,248, 32, 32, 32, 32, 32, 32, 32,248,  0,  0};
static const GLubyte Fixed8x13_Character_074[] = {  8,  0,  0,112,136,  8,  8,  8,  8,  8,  8, 60,  0,  0};
static const GLubyte Fixed8x13_Character_075[] = {  8,  0,  0,132,136,144,160,192,160,144,136,132,  0,  0};
static const GLubyte Fixed8x13_Character_076[] = {  8,  0,  0,252,128,128,128,128,128,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_077[] = {  8,  0,  0,130,130,130,146,146,170,198,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_078[] = {  8,  0,  0,132,132,132,140,148,164,196,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_079[] = {  8,  0,  0,120,132,132,132,132,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_080[] = {  8,  0,  0,128,128,128,128,248,132,132,132,248,  0,  0};
static const GLubyte Fixed8x13_Character_081[] = {  8,  0,  4,120,148,164,132,132,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_082[] = {  8,  0,  0,132,136,144,160,248,132,132,132,248,  0,  0};
static const GLubyte Fixed8x13_Character_083[] = {  8,  0,  0,120,132,  4,  4,120,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_084[] = {  8,  0,  0, 16, 16, 16, 16, 16, 16, 16, 16,254,  0,  0};
static const GLubyte Fixed8x13_Character_085[] = {  8,  0,  0,120,132,132,132,132,132,132,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_087[] = {  8,  0,  0, 68,170,146,146,146,130,130,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_086[] = {  8,  0,  0, 16, 40, 40, 40, 68, 68, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_088[] = {  8,  0,  0,130,130, 68, 40, 16, 40, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_089[] = {  8,  0,  0, 16, 16, 16, 16, 16, 40, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_090[] = {  8,  0,  0,252,128,128, 64, 32, 16,  8,  4,252,  0,  0}; /* "Z" */
static const GLubyte Fixed8x13_Character_048[] = {  8,  0,  0, 48, 72,132,132,132,132,132, 72, 48,  0,  0}; /* "0" */
static const GLubyte Fixed8x13_Character_049[] = {  8,  0,  0,248, 32, 32, 32, 32, 32,160, 96, 32,  0,  0};
static const GLubyte Fixed8x13_Character_050[] = {  8,  0,  0,252,128, 64, 48,  8,  4,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_051[] = {  8,  0,  0,120,132,  4,  4, 56, 16,  8,  4,252,  0,  0};
static const GLubyte Fixed8x13_Character_052[] = {  8,  0,  0,  8,  8,252,136,136, 72, 40, 24,  8,  0,  0};
static const GLubyte Fixed8x13_Character_053[] = {  8,  0,  0,120,132,  4,  4,196,184,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_054[] = {  8,  0,  0,120,132,132,196,184,128,128, 64, 56,  0,  0};
static const GLubyte Fixed8x13_Character_055[] = {  8,  0,  0, 64, 64, 32, 32, 16, 16,  8,  4,252,  0,  0};
static const GLubyte Fixed8x13_Character_056[] = {  8,  0,  0,120,132,132,132,120,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_057[] = {  8,  0,  0,112,  8,  4,  4,116,140,132,132,120,  0,  0}; /* "9" */
static const GLubyte Fixed8x13_Character_096[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0, 16, 96,224,  0,  0}; /* "`" */
static const GLubyte Fixed8x13_Character_126[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,144,168, 72,  0,  0}; /* "~" */
static const GLubyte Fixed8x13_Character_033[] = {  8,  0,  0,128,  0,128,128,128,128,128,128,128,  0,  0}; /* "!" */
static const GLubyte Fixed8x13_Character_064[] = {  8,  0,  0,120,128,148,172,164,156,132,132,120,  0,  0}; /* "@" */
static const GLubyte Fixed8x13_Character_035[] = {  8,  0,  0,  0, 72, 72,252, 72,252, 72, 72,  0,  0,  0}; /* "#" */
static const GLubyte Fixed8x13_Character_036[] = {  8,  0,  0,  0, 32,240, 40,112,160,120, 32,  0,  0,  0}; /* "$" */
static const GLubyte Fixed8x13_Character_037[] = {  8,  0,  0,136, 84, 72, 32, 16, 16, 72,164, 68,  0,  0}; /* "%" */
static const GLubyte Fixed8x13_Character_094[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,136, 80, 32,  0,  0}; /* "^" */
static const GLubyte Fixed8x13_Character_038[] = {  8,  0,  0,116,136,148, 96,144,144, 96,  0,  0,  0,  0}; /* "&" */
static const GLubyte Fixed8x13_Character_042[] = {  8,  0,  0,  0,  0, 72, 48,252, 48, 72,  0,  0,  0,  0}; /* "*" */
static const GLubyte Fixed8x13_Character_040[] = {  8,  0,  0, 32, 64, 64,128,128,128, 64, 64, 32,  0,  0}; /* "(" */
static const GLubyte Fixed8x13_Character_041[] = {  8,  0,  0,128, 64, 64, 32, 32, 32, 64, 64,128,  0,  0}; /* ")" */
static const GLubyte Fixed8x13_Character_045[] = {  8,  0,  0,  0,  0,  0,  0,252,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte Fixed8x13_Character_095[] = {  8,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte Fixed8x13_Character_061[] = {  8,  0,  0,  0,  0,252,  0,  0,252,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte Fixed8x13_Character_043[] = {  8,  0,  0,  0,  0, 32, 32,248, 32, 32,  0,  0,  0,  0}; /* "+" */
static const GLubyte Fixed8x13_Character_091[] = {  8,  0,  0,240,128,128,128,128,128,128,128,240,  0,  0}; /* "[" */
static const GLubyte Fixed8x13_Character_123[] = {  8,  0,  0, 56, 64, 64, 32,192, 32, 64, 64, 56,  0,  0}; /* "{" */
static const GLubyte Fixed8x13_Character_125[] = {  8,  0,  0,224, 16, 16, 32, 24, 32, 16, 16,224,  0,  0}; /* "}" */
static const GLubyte Fixed8x13_Character_093[] = {  8,  0,  0,240, 16, 16, 16, 16, 16, 16, 16,240,  0,  0}; /* "]" */
static const GLubyte Fixed8x13_Character_059[] = {  8,  0,128, 96,112,  0,  0, 32,112, 32,  0,  0,  0,  0}; /* ";" */
static const GLubyte Fixed8x13_Character_058[] = {  8,  0, 64,224, 64,  0,  0, 64,224, 64,  0,  0,  0,  0}; /* ":" */
static const GLubyte Fixed8x13_Character_044[] = {  8,  0,128, 96,112,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte Fixed8x13_Character_046[] = {  8,  0, 64,224, 64,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte Fixed8x13_Character_060[] = {  8,  0,  0,  8, 16, 32, 64,128, 64, 32, 16,  8,  0,  0}; /* "<" */
static const GLubyte Fixed8x13_Character_062[] = {  8,  0,  0,128, 64, 32, 16,  8, 16, 32, 64,128,  0,  0}; /* ">" */
static const GLubyte Fixed8x13_Character_047[] = {  8,  0,  0,128,128, 64, 32, 16,  8,  4,  2,  2,  0,  0}; /* "/" */
static const GLubyte Fixed8x13_Character_063[] = {  8,  0,  0, 16,  0, 16, 16,  8,  4,132,132,120,  0,  0}; /* "?" */
static const GLubyte Fixed8x13_Character_092[] = {  8,  0,  0,  2,  2,  4,  8, 16, 32, 64,128,128,  0,  0}; /* "\" */
static const GLubyte Fixed8x13_Character_034[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,144,144,144,  0,  0}; /* """ */

/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte Fixed8x13_Character_039[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0, 32, 32, 32,  0,  0}; /* """ */
static const GLubyte Fixed8x13_Character_124[] = {  8, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,  0,  0}; /* """ */


/* The font characters mapping: */
static const GLubyte* Fixed8x13_Character_Map[] = {Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_032,Fixed8x13_Character_033,Fixed8x13_Character_034,Fixed8x13_Character_035,Fixed8x13_Character_036,Fixed8x13_Character_037,Fixed8x13_Character_038,Fixed8x13_Character_039,Fixed8x13_Character_040,
   Fixed8x13_Character_041,Fixed8x13_Character_042,Fixed8x13_Character_043,Fixed8x13_Character_044,Fixed8x13_Character_045,Fixed8x13_Character_046,Fixed8x13_Character_047,Fixed8x13_Character_048,Fixed8x13_Character_049,Fixed8x13_Character_050,Fixed8x13_Character_051,Fixed8x13_Character_052,Fixed8x13_Character_053,Fixed8x13_Character_054,Fixed8x13_Character_055,Fixed8x13_Character_056,Fixed8x13_Character_057,Fixed8x13_Character_058,Fixed8x13_Character_059,Fixed8x13_Character_060,Fixed8x13_Character_061,Fixed8x13_Character_062,Fixed8x13_Character_063,Fixed8x13_Character_064,Fixed8x13_Character_065,Fixed8x13_Character_066,Fixed8x13_Character_067,Fixed8x13_Character_068,Fixed8x13_Character_069,Fixed8x13_Character_070,Fixed8x13_Character_071,Fixed8x13_Character_072,Fixed8x13_Character_073,Fixed8x13_Character_074,Fixed8x13_Character_075,Fixed8x13_Character_076,Fixed8x13_Character_077,Fixed8x13_Character_078,Fixed8x13_Character_079,Fixed8x13_Character_080,Fixed8x13_Character_081,Fixed8x13_Character_082,
   Fixed8x13_Character_083,Fixed8x13_Character_084,Fixed8x13_Character_085,Fixed8x13_Character_086,Fixed8x13_Character_087,Fixed8x13_Character_088,Fixed8x13_Character_089,Fixed8x13_Character_090,Fixed8x13_Character_091,Fixed8x13_Character_092,Fixed8x13_Character_093,Fixed8x13_Character_094,Fixed8x13_Character_095,Fixed8x13_Character_096,Fixed8x13_Character_097,Fixed8x13_Character_098,Fixed8x13_Character_099,Fixed8x13_Character_100,Fixed8x13_Character_101,Fixed8x13_Character_102,Fixed8x13_Character_103,Fixed8x13_Character_104,Fixed8x13_Character_105,Fixed8x13_Character_106,Fixed8x13_Character_107,Fixed8x13_Character_108,Fixed8x13_Character_109,Fixed8x13_Character_110,Fixed8x13_Character_111,Fixed8x13_Character_112,Fixed8x13_Character_113,Fixed8x13_Character_114,Fixed8x13_Character_115,Fixed8x13_Character_116,Fixed8x13_Character_117,Fixed8x13_Character_118,Fixed8x13_Character_119,Fixed8x13_Character_120,Fixed8x13_Character_121,Fixed8x13_Character_122,Fixed8x13_Character_123,Fixed8x13_Character_124,
   Fixed8x13_Character_125,Fixed8x13_Character_126,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,
   Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,
   Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,
   Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,Fixed8x13_Character_042,NULL};

/* The font structure: */
const BitmapFontData FontFixed8x13 = { "-misc-fixed-medium-r-normal--13-120-75-75-C-80-iso8859-1", 93, 13, Fixed8x13_Character_Map, -1.0f, 2.0f };

static const GLubyte Fixed9x15_Character_032[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* blank */
static const GLubyte Fixed9x15_Character_097[] = {  9,  0,  0,  0,  0,  0,  0,122,  0,134,  0,130,  0,126,  0,  2,  0,  2,  0,124,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte Fixed9x15_Character_098[] = {  9,  0,  0,  0,  0,  0,  0,188,  0,194,  0,130,  0,130,  0,130,  0,194,  0,188,  0,128,  0,128,  0,128,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_099[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,128,  0,128,  0,128,  0,130,  0,124,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_100[] = {  9,  0,  0,  0,  0,  0,  0,122,  0,134,  0,130,  0,130,  0,130,  0,134,  0,122,  0,  2,  0,  2,  0,  2,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_101[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,128,  0,128,  0,254,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_102[] = {  9,  0,  0,  0,  0,  0,  0, 32,  0, 32,  0, 32,  0, 32,  0,248,  0, 32,  0, 32,  0, 34,  0, 34,  0, 28,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_103[] = {  9,124,  0,130,  0,130,  0,124,  0,128,  0,120,  0,132,  0,132,  0,132,  0,122,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_104[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,130,  0,130,  0,130,  0,194,  0,188,  0,128,  0,128,  0,128,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_105[] = {  9,  0,  0,  0,  0,  0,  0,248,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0,224,  0,  0,  0,  0,  0, 96,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_106[] = {  9,120,  0,132,  0,132,  0,132,  0,  4,  0,  4,  0,  4,  0,  4,  0,  4,  0, 28,  0,  0,  0,  0,  0, 12,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_107[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,140,  0,176,  0,192,  0,176,  0,140,  0,130,  0,128,  0,128,  0,128,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_108[] = {  9,  0,  0,  0,  0,  0,  0,248,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0,224,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_109[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,146,  0,146,  0,146,  0,146,  0,146,  0,236,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_110[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,130,  0,130,  0,130,  0,194,  0,188,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_111[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,130,  0,130,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_112[] = {  9,128,  0,128,  0,128,  0,188,  0,194,  0,130,  0,130,  0,130,  0,194,  0,188,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_113[] = {  9,  2,  0,  2,  0,  2,  0,122,  0,134,  0,130,  0,130,  0,130,  0,134,  0,122,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_114[] = {  9,  0,  0,  0,  0,  0,  0, 64,  0, 64,  0, 64,  0, 64,  0, 66,  0, 98,  0,156,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_115[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,  2,  0,124,  0,128,  0,130,  0,124,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_116[] = {  9,  0,  0,  0,  0,  0,  0, 28,  0, 34,  0, 32,  0, 32,  0, 32,  0, 32,  0,252,  0, 32,  0, 32,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_117[] = {  9,  0,  0,  0,  0,  0,  0,122,  0,132,  0,132,  0,132,  0,132,  0,132,  0,132,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_119[] = {  9,  0,  0,  0,  0,  0,  0, 68,  0,170,  0,146,  0,146,  0,146,  0,130,  0,130,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_118[] = {  9,  0,  0,  0,  0,  0,  0, 16,  0, 40,  0, 40,  0, 68,  0, 68,  0,130,  0,130,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_120[] = {  9,  0,  0,  0,  0,  0,  0,130,  0, 68,  0, 40,  0, 16,  0, 40,  0, 68,  0,130,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_121[] = {  9,120,  0,132,  0,  4,  0,116,  0,140,  0,132,  0,132,  0,132,  0,132,  0,132,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_122[] = {  9,  0,  0,  0,  0,  0,  0,254,  0, 64,  0, 32,  0, 16,  0,  8,  0,  4,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte Fixed9x15_Character_065[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,130,  0,254,  0,130,  0,130,  0,130,  0, 68,  0, 40,  0, 16,  0,  0,  0,  0,  0}; /* "A" */
static const GLubyte Fixed9x15_Character_066[] = {  9,  0,  0,  0,  0,  0,  0,252,  0, 66,  0, 66,  0, 66,  0, 66,  0,124,  0, 66,  0, 66,  0, 66,  0,252,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_067[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,130,  0,124,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_068[] = {  9,  0,  0,  0,  0,  0,  0,252,  0, 66,  0, 66,  0, 66,  0, 66,  0, 66,  0, 66,  0, 66,  0, 66,  0,252,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_069[] = {  9,  0,  0,  0,  0,  0,  0,254,  0, 64,  0, 64,  0, 64,  0, 64,  0,120,  0, 64,  0, 64,  0, 64,  0,254,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_070[] = {  9,  0,  0,  0,  0,  0,  0, 64,  0, 64,  0, 64,  0, 64,  0, 64,  0,120,  0, 64,  0, 64,  0, 64,  0,254,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_071[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,130,  0,130,  0,142,  0,128,  0,128,  0,128,  0,130,  0,124,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_072[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,130,  0,130,  0,130,  0,254,  0,130,  0,130,  0,130,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_073[] = {  9,  0,  0,  0,  0,  0,  0,248,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0,248,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_074[] = {  9,  0,  0,  0,  0,  0,  0,120,  0,132,  0,  4,  0,  4,  0,  4,  0,  4,  0,  4,  0,  4,  0,  4,  0, 30,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_075[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,132,  0,136,  0,144,  0,160,  0,224,  0,144,  0,136,  0,132,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_076[] = {  9,  0,  0,  0,  0,  0,  0,254,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_077[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,130,  0,146,  0,146,  0,170,  0,170,  0,198,  0,130,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_078[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,130,  0,134,  0,138,  0,146,  0,162,  0,194,  0,130,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_079[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_080[] = {  9,  0,  0,  0,  0,  0,  0,128,  0,128,  0,128,  0,128,  0,128,  0,252,  0,130,  0,130,  0,130,  0,252,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_081[] = {  9,  0,  0,  6,  0,  8,  0,124,  0,146,  0,162,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_082[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,132,  0,136,  0,144,  0,252,  0,130,  0,130,  0,130,  0,252,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_083[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,130,  0,  2,  0, 12,  0,112,  0,128,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_084[] = {  9,  0,  0,  0,  0,  0,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0,254,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_085[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_087[] = {  9,  0,  0,  0,  0,  0,  0, 68,  0,170,  0,146,  0,146,  0,146,  0,146,  0,130,  0,130,  0,130,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_086[] = {  9,  0,  0,  0,  0,  0,  0, 16,  0, 40,  0, 40,  0, 40,  0, 68,  0, 68,  0, 68,  0,130,  0,130,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_088[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0, 68,  0, 40,  0, 16,  0, 16,  0, 40,  0, 68,  0,130,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_089[] = {  9,  0,  0,  0,  0,  0,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 40,  0, 68,  0,130,  0,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_090[] = {  9,  0,  0,  0,  0,  0,  0,254,  0,128,  0,128,  0, 64,  0, 32,  0, 16,  0,  8,  0,  4,  0,  2,  0,254,  0,  0,  0,  0,  0}; /* "Z" */
static const GLubyte Fixed9x15_Character_048[] = {  9,  0,  0,  0,  0,  0,  0, 56,  0, 68,  0,130,  0,130,  0,130,  0,130,  0,130,  0,130,  0, 68,  0, 56,  0,  0,  0,  0,  0}; /* "0" */
static const GLubyte Fixed9x15_Character_049[] = {  9,  0,  0,  0,  0,  0,  0,254,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0,144,  0, 80,  0, 48,  0, 16,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_050[] = {  9,  0,  0,  0,  0,  0,  0,254,  0,128,  0, 64,  0, 48,  0,  8,  0,  4,  0,  2,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_051[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,  2,  0,  2,  0,  2,  0, 28,  0,  8,  0,  4,  0,  2,  0,254,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_052[] = {  9,  0,  0,  0,  0,  0,  0,  4,  0,  4,  0,  4,  0,254,  0,132,  0, 68,  0, 36,  0, 20,  0, 12,  0,  4,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_053[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,  2,  0,  2,  0,  2,  0,194,  0,188,  0,128,  0,128,  0,254,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_054[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,130,  0,130,  0,130,  0,194,  0,188,  0,128,  0,128,  0, 64,  0, 60,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_055[] = {  9,  0,  0,  0,  0,  0,  0, 64,  0, 64,  0, 32,  0, 32,  0, 16,  0,  8,  0,  4,  0,  2,  0,  2,  0,254,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_056[] = {  9,  0,  0,  0,  0,  0,  0, 56,  0, 68,  0,130,  0,130,  0, 68,  0, 56,  0, 68,  0,130,  0, 68,  0, 56,  0,  0,  0,  0,  0};
static const GLubyte Fixed9x15_Character_057[] = {  9,  0,  0,  0,  0,  0,  0,120,  0,  4,  0,  2,  0,  2,  0,122,  0,134,  0,130,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0}; /* "9" */
static const GLubyte Fixed9x15_Character_096[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 16,  0, 32,  0, 64,  0,192,  0,  0,  0,  0,  0}; /* "`" */
static const GLubyte Fixed9x15_Character_126[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,140,  0,146,  0, 98,  0,  0,  0,  0,  0}; /* "~" */
static const GLubyte Fixed9x15_Character_033[] = {  9,  0,  0,  0,  0,  0,  0,128,  0,128,  0,  0,  0,  0,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,  0,  0}; /* "!" */
static const GLubyte Fixed9x15_Character_064[] = {  9,  0,  0,  0,  0,  0,  0,124,  0,128,  0,128,  0,154,  0,166,  0,162,  0,158,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0}; /* "@" */
static const GLubyte Fixed9x15_Character_035[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0, 72,  0, 72,  0,252,  0, 72,  0, 72,  0,252,  0, 72,  0, 72,  0,  0,  0,  0,  0,  0,  0}; /* "#" */
static const GLubyte Fixed9x15_Character_036[] = {  9,  0,  0,  0,  0, 16,  0,124,  0,146,  0, 18,  0, 18,  0, 20,  0, 56,  0, 80,  0,144,  0,146,  0,124,  0, 16,  0,  0,  0}; /* "$" */
static const GLubyte Fixed9x15_Character_037[] = {  9,  0,  0,  0,  0,  0,  0,132,  0, 74,  0, 74,  0, 36,  0, 16,  0, 16,  0, 72,  0,164,  0,164,  0, 66,  0,  0,  0,  0,  0}; /* "%" */
static const GLubyte Fixed9x15_Character_094[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  0, 68,  0, 40,  0, 16,  0,  0,  0,  0,  0}; /* "^" */
static const GLubyte Fixed9x15_Character_038[] = {  9,  0,  0,  0,  0,  0,  0, 98,  0,148,  0,136,  0,148,  0, 98,  0, 96,  0,144,  0,144,  0,144,  0, 96,  0,  0,  0,  0,  0}; /* "&" */
static const GLubyte Fixed9x15_Character_042[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0, 16,  0,146,  0, 84,  0, 56,  0, 84,  0,146,  0, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "*" */
static const GLubyte Fixed9x15_Character_040[] = {  9,  0,  0,  0,  0, 32,  0, 64,  0, 64,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0, 64,  0, 64,  0, 32,  0,  0,  0}; /* "(" */
static const GLubyte Fixed9x15_Character_041[] = {  9,  0,  0,  0,  0,128,  0, 64,  0, 64,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 64,  0, 64,  0,128,  0,  0,  0}; /* ")" */
static const GLubyte Fixed9x15_Character_045[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte Fixed9x15_Character_095[] = {  9,  0,  0,  0,  0,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte Fixed9x15_Character_061[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,254,  0,  0,  0,  0,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte Fixed9x15_Character_043[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0, 16,  0, 16,  0, 16,  0,254,  0, 16,  0, 16,  0, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "+" */
static const GLubyte Fixed9x15_Character_091[] = {  9,  0,  0,  0,  0,240,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,128,  0,240,  0,  0,  0}; /* "[" */
static const GLubyte Fixed9x15_Character_123[] = {  9,  0,  0,  0,  0, 56,  0, 64,  0, 64,  0, 64,  0, 32,  0,192,  0,192,  0, 32,  0, 64,  0, 64,  0, 64,  0, 56,  0,  0,  0}; /* "{" */
static const GLubyte Fixed9x15_Character_125[] = {  9,  0,  0,  0,  0,224,  0, 16,  0, 16,  0, 16,  0, 32,  0, 24,  0, 24,  0, 32,  0, 16,  0, 16,  0, 16,  0,224,  0,  0,  0}; /* "}" */
static const GLubyte Fixed9x15_Character_093[] = {  9,  0,  0,  0,  0,240,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0, 16,  0,240,  0,  0,  0}; /* "]" */
static const GLubyte Fixed9x15_Character_059[] = {  9,128,  0, 64,  0, 64,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* ";" */
static const GLubyte Fixed9x15_Character_058[] = {  9,  0,  0,  0,  0,  0,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* ":" */
static const GLubyte Fixed9x15_Character_044[] = {  9,128,  0, 64,  0, 64,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte Fixed9x15_Character_046[] = {  9,  0,  0,  0,  0,  0,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte Fixed9x15_Character_060[] = {  9,  0,  0,  0,  0,  0,  0,  8,  0, 16,  0, 32,  0, 64,  0,128,  0,128,  0, 64,  0, 32,  0, 16,  0,  8,  0,  0,  0,  0,  0}; /* "<" */
static const GLubyte Fixed9x15_Character_062[] = {  9,  0,  0,  0,  0,  0,  0,128,  0, 64,  0, 32,  0, 16,  0,  8,  0,  8,  0, 16,  0, 32,  0, 64,  0,128,  0,  0,  0,  0,  0}; /* ">" */
static const GLubyte Fixed9x15_Character_047[] = {  9,  0,  0,  0,  0,  0,  0,128,  0, 64,  0, 64,  0, 32,  0, 16,  0, 16,  0,  8,  0,  4,  0,  4,  0,  2,  0,  0,  0,  0,  0}; /* "/" */
static const GLubyte Fixed9x15_Character_063[] = {  9,  0,  0,  0,  0,  0,  0, 16,  0,  0,  0, 16,  0, 16,  0,  8,  0,  4,  0,  2,  0,130,  0,130,  0,124,  0,  0,  0,  0,  0}; /* "?" */
static const GLubyte Fixed9x15_Character_092[] = {  9,  0,  0,  0,  0,  0,  0,  2,  0,  4,  0,  4,  0,  8,  0, 16,  0, 16,  0, 32,  0, 64,  0, 64,  0,128,  0,  0,  0,  0,  0}; /* "\" */
static const GLubyte Fixed9x15_Character_034[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,144,  0,144,  0,144,  0,  0,  0,  0,  0}; /* """ */

/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte Fixed9x15_Character_039[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 32,  0, 32,  0, 32,  0, 32,  0,  0,  0,  0,  0}; /* "'" */
static const GLubyte Fixed9x15_Character_124[] = {  9, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0, 32,  0,  0,  0,  0,  0}; /* "|" */


/* The font characters mapping: */
static const GLubyte* Fixed9x15_Character_Map[] = {Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_032,Fixed9x15_Character_033,Fixed9x15_Character_034,Fixed9x15_Character_035,Fixed9x15_Character_036,Fixed9x15_Character_037,Fixed9x15_Character_038,Fixed9x15_Character_039,Fixed9x15_Character_040,
   Fixed9x15_Character_041,Fixed9x15_Character_042,Fixed9x15_Character_043,Fixed9x15_Character_044,Fixed9x15_Character_045,Fixed9x15_Character_046,Fixed9x15_Character_047,Fixed9x15_Character_048,Fixed9x15_Character_049,Fixed9x15_Character_050,Fixed9x15_Character_051,Fixed9x15_Character_052,Fixed9x15_Character_053,Fixed9x15_Character_054,Fixed9x15_Character_055,Fixed9x15_Character_056,Fixed9x15_Character_057,Fixed9x15_Character_058,Fixed9x15_Character_059,Fixed9x15_Character_060,Fixed9x15_Character_061,Fixed9x15_Character_062,Fixed9x15_Character_063,Fixed9x15_Character_064,Fixed9x15_Character_065,Fixed9x15_Character_066,Fixed9x15_Character_067,Fixed9x15_Character_068,Fixed9x15_Character_069,Fixed9x15_Character_070,Fixed9x15_Character_071,Fixed9x15_Character_072,Fixed9x15_Character_073,Fixed9x15_Character_074,Fixed9x15_Character_075,Fixed9x15_Character_076,Fixed9x15_Character_077,Fixed9x15_Character_078,Fixed9x15_Character_079,Fixed9x15_Character_080,Fixed9x15_Character_081,Fixed9x15_Character_082,
   Fixed9x15_Character_083,Fixed9x15_Character_084,Fixed9x15_Character_085,Fixed9x15_Character_086,Fixed9x15_Character_087,Fixed9x15_Character_088,Fixed9x15_Character_089,Fixed9x15_Character_090,Fixed9x15_Character_091,Fixed9x15_Character_092,Fixed9x15_Character_093,Fixed9x15_Character_094,Fixed9x15_Character_095,Fixed9x15_Character_096,Fixed9x15_Character_097,Fixed9x15_Character_098,Fixed9x15_Character_099,Fixed9x15_Character_100,Fixed9x15_Character_101,Fixed9x15_Character_102,Fixed9x15_Character_103,Fixed9x15_Character_104,Fixed9x15_Character_105,Fixed9x15_Character_106,Fixed9x15_Character_107,Fixed9x15_Character_108,Fixed9x15_Character_109,Fixed9x15_Character_110,Fixed9x15_Character_111,Fixed9x15_Character_112,Fixed9x15_Character_113,Fixed9x15_Character_114,Fixed9x15_Character_115,Fixed9x15_Character_116,Fixed9x15_Character_117,Fixed9x15_Character_118,Fixed9x15_Character_119,Fixed9x15_Character_120,Fixed9x15_Character_121,Fixed9x15_Character_122,Fixed9x15_Character_123,Fixed9x15_Character_124,
   Fixed9x15_Character_125,Fixed9x15_Character_126,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,
   Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,
   Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,
   Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,Fixed9x15_Character_042,NULL};

/* The font structure: */
const BitmapFontData FontFixed9x15 = { "-misc-fixed-medium-r-normal--15-140-75-75-C-90-iso8859-1", 93, 15, Fixed9x15_Character_Map, -1.0f, 3.0f };

static const GLubyte Helvetica10_Character_032[] = {  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* " " */
static const GLubyte Helvetica10_Character_097[] = {  5,  0,  0,104,144,144,112, 16,224,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte Helvetica10_Character_098[] = {  6,  0,  0,176,200,136,136,200,176,128,128,  0,  0,  0}; /* "b" */
static const GLubyte Helvetica10_Character_099[] = {  5,  0,  0, 96,144,128,128,144, 96,  0,  0,  0,  0,  0}; /* "c" */
static const GLubyte Helvetica10_Character_100[] = {  6,  0,  0,104,152,136,136,152,104,  8,  8,  0,  0,  0}; /* "d" */
static const GLubyte Helvetica10_Character_101[] = {  5,  0,  0, 96,144,128,240,144, 96,  0,  0,  0,  0,  0}; /* "e" */
static const GLubyte Helvetica10_Character_102[] = {  4,  0,  0, 64, 64, 64, 64, 64,224, 64, 48,  0,  0,  0}; /* "f" */
static const GLubyte Helvetica10_Character_103[] = {  6,112,  8,104,152,136,136,152,104,  0,  0,  0,  0,  0}; /* "g" */
static const GLubyte Helvetica10_Character_104[] = {  6,  0,  0,136,136,136,136,200,176,128,128,  0,  0,  0}; /* "h" */
static const GLubyte Helvetica10_Character_105[] = {  2,  0,  0,128,128,128,128,128,128,  0,128,  0,  0,  0}; /* "i" */
static const GLubyte Helvetica10_Character_106[] = {  2,  0,128,128,128,128,128,128,128,  0,128,  0,  0,  0}; /* "j" */
static const GLubyte Helvetica10_Character_107[] = {  5,  0,  0,144,144,160,192,160,144,128,128,  0,  0,  0}; /* "k" */
static const GLubyte Helvetica10_Character_108[] = {  2,  0,  0,128,128,128,128,128,128,128,128,  0,  0,  0}; /* "l" */
static const GLubyte Helvetica10_Character_109[] = {  8,  0,  0,146,146,146,146,146,236,  0,  0,  0,  0,  0}; /* "m" */
static const GLubyte Helvetica10_Character_110[] = {  6,  0,  0,136,136,136,136,200,176,  0,  0,  0,  0,  0}; /* "n" */
static const GLubyte Helvetica10_Character_111[] = {  6,  0,  0,112,136,136,136,136,112,  0,  0,  0,  0,  0}; /* "o" */
static const GLubyte Helvetica10_Character_112[] = {  6,128,128,176,200,136,136,200,176,  0,  0,  0,  0,  0}; /* "p" */
static const GLubyte Helvetica10_Character_113[] = {  6,  8,  8,104,152,136,136,152,104,  0,  0,  0,  0,  0}; /* "q" */
static const GLubyte Helvetica10_Character_114[] = {  4,  0,  0,128,128,128,128,192,160,  0,  0,  0,  0,  0}; /* "r" */
static const GLubyte Helvetica10_Character_115[] = {  5,  0,  0, 96,144, 16, 96,144, 96,  0,  0,  0,  0,  0}; /* "s" */
static const GLubyte Helvetica10_Character_116[] = {  4,  0,  0, 96, 64, 64, 64, 64,224, 64, 64,  0,  0,  0}; /* "t" */
static const GLubyte Helvetica10_Character_117[] = {  5,  0,  0,112,144,144,144,144,144,  0,  0,  0,  0,  0}; /* "u" */
static const GLubyte Helvetica10_Character_118[] = {  6,  0,  0, 32, 32, 80, 80,136,136,  0,  0,  0,  0,  0}; /* "v" */
static const GLubyte Helvetica10_Character_119[] = {  8,  0,  0, 40, 40, 84, 84,146,146,  0,  0,  0,  0,  0}; /* "w" */
static const GLubyte Helvetica10_Character_120[] = {  6,  0,  0,136,136, 80, 32, 80,136,  0,  0,  0,  0,  0}; /* "x" */
static const GLubyte Helvetica10_Character_121[] = {  5,128, 64, 64, 96,160,160,144,144,  0,  0,  0,  0,  0}; /* "y" */
static const GLubyte Helvetica10_Character_122[] = {  5,  0,  0,240,128, 64, 32, 16,240,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte Helvetica10_Character_065[] = {  7,  0,  0,130,130,124, 68, 40, 40, 16, 16,  0,  0,  0}; /* "A" */
static const GLubyte Helvetica10_Character_066[] = {  7,  0,  0,240,136,136,136,240,136,136,240,  0,  0,  0}; /* "B" */
static const GLubyte Helvetica10_Character_067[] = {  8,  0,  0,120,132,128,128,128,128,132,120,  0,  0,  0}; /* "C" */
static const GLubyte Helvetica10_Character_068[] = {  8,  0,  0,240,136,132,132,132,132,136,240,  0,  0,  0}; /* "D" */
static const GLubyte Helvetica10_Character_069[] = {  7,  0,  0,248,128,128,128,248,128,128,248,  0,  0,  0}; /* "E" */
static const GLubyte Helvetica10_Character_070[] = {  6,  0,  0,128,128,128,128,240,128,128,248,  0,  0,  0}; /* "F" */
static const GLubyte Helvetica10_Character_071[] = {  8,  0,  0,116,140,132,140,128,128,132,120,  0,  0,  0}; /* "G" */
static const GLubyte Helvetica10_Character_072[] = {  8,  0,  0,132,132,132,132,252,132,132,132,  0,  0,  0}; /* "H" */
static const GLubyte Helvetica10_Character_073[] = {  3,  0,  0,128,128,128,128,128,128,128,128,  0,  0,  0}; /* "I" */
static const GLubyte Helvetica10_Character_074[] = {  5,  0,  0, 96,144, 16, 16, 16, 16, 16, 16,  0,  0,  0}; /* "J" */
static const GLubyte Helvetica10_Character_075[] = {  7,  0,  0,136,136,144,144,224,160,144,136,  0,  0,  0}; /* "K" */
static const GLubyte Helvetica10_Character_076[] = {  6,  0,  0,240,128,128,128,128,128,128,128,  0,  0,  0}; /* "L" */
static const GLubyte Helvetica10_Character_077[] = {  9,  0,  0,  0,  0,146,  0,146,  0,146,  0,170,  0,170,  0,198,  0,198,  0,130,  0,  0,  0,  0,  0,  0,  0}; /* "M" */
static const GLubyte Helvetica10_Character_078[] = {  8,  0,  0,140,140,148,148,164,164,196,196,  0,  0,  0}; /* "N" */
static const GLubyte Helvetica10_Character_079[] = {  8,  0,  0,120,132,132,132,132,132,132,120,  0,  0,  0}; /* "O" */
static const GLubyte Helvetica10_Character_080[] = {  7,  0,  0,128,128,128,128,240,136,136,240,  0,  0,  0}; /* "P" */
static const GLubyte Helvetica10_Character_081[] = {  8,  0,  2,124,140,148,132,132,132,132,120,  0,  0,  0}; /* "Q" */
static const GLubyte Helvetica10_Character_082[] = {  7,  0,  0,136,136,136,136,240,136,136,240,  0,  0,  0}; /* "R" */
static const GLubyte Helvetica10_Character_083[] = {  7,  0,  0,112,136,136,  8,112,128,136,112,  0,  0,  0}; /* "S" */
static const GLubyte Helvetica10_Character_084[] = {  5,  0,  0, 32, 32, 32, 32, 32, 32, 32,248,  0,  0,  0}; /* "T" */
static const GLubyte Helvetica10_Character_085[] = {  8,  0,  0,120,132,132,132,132,132,132,132,  0,  0,  0}; /* "U" */
static const GLubyte Helvetica10_Character_086[] = {  7,  0,  0, 16, 40, 40, 68, 68, 68,130,130,  0,  0,  0}; /* "V" */
static const GLubyte Helvetica10_Character_087[] = {  9,  0,  0,  0,  0, 34,  0, 34,  0, 34,  0, 85,  0, 73,  0, 73,  0,136,128,136,128,  0,  0,  0,  0,  0,  0}; /* "W" */
static const GLubyte Helvetica10_Character_088[] = {  7,  0,  0,136,136, 80, 80, 32, 80,136,136,  0,  0,  0}; /* "X" */
static const GLubyte Helvetica10_Character_089[] = {  7,  0,  0, 16, 16, 16, 40, 40, 68, 68,130,  0,  0,  0}; /* "Y" */
static const GLubyte Helvetica10_Character_090[] = {  7,  0,  0,248,128, 64, 32, 32, 16,  8,248,  0,  0,  0}; /* "Z" */
static const GLubyte Helvetica10_Character_048[] = {  6,  0,  0,112,136,136,136,136,136,136,112,  0,  0,  0}; /* "0" */
static const GLubyte Helvetica10_Character_049[] = {  6,  0,  0, 64, 64, 64, 64, 64, 64,192, 64,  0,  0,  0}; /* "1" */
static const GLubyte Helvetica10_Character_050[] = {  6,  0,  0,248,128, 64, 48,  8,  8,136,112,  0,  0,  0}; /* "2" */
static const GLubyte Helvetica10_Character_051[] = {  6,  0,  0,112,136,  8,  8, 48,  8,136,112,  0,  0,  0}; /* "3" */
static const GLubyte Helvetica10_Character_052[] = {  6,  0,  0, 16, 16,248,144, 80, 80, 48, 16,  0,  0,  0}; /* "4" */
static const GLubyte Helvetica10_Character_053[] = {  6,  0,  0,112,136,  8,  8,240,128,128,248,  0,  0,  0}; /* "5" */
static const GLubyte Helvetica10_Character_054[] = {  6,  0,  0,112,136,136,200,176,128,136,112,  0,  0,  0}; /* "6" */
static const GLubyte Helvetica10_Character_055[] = {  6,  0,  0, 64, 64, 32, 32, 16, 16,  8,248,  0,  0,  0}; /* "7" */
static const GLubyte Helvetica10_Character_056[] = {  6,  0,  0,112,136,136,136,112,136,136,112,  0,  0,  0}; /* "8" */
static const GLubyte Helvetica10_Character_057[] = {  6,  0,  0,112,136,  8,104,152,136,136,112,  0,  0,  0}; /* "9" */
static const GLubyte Helvetica10_Character_096[] = {  3,  0,  0,  0,  0,  0,  0,  0,128,128, 64,  0,  0,  0}; /* "`" */
static const GLubyte Helvetica10_Character_126[] = {  7,  0,  0,  0,  0,  0,152,100,  0,  0,  0,  0,  0,  0}; /* "~" */
static const GLubyte Helvetica10_Character_033[] = {  3,  0,  0,128,  0,128,128,128,128,128,128,  0,  0,  0}; /* "!" */
static const GLubyte Helvetica10_Character_064[] = { 11, 62,  0, 64,  0,155,  0,164,128,164,128,162, 64,146, 64, 77, 64, 32,128, 31,  0,  0,  0,  0,  0,  0,  0}; /* "@" */
static const GLubyte Helvetica10_Character_035[] = {  6,  0,  0, 80, 80,248, 40,124, 40, 40,  0,  0,  0,  0}; /* "#" */
static const GLubyte Helvetica10_Character_036[] = {  6,  0, 32,112,168, 40,112,160,168,112, 32,  0,  0,  0}; /* "$" */
static const GLubyte Helvetica10_Character_037[] = {  9,  0,  0,  0,  0, 38,  0, 41,  0, 22,  0, 16,  0,  8,  0,104,  0,148,  0,100,  0,  0,  0,  0,  0,  0,  0}; /* "%" */
static const GLubyte Helvetica10_Character_094[] = {  6,  0,  0,  0,  0,  0,136, 80, 80, 32, 32,  0,  0,  0}; /* "^" */
static const GLubyte Helvetica10_Character_038[] = {  8,  0,  0,100,152,152,164, 96, 80, 80, 32,  0,  0,  0}; /* "&" */
static const GLubyte Helvetica10_Character_042[] = {  4,  0,  0,  0,  0,  0,  0,  0,160, 64,160,  0,  0,  0}; /* "*" */
static const GLubyte Helvetica10_Character_040[] = {  4, 32, 64, 64,128,128,128,128, 64, 64, 32,  0,  0,  0}; /* "(" */
static const GLubyte Helvetica10_Character_041[] = {  4,128, 64, 64, 32, 32, 32, 32, 64, 64,128,  0,  0,  0}; /* ")" */
static const GLubyte Helvetica10_Character_045[] = {  7,  0,  0,  0,  0,  0,248,  0,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte Helvetica10_Character_095[] = {  6,252,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte Helvetica10_Character_061[] = {  5,  0,  0,  0,  0,240,  0,240,  0,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte Helvetica10_Character_043[] = {  6,  0,  0,  0, 32, 32,248, 32, 32,  0,  0,  0,  0,  0}; /* "+" */
static const GLubyte Helvetica10_Character_091[] = {  3,192,128,128,128,128,128,128,128,128,192,  0,  0,  0}; /* "[" */
static const GLubyte Helvetica10_Character_123[] = {  3, 32, 64, 64, 64, 64,128, 64, 64, 64, 32,  0,  0,  0}; /* "{" */
static const GLubyte Helvetica10_Character_125[] = {  3,128, 64, 64, 64, 64, 32, 64, 64, 64,128,  0,  0,  0}; /* "}" */
static const GLubyte Helvetica10_Character_093[] = {  3,192, 64, 64, 64, 64, 64, 64, 64, 64,192,  0,  0,  0}; /* "]" */
static const GLubyte Helvetica10_Character_059[] = {  3,128, 64, 64,  0,  0,  0,  0, 64,  0,  0,  0,  0,  0}; /* ";" */
static const GLubyte Helvetica10_Character_058[] = {  3,  0,  0,128,  0,  0,  0,  0,128,  0,  0,  0,  0,  0}; /* ":" */
static const GLubyte Helvetica10_Character_044[] = {  3,128, 64, 64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte Helvetica10_Character_046[] = {  3,  0,  0,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte Helvetica10_Character_060[] = {  6,  0,  0,  0, 32, 64,128, 64, 32,  0,  0,  0,  0,  0}; /* "<" */
static const GLubyte Helvetica10_Character_062[] = {  6,  0,  0,  0,128, 64, 32, 64,128,  0,  0,  0,  0,  0}; /* ">" */
static const GLubyte Helvetica10_Character_047[] = {  3,  0,  0,128,128, 64, 64, 64, 64, 32, 32,  0,  0,  0}; /* "/" */
static const GLubyte Helvetica10_Character_063[] = {  6,  0,  0, 64,  0, 64, 64, 32, 16,144, 96,  0,  0,  0}; /* "?" */
static const GLubyte Helvetica10_Character_092[] = {  3,  0,  0, 32, 32, 64, 64, 64, 64,128,128,  0,  0,  0}; /* "\" */
static const GLubyte Helvetica10_Character_034[] = {  4,  0,  0,  0,  0,  0,  0,  0,  0,160,160,  0,  0,  0}; /* """ */

/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte Helvetica10_Character_039[] = {  3,  0,  0,  0,  0,  0,  0,  0,128, 64, 64,  0,  0,  0}; /* "'" */
static const GLubyte Helvetica10_Character_124[] = {  3, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,  0,  0}; /* "|" */


/* The font characters mapping: */
static const GLubyte* Helvetica10_Character_Map[] = {Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_032,Helvetica10_Character_033,Helvetica10_Character_034,Helvetica10_Character_035,Helvetica10_Character_036,Helvetica10_Character_037,
 Helvetica10_Character_038,Helvetica10_Character_039,Helvetica10_Character_040,Helvetica10_Character_041,Helvetica10_Character_042,Helvetica10_Character_043,Helvetica10_Character_044,Helvetica10_Character_045,Helvetica10_Character_046,Helvetica10_Character_047,Helvetica10_Character_048,Helvetica10_Character_049,Helvetica10_Character_050,Helvetica10_Character_051,Helvetica10_Character_052,Helvetica10_Character_053,Helvetica10_Character_054,Helvetica10_Character_055,Helvetica10_Character_056,Helvetica10_Character_057,Helvetica10_Character_058,Helvetica10_Character_059,Helvetica10_Character_060,Helvetica10_Character_061,Helvetica10_Character_062,Helvetica10_Character_063,Helvetica10_Character_064,Helvetica10_Character_065,Helvetica10_Character_066,Helvetica10_Character_067,Helvetica10_Character_068,Helvetica10_Character_069,Helvetica10_Character_070,Helvetica10_Character_071,Helvetica10_Character_072,Helvetica10_Character_073,Helvetica10_Character_074,Helvetica10_Character_075,Helvetica10_Character_076,
 Helvetica10_Character_077,Helvetica10_Character_078,Helvetica10_Character_079,Helvetica10_Character_080,Helvetica10_Character_081,Helvetica10_Character_082,Helvetica10_Character_083,Helvetica10_Character_084,Helvetica10_Character_085,Helvetica10_Character_086,Helvetica10_Character_087,Helvetica10_Character_088,Helvetica10_Character_089,Helvetica10_Character_090,Helvetica10_Character_091,Helvetica10_Character_092,Helvetica10_Character_093,Helvetica10_Character_094,Helvetica10_Character_095,Helvetica10_Character_096,Helvetica10_Character_097,Helvetica10_Character_098,Helvetica10_Character_099,Helvetica10_Character_100,Helvetica10_Character_101,Helvetica10_Character_102,Helvetica10_Character_103,Helvetica10_Character_104,Helvetica10_Character_105,Helvetica10_Character_106,Helvetica10_Character_107,Helvetica10_Character_108,Helvetica10_Character_109,Helvetica10_Character_110,Helvetica10_Character_111,Helvetica10_Character_112,Helvetica10_Character_113,Helvetica10_Character_114,Helvetica10_Character_115,
 Helvetica10_Character_116,Helvetica10_Character_117,Helvetica10_Character_118,Helvetica10_Character_119,Helvetica10_Character_120,Helvetica10_Character_121,Helvetica10_Character_122,Helvetica10_Character_123,Helvetica10_Character_124,Helvetica10_Character_125,Helvetica10_Character_126,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,
 Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,
 Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,
 Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,Helvetica10_Character_042,NULL};

/* The font structure: */
const BitmapFontData FontHelvetica10 = { "-adobe-helvetica-medium-r-normal--10-100-75-75-p-56-iso8859-1", 93, 13, Helvetica10_Character_Map, -1.0f, 2.0f };

static const GLubyte Helvetica12_Character_032[] = {  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* " " */
static const GLubyte Helvetica12_Character_097[] = {  7,  0,  0,  0,116,136,136,120,  8,136,112,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte Helvetica12_Character_098[] = {  7,  0,  0,  0,176,200,136,136,136,200,176,128,128,  0,  0,  0}; /* "b" */
static const GLubyte Helvetica12_Character_099[] = {  7,  0,  0,  0,112,136,128,128,128,136,112,  0,  0,  0,  0,  0}; /* "c" */
static const GLubyte Helvetica12_Character_100[] = {  7,  0,  0,  0,104,152,136,136,136,152,104,  8,  8,  0,  0,  0}; /* "d" */
static const GLubyte Helvetica12_Character_101[] = {  7,  0,  0,  0,112,136,128,248,136,136,112,  0,  0,  0,  0,  0}; /* "e" */
static const GLubyte Helvetica12_Character_102[] = {  3,  0,  0,  0, 64, 64, 64, 64, 64, 64,224, 64, 48,  0,  0,  0}; /* "f" */
static const GLubyte Helvetica12_Character_103[] = {  7,112,136,  8,104,152,136,136,136,152,104,  0,  0,  0,  0,  0}; /* "g" */
static const GLubyte Helvetica12_Character_104[] = {  7,  0,  0,  0,136,136,136,136,136,200,176,128,128,  0,  0,  0}; /* "h" */
static const GLubyte Helvetica12_Character_105[] = {  3,  0,  0,  0,128,128,128,128,128,128,128,  0,128,  0,  0,  0}; /* "i" */
static const GLubyte Helvetica12_Character_106[] = {  3,128, 64, 64, 64, 64, 64, 64, 64, 64, 64,  0, 64,  0,  0,  0}; /* "j" */
static const GLubyte Helvetica12_Character_107[] = {  6,  0,  0,  0,136,144,160,192,192,160,144,128,128,  0,  0,  0}; /* "k" */
static const GLubyte Helvetica12_Character_108[] = {  3,  0,  0,  0,128,128,128,128,128,128,128,128,128,  0,  0,  0}; /* "l" */
static const GLubyte Helvetica12_Character_109[] = {  9,  0,  0,  0,  0,  0,  0,146,  0,146,  0,146,  0,146,  0,146,  0,218,  0,164,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "m" */
static const GLubyte Helvetica12_Character_110[] = {  7,  0,  0,  0,136,136,136,136,136,200,176,  0,  0,  0,  0,  0}; /* "n" */
static const GLubyte Helvetica12_Character_111[] = {  7,  0,  0,  0,112,136,136,136,136,136,112,  0,  0,  0,  0,  0}; /* "o" */
static const GLubyte Helvetica12_Character_112[] = {  7,128,128,128,176,200,136,136,136,200,176,  0,  0,  0,  0,  0}; /* "p" */
static const GLubyte Helvetica12_Character_113[] = {  7,  8,  8,  8,104,152,136,136,136,152,104,  0,  0,  0,  0,  0}; /* "q" */
static const GLubyte Helvetica12_Character_114[] = {  4,  0,  0,  0,128,128,128,128,128,192,160,  0,  0,  0,  0,  0}; /* "r" */
static const GLubyte Helvetica12_Character_115[] = {  6,  0,  0,  0, 96,144, 16, 96,128,144, 96,  0,  0,  0,  0,  0}; /* "s" */
static const GLubyte Helvetica12_Character_116[] = {  4,  0,  0,  0, 96, 64, 64, 64, 64, 64,224, 64, 64,  0,  0,  0}; /* "t" */
static const GLubyte Helvetica12_Character_117[] = {  7,  0,  0,  0,104,152,136,136,136,136,136,  0,  0,  0,  0,  0}; /* "u" */
static const GLubyte Helvetica12_Character_118[] = {  7,  0,  0,  0, 32, 32, 80, 80,136,136,136,  0,  0,  0,  0,  0}; /* "v" */
static const GLubyte Helvetica12_Character_119[] = { 10,  0,  0,  0,  0,  0,  0, 34,  0, 34,  0, 85,  0, 73,  0, 73,  0,136,128,136,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "w" */
static const GLubyte Helvetica12_Character_120[] = {  6,  0,  0,  0,132,132, 72, 48, 48, 72,132,  0,  0,  0,  0,  0}; /* "x" */
static const GLubyte Helvetica12_Character_121[] = {  7,128, 64, 32, 32, 80, 80,144,136,136,136,  0,  0,  0,  0,  0}; /* "y" */
static const GLubyte Helvetica12_Character_122[] = {  6,  0,  0,  0,240,128, 64, 64, 32, 16,240,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte Helvetica12_Character_065[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,130,  0,124,  0, 68,  0, 68,  0, 40,  0, 40,  0, 16,  0,  0,  0,  0,  0,  0,  0}; /* "A" */
static const GLubyte Helvetica12_Character_066[] = {  8,  0,  0,  0,248,132,132,132,248,132,132,132,248,  0,  0,  0}; /* "B" */
static const GLubyte Helvetica12_Character_067[] = {  9,  0,  0,  0,  0,  0,  0, 60,  0, 66,  0,128,  0,128,  0,128,  0,128,  0,128,  0, 66,  0, 60,  0,  0,  0,  0,  0,  0,  0}; /* "C" */
static const GLubyte Helvetica12_Character_068[] = {  9,  0,  0,  0,  0,  0,  0,248,  0,132,  0,130,  0,130,  0,130,  0,130,  0,130,  0,132,  0,248,  0,  0,  0,  0,  0,  0,  0}; /* "D" */
static const GLubyte Helvetica12_Character_069[] = {  8,  0,  0,  0,252,128,128,128,252,128,128,128,252,  0,  0,  0}; /* "E" */
static const GLubyte Helvetica12_Character_070[] = {  8,  0,  0,  0,128,128,128,128,248,128,128,128,252,  0,  0,  0}; /* "F" */
static const GLubyte Helvetica12_Character_071[] = {  9,  0,  0,  0,  0,  0,  0, 58,  0, 70,  0,130,  0,130,  0,142,  0,128,  0,128,  0, 66,  0, 60,  0,  0,  0,  0,  0,  0,  0}; /* "G" */
static const GLubyte Helvetica12_Character_072[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,130,  0,130,  0,130,  0,254,  0,130,  0,130,  0,130,  0,130,  0,  0,  0,  0,  0,  0,  0}; /* "H" */
static const GLubyte Helvetica12_Character_073[] = {  3,  0,  0,  0,128,128,128,128,128,128,128,128,128,  0,  0,  0}; /* "I" */
static const GLubyte Helvetica12_Character_074[] = {  7,  0,  0,  0,112,136,136,  8,  8,  8,  8,  8,  8,  0,  0,  0}; /* "J" */
static const GLubyte Helvetica12_Character_075[] = {  8,  0,  0,  0,130,132,136,144,224,160,144,136,132,  0,  0,  0}; /* "K" */
static const GLubyte Helvetica12_Character_076[] = {  7,  0,  0,  0,248,128,128,128,128,128,128,128,128,  0,  0,  0}; /* "L" */
static const GLubyte Helvetica12_Character_077[] = { 11,  0,  0,  0,  0,  0,  0,136,128,136,128,148,128,148,128,162,128,162,128,193,128,193,128,128,128,  0,  0,  0,  0,  0,  0}; /* "M" */
static const GLubyte Helvetica12_Character_078[] = {  9,  0,  0,  0,  0,  0,  0,130,  0,134,  0,138,  0,138,  0,146,  0,162,  0,162,  0,194,  0,130,  0,  0,  0,  0,  0,  0,  0}; /* "N" */
static const GLubyte Helvetica12_Character_079[] = { 10,  0,  0,  0,  0,  0,  0, 60,  0, 66,  0,129,  0,129,  0,129,  0,129,  0,129,  0, 66,  0, 60,  0,  0,  0,  0,  0,  0,  0}; /* "O" */
static const GLubyte Helvetica12_Character_080[] = {  8,  0,  0,  0,128,128,128,128,248,132,132,132,248,  0,  0,  0}; /* "P" */
static const GLubyte Helvetica12_Character_081[] = { 10,  0,  0,  0,  0,  0,  0, 61,  0, 66,  0,133,  0,137,  0,129,  0,129,  0,129,  0, 66,  0, 60,  0,  0,  0,  0,  0,  0,  0}; /* "Q" */
static const GLubyte Helvetica12_Character_082[] = {  8,  0,  0,  0,132,132,132,136,248,132,132,132,248,  0,  0,  0}; /* "R" */
static const GLubyte Helvetica12_Character_083[] = {  8,  0,  0,  0,120,132,132,  4, 24, 96,128,132,120,  0,  0,  0}; /* "S" */
static const GLubyte Helvetica12_Character_084[] = {  7,  0,  0,  0, 16, 16, 16, 16, 16, 16, 16, 16,254,  0,  0,  0}; /* "T" */
static const GLubyte Helvetica12_Character_085[] = {  8,  0,  0,  0,120,132,132,132,132,132,132,132,132,  0,  0,  0}; /* "U" */
static const GLubyte Helvetica12_Character_086[] = {  9,  0,  0,  0,  0,  0,  0, 16,  0, 16,  0, 40,  0, 40,  0, 68,  0, 68,  0, 68,  0,130,  0,130,  0,  0,  0,  0,  0,  0,  0}; /* "V" */
static const GLubyte Helvetica12_Character_087[] = { 11,  0,  0,  0,  0,  0,  0, 34,  0, 34,  0, 34,  0, 85,  0, 85,  0, 73,  0,136,128,136,128,136,128,  0,  0,  0,  0,  0,  0}; /* "W" */
static const GLubyte Helvetica12_Character_088[] = {  9,  0,  0,  0,  0,  0,  0,130,  0, 68,  0, 68,  0, 40,  0, 16,  0, 40,  0, 68,  0, 68,  0,130,  0,  0,  0,  0,  0,  0,  0}; /* "X" */
static const GLubyte Helvetica12_Character_089[] = {  9,  0,  0,  0,  0,  0,  0, 16,  0, 16,  0, 16,  0, 16,  0, 40,  0, 68,  0, 68,  0,130,  0,130,  0,  0,  0,  0,  0,  0,  0}; /* "Y" */
static const GLubyte Helvetica12_Character_090[] = {  9,  0,  0,  0,  0,  0,  0,254,  0,128,  0, 64,  0, 32,  0, 16,  0,  8,  0,  4,  0,  2,  0,254,  0,  0,  0,  0,  0,  0,  0}; /* "Z" */
static const GLubyte Helvetica12_Character_048[] = {  7,  0,  0,  0,112,136,136,136,136,136,136,136,112,  0,  0,  0}; /* "0" */
static const GLubyte Helvetica12_Character_049[] = {  7,  0,  0,  0, 32, 32, 32, 32, 32, 32, 32,224, 32,  0,  0,  0}; /* "1" */
static const GLubyte Helvetica12_Character_050[] = {  7,  0,  0,  0,248,128,128, 64, 32, 16,  8,136,112,  0,  0,  0}; /* "2" */
static const GLubyte Helvetica12_Character_051[] = {  7,  0,  0,  0,112,136,136,  8,  8, 48,  8,136,112,  0,  0,  0}; /* "3" */
static const GLubyte Helvetica12_Character_052[] = {  7,  0,  0,  0,  8,  8,252,136, 72, 40, 40, 24,  8,  0,  0,  0}; /* "4" */
static const GLubyte Helvetica12_Character_053[] = {  7,  0,  0,  0,112,136,136,  8,  8,240,128,128,248,  0,  0,  0}; /* "5" */
static const GLubyte Helvetica12_Character_054[] = {  7,  0,  0,  0,112,136,136,136,200,176,128,136,112,  0,  0,  0}; /* "6" */
static const GLubyte Helvetica12_Character_055[] = {  7,  0,  0,  0, 64, 64, 32, 32, 32, 16, 16,  8,248,  0,  0,  0}; /* "7" */
static const GLubyte Helvetica12_Character_056[] = {  7,  0,  0,  0,112,136,136,136,136,112,136,136,112,  0,  0,  0}; /* "8" */
static const GLubyte Helvetica12_Character_057[] = {  7,  0,  0,  0,112,136,  8,  8,120,136,136,136,112,  0,  0,  0}; /* "9" */
static const GLubyte Helvetica12_Character_096[] = {  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,128, 64,  0,  0,  0}; /* "`" */
static const GLubyte Helvetica12_Character_126[] = {  8,  0,  0,  0,  0,  0,  0,152,100,  0,  0,  0,  0,  0,  0,  0}; /* "~" */
static const GLubyte Helvetica12_Character_033[] = {  3,  0,  0,  0,128,  0,128,128,128,128,128,128,128,  0,  0,  0}; /* "!" */
static const GLubyte Helvetica12_Character_064[] = { 12,  0,  0,  0,  0, 62,  0, 64,  0,155,  0,166,128,162, 64,162, 64,146, 64, 77, 64, 96,128, 31,  0,  0,  0,  0,  0,  0,  0}; /* "@" */
static const GLubyte Helvetica12_Character_035[] = {  7,  0,  0,  0, 80, 80, 80,252, 40,252, 40, 40,  0,  0,  0,  0}; /* "#" */
static const GLubyte Helvetica12_Character_036[] = {  7,  0,  0, 32,112,168,168, 40,112,160,168,112, 32,  0,  0,  0}; /* "$" */
static const GLubyte Helvetica12_Character_037[] = { 11,  0,  0,  0,  0,  0,  0, 35,  0, 20,128, 20,128, 19,  0,  8,  0,104,  0,148,  0,148,  0, 98,  0,  0,  0,  0,  0,  0,  0}; /* "%" */
static const GLubyte Helvetica12_Character_094[] = {  6,  0,  0,  0,  0,  0,  0,  0,  0,136, 80, 32,  0,  0,  0,  0}; /* "^" */
static const GLubyte Helvetica12_Character_038[] = {  9,  0,  0,  0,  0,  0,  0,114,  0,140,  0,132,  0,138,  0, 80,  0, 48,  0, 72,  0, 72,  0, 48,  0,  0,  0,  0,  0,  0,  0}; /* "&" */
static const GLubyte Helvetica12_Character_042[] = {  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,160, 64,160,  0,  0,  0}; /* "*" */
static const GLubyte Helvetica12_Character_040[] = {  4, 32, 64, 64,128,128,128,128,128,128, 64, 64, 32,  0,  0,  0}; /* "(" */
static const GLubyte Helvetica12_Character_041[] = {  4,128, 64, 64, 32, 32, 32, 32, 32, 32, 64, 64,128,  0,  0,  0}; /* ")" */
static const GLubyte Helvetica12_Character_045[] = {  8,  0,  0,  0,  0,  0,  0,248,  0,  0,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte Helvetica12_Character_095[] = {  7,  0,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte Helvetica12_Character_061[] = {  7,  0,  0,  0,  0,  0,248,  0,248,  0,  0,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte Helvetica12_Character_043[] = {  7,  0,  0,  0,  0, 32, 32,248, 32, 32,  0,  0,  0,  0,  0,  0}; /* "+" */
static const GLubyte Helvetica12_Character_091[] = {  3,192,128,128,128,128,128,128,128,128,128,128,192,  0,  0,  0}; /* "[" */
static const GLubyte Helvetica12_Character_123[] = {  4, 48, 64, 64, 64, 64, 64,128, 64, 64, 64, 64, 48,  0,  0,  0}; /* "{" */
static const GLubyte Helvetica12_Character_125[] = {  4,192, 32, 32, 32, 32, 32, 16, 32, 32, 32, 32,192,  0,  0,  0}; /* "}" */
static const GLubyte Helvetica12_Character_093[] = {  3,192, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,192,  0,  0,  0}; /* "]" */
static const GLubyte Helvetica12_Character_059[] = {  3,  0,128, 64, 64,  0,  0,  0,  0, 64,  0,  0,  0,  0,  0,  0}; /* ";" */
static const GLubyte Helvetica12_Character_058[] = {  3,  0,  0,  0,128,  0,  0,  0,  0,128,  0,  0,  0,  0,  0,  0}; /* ":" */
static const GLubyte Helvetica12_Character_044[] = {  4,  0,128, 64, 64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte Helvetica12_Character_046[] = {  3,  0,  0,  0,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte Helvetica12_Character_060[] = {  7,  0,  0,  0,  0, 12, 48,192, 48, 12,  0,  0,  0,  0,  0,  0}; /* "<" */
static const GLubyte Helvetica12_Character_062[] = {  7,  0,  0,  0,  0,192, 48, 12, 48,192,  0,  0,  0,  0,  0,  0}; /* ">" */
static const GLubyte Helvetica12_Character_047[] = {  4,  0,  0,  0,128,128,128, 64, 64, 64, 32, 32, 32,  0,  0,  0}; /* "/" */
static const GLubyte Helvetica12_Character_063[] = {  7,  0,  0,  0, 32,  0, 32, 32, 16, 16,136,136,112,  0,  0,  0}; /* "?" */
static const GLubyte Helvetica12_Character_092[] = {  4,  0,  0,  0, 32, 32, 32, 64, 64, 64,128,128,128,  0,  0,  0}; /* "\" */
static const GLubyte Helvetica12_Character_034[] = {  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,160,160,160,  0,  0,  0}; /* """ */

/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte Helvetica12_Character_039[] = {  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128, 64, 64,  0,  0}; /* "'" */
static const GLubyte Helvetica12_Character_124[] = {  3, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  0,  0}; /* "|" */
static const GLubyte Helvetica12_Character_131[] = {  7,  0,  128,  128,232,152,136,136,136,136,136,  0,  0,  0,  0,  0}; /* "mu" */


/* The font characters mapping: */
static const GLubyte* Helvetica12_Character_Map[] = {Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_032,Helvetica12_Character_033,Helvetica12_Character_034,Helvetica12_Character_035,Helvetica12_Character_036,Helvetica12_Character_037,
 Helvetica12_Character_038,Helvetica12_Character_039,Helvetica12_Character_040,Helvetica12_Character_041,Helvetica12_Character_042,Helvetica12_Character_043,Helvetica12_Character_044,Helvetica12_Character_045,Helvetica12_Character_046,Helvetica12_Character_047,Helvetica12_Character_048,Helvetica12_Character_049,Helvetica12_Character_050,Helvetica12_Character_051,Helvetica12_Character_052,Helvetica12_Character_053,Helvetica12_Character_054,Helvetica12_Character_055,Helvetica12_Character_056,Helvetica12_Character_057,Helvetica12_Character_058,Helvetica12_Character_059,Helvetica12_Character_060,Helvetica12_Character_061,Helvetica12_Character_062,Helvetica12_Character_063,Helvetica12_Character_064,Helvetica12_Character_065,Helvetica12_Character_066,Helvetica12_Character_067,Helvetica12_Character_068,Helvetica12_Character_069,Helvetica12_Character_070,Helvetica12_Character_071,Helvetica12_Character_072,Helvetica12_Character_073,Helvetica12_Character_074,Helvetica12_Character_075,Helvetica12_Character_076,
 Helvetica12_Character_077,Helvetica12_Character_078,Helvetica12_Character_079,Helvetica12_Character_080,Helvetica12_Character_081,Helvetica12_Character_082,Helvetica12_Character_083,Helvetica12_Character_084,Helvetica12_Character_085,Helvetica12_Character_086,Helvetica12_Character_087,Helvetica12_Character_088,Helvetica12_Character_089,Helvetica12_Character_090,Helvetica12_Character_091,Helvetica12_Character_092,Helvetica12_Character_093,Helvetica12_Character_094,Helvetica12_Character_095,Helvetica12_Character_096,Helvetica12_Character_097,Helvetica12_Character_098,Helvetica12_Character_099,Helvetica12_Character_100,Helvetica12_Character_101,Helvetica12_Character_102,Helvetica12_Character_103,Helvetica12_Character_104,Helvetica12_Character_105,Helvetica12_Character_106,Helvetica12_Character_107,Helvetica12_Character_108,Helvetica12_Character_109,Helvetica12_Character_110,Helvetica12_Character_111,Helvetica12_Character_112,Helvetica12_Character_113,Helvetica12_Character_114,Helvetica12_Character_115,
 Helvetica12_Character_116,Helvetica12_Character_117,Helvetica12_Character_118,Helvetica12_Character_119,Helvetica12_Character_120,Helvetica12_Character_121,Helvetica12_Character_122,Helvetica12_Character_123,Helvetica12_Character_124,Helvetica12_Character_125,Helvetica12_Character_126,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_131,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,
 Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,
 Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,
 Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,Helvetica12_Character_042,NULL};

/* The font structure: */
const BitmapFontData FontHelvetica12 = { "-adobe-helvetica-medium-r-normal--12-120-75-75-p-67-iso8859-1", 93, 15, Helvetica12_Character_Map, -1.0f, 3.0f };

static const GLubyte Helvetica18_Character_032[] = {  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* " " */
static const GLubyte Helvetica18_Character_097[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,118,  0,238,  0,198,  0,198,  0,230,  0,126,  0, 14,  0,198,  0,238,  0,124,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte Helvetica18_Character_098[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,222,  0,255,  0,227,  0,193,128,193,128,193,128,193,128,227,  0,255,  0,222,  0,192,  0,192,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "b" */
static const GLubyte Helvetica18_Character_099[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,127,  0, 99,  0,192,  0,192,  0,192,  0,192,  0, 99,  0,127,  0, 62,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "c" */
static const GLubyte Helvetica18_Character_100[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0, 61,128,127,128, 99,128,193,128,193,128,193,128,193,128, 99,128,127,128, 61,128,  1,128,  1,128,  1,128,  1,128,  0,  0,  0,  0,  0,  0,  0,  0}; /* "d" */
static const GLubyte Helvetica18_Character_101[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 60,  0,127,  0,227,  0,192,  0,192,  0,255,  0,195,  0,195,  0,126,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "e" */
static const GLubyte Helvetica18_Character_102[] = {  6,  0,  0,  0,  0, 48, 48, 48, 48, 48, 48, 48, 48,252,252, 48, 48, 60, 28,  0,  0,  0,  0}; /* "f" */
static const GLubyte Helvetica18_Character_103[] = { 11, 28,  0,127,  0, 99,  0,  1,128, 61,128,127,128, 99,128,193,128,193,128,193,128,193,128, 97,128,127,128, 61,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "g" */
static const GLubyte Helvetica18_Character_104[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,227,  0,223,  0,206,  0,192,  0,192,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "h" */
static const GLubyte Helvetica18_Character_105[] = {  4,  0,  0,  0,  0,192,192,192,192,192,192,192,192,192,192,  0,  0,192,192,  0,  0,  0,  0}; /* "i" */
static const GLubyte Helvetica18_Character_106[] = {  4,224,240, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,  0,  0, 48, 48,  0,  0,  0,  0}; /* "j" */
static const GLubyte Helvetica18_Character_107[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,199,  0,198,  0,206,  0,204,  0,216,  0,248,  0,240,  0,216,  0,204,  0,198,  0,192,  0,192,  0,192,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "k" */
static const GLubyte Helvetica18_Character_108[] = {  4,  0,  0,  0,  0,192,192,192,192,192,192,192,192,192,192,192,192,192,192,  0,  0,  0,  0}; /* "l" */
static const GLubyte Helvetica18_Character_109[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0,198, 48,198, 48,198, 48,198, 48,198, 48,198, 48,198, 48,231, 48,222,240,204, 96,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "m" */
static const GLubyte Helvetica18_Character_110[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,227,  0,223,  0,206,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "n" */
static const GLubyte Helvetica18_Character_111[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,127,  0, 99,  0,193,128,193,128,193,128,193,128, 99,  0,127,  0, 62,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "o" */
static const GLubyte Helvetica18_Character_112[] = { 11,192,  0,192,  0,192,  0,192,  0,222,  0,255,  0,227,  0,193,128,193,128,193,128,193,128,227,  0,255,  0,222,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "p" */
static const GLubyte Helvetica18_Character_113[] = { 11,  1,128,  1,128,  1,128,  1,128, 61,128,127,128, 99,128,193,128,193,128,193,128,193,128, 99,128,127,128, 61,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "q" */
static const GLubyte Helvetica18_Character_114[] = {  6,  0,  0,  0,  0,192,192,192,192,192,192,192,224,216,216,  0,  0,  0,  0,  0,  0,  0,  0}; /* "r" */
static const GLubyte Helvetica18_Character_115[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,120,  0,252,  0,198,  0,  6,  0, 62,  0,252,  0,192,  0,198,  0,126,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "s" */
static const GLubyte Helvetica18_Character_116[] = {  6,  0,  0,  0,  0, 24, 56, 48, 48, 48, 48, 48, 48,252,252, 48, 48, 48,  0,  0,  0,  0,  0}; /* "t" */
static const GLubyte Helvetica18_Character_117[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,115,  0,251,  0,199,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "u" */
static const GLubyte Helvetica18_Character_118[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 24,  0, 24,  0, 60,  0, 36,  0,102,  0,102,  0,102,  0,195,  0,195,  0,195,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "v" */
static const GLubyte Helvetica18_Character_119[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0, 25,128, 25,128, 57,192, 41, 64,105, 96,102, 96,102, 96,198, 48,198, 48,198, 48,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "w" */
static const GLubyte Helvetica18_Character_120[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,195,  0,231,  0,102,  0, 60,  0, 24,  0, 24,  0, 60,  0,102,  0,231,  0,195,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "x" */
static const GLubyte Helvetica18_Character_121[] = { 10,112,  0,112,  0, 24,  0, 24,  0, 24,  0, 24,  0, 60,  0, 36,  0,102,  0,102,  0,102,  0,195,  0,195,  0,195,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "y" */
static const GLubyte Helvetica18_Character_122[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,254,  0,254,  0,192,  0, 96,  0, 48,  0, 24,  0, 12,  0,  6,  0,254,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte Helvetica18_Character_065[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,192, 48,192, 48, 96, 96, 96, 96,127,224, 63,192, 48,192, 48,192, 25,128, 25,128, 15,  0, 15,  0,  6,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "A" */
static const GLubyte Helvetica18_Character_066[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,255,128,255,192,192,224,192, 96,192, 96,192,224,255,192,255,128,193,128,192,192,192,192,193,192,255,128,255,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "B" */
static const GLubyte Helvetica18_Character_067[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0, 15,128, 63,224,112,112, 96, 48,224,  0,192,  0,192,  0,192,  0,192,  0,224,  0, 96, 48,112,112, 63,224, 15,128,  0,  0,  0,  0,  0,  0,  0,  0}; /* "C" */
static const GLubyte Helvetica18_Character_068[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,255,128,193,192,192,192,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192,192,193,192,255,128,255,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "D" */
static const GLubyte Helvetica18_Character_069[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,255,128,255,128,192,  0,192,  0,192,  0,192,  0,255,  0,255,  0,192,  0,192,  0,192,  0,192,  0,255,128,255,128,  0,  0,  0,  0,  0,  0,  0,  0}; /* "E" */
static const GLubyte Helvetica18_Character_070[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0,255,  0,255,  0,192,  0,192,  0,192,  0,192,  0,255,128,255,128,  0,  0,  0,  0,  0,  0,  0,  0}; /* "F" */
static const GLubyte Helvetica18_Character_071[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0, 15,176, 63,240,112,112, 96, 48,224, 48,193,240,193,240,192,  0,192,  0,224, 48, 96, 48,112,112, 63,224, 15,128,  0,  0,  0,  0,  0,  0,  0,  0}; /* "G" */
static const GLubyte Helvetica18_Character_072[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,255,224,255,224,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,  0,  0,  0,  0,  0,  0,  0,  0}; /* "H" */
static const GLubyte Helvetica18_Character_073[] = {  6,  0,  0,  0,  0,192,192,192,192,192,192,192,192,192,192,192,192,192,192,  0,  0,  0,  0}; /* "I" */
static const GLubyte Helvetica18_Character_074[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 60,  0,126,  0,231,  0,195,  0,195,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "J" */
static const GLubyte Helvetica18_Character_075[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,192,112,192,224,193,192,195,128,199,  0,206,  0,252,  0,248,  0,220,  0,206,  0,199,  0,195,128,193,192,192,224,  0,  0,  0,  0,  0,  0,  0,  0}; /* "K" */
static const GLubyte Helvetica18_Character_076[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,255,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "L" */
static const GLubyte Helvetica18_Character_077[] = { 16,  0,  0,  0,  0,  0,  0,  0,  0,195, 12,195, 12,199,140,196,140,204,204,204,204,216,108,216,108,240, 60,240, 60,224, 28,224, 28,192, 12,192, 12,  0,  0,  0,  0,  0,  0,  0,  0}; /* "M" */
static const GLubyte Helvetica18_Character_078[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,192, 96,192,224,193,224,193,224,195, 96,198, 96,198, 96,204, 96,204, 96,216, 96,240, 96,240, 96,224, 96,192, 96,  0,  0,  0,  0,  0,  0,  0,  0}; /* "N" */
static const GLubyte Helvetica18_Character_079[] = { 15,  0,  0,  0,  0,  0,  0,  0,  0, 15,128, 63,224,112,112, 96, 48,224, 56,192, 24,192, 24,192, 24,192, 24,224, 56, 96, 48,112,112, 63,224, 15,128,  0,  0,  0,  0,  0,  0,  0,  0}; /* "O" */
static const GLubyte Helvetica18_Character_080[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0,255,  0,255,128,193,192,192,192,192,192,193,192,255,128,255,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "P" */
static const GLubyte Helvetica18_Character_081[] = { 15,  0,  0,  0,  0,  0,  0,  0, 48, 15,176, 63,224,112,240, 97,176,225,184,192, 24,192, 24,192, 24,192, 24,224, 56, 96, 48,112,112, 63,224, 15,128,  0,  0,  0,  0,  0,  0,  0,  0}; /* "Q" */
static const GLubyte Helvetica18_Character_082[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,192,192,192,192,192,192,192,192,193,128,193,128,255,  0,255,128,193,192,192,192,192,192,193,192,255,128,255,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "R" */
static const GLubyte Helvetica18_Character_083[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0, 63,  0,127,192,224,224,192, 96,  0, 96,  0,224,  3,192, 31,  0,124,  0,224,  0,192, 96,224,224,127,192, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "S" */
static const GLubyte Helvetica18_Character_084[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0, 12,  0, 12,  0, 12,  0, 12,  0, 12,  0, 12,  0, 12,  0, 12,  0, 12,  0, 12,  0, 12,  0, 12,  0,255,192,255,192,  0,  0,  0,  0,  0,  0,  0,  0}; /* "T" */
static const GLubyte Helvetica18_Character_085[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0, 31,  0,127,192, 96,192,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,  0,  0,  0,  0,  0,  0,  0,  0}; /* "U" */
static const GLubyte Helvetica18_Character_086[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0, 15,  0, 15,  0, 25,128, 25,128, 25,128, 48,192, 48,192, 48,192, 96, 96, 96, 96, 96, 96,192, 48,192, 48,  0,  0,  0,  0,  0,  0,  0,  0}; /* "V" */
static const GLubyte Helvetica18_Character_087[] = { 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 24, 24,  0, 24, 24,  0, 28, 56,  0, 52, 44,  0, 54,108,  0, 54,108,  0,102,102,  0,102,102,  0, 98, 70,  0, 99,198,  0,195,195,  0,193,131,  0,193,131,  0,193,131,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "W" */
static const GLubyte Helvetica18_Character_088[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,192, 96,224,224, 96,192,113,192, 49,128, 27,  0, 14,  0, 14,  0, 27,  0, 49,128,113,192, 96,192,224,224,192, 96,  0,  0,  0,  0,  0,  0,  0,  0}; /* "X" */
static const GLubyte Helvetica18_Character_089[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0, 15,  0, 25,128, 48,192, 48,192, 96, 96, 96, 96,192, 48,192, 48,  0,  0,  0,  0,  0,  0,  0,  0}; /* "Y" */
static const GLubyte Helvetica18_Character_090[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,255,192,255,192,192,  0, 96,  0, 48,  0, 24,  0, 28,  0, 12,  0,  6,  0,  3,  0,  1,128,  0,192,255,192,255,192,  0,  0,  0,  0,  0,  0,  0,  0}; /* "Z" */
static const GLubyte Helvetica18_Character_048[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 60,  0,126,  0,102,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,102,  0,126,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "0" */
static const GLubyte Helvetica18_Character_049[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0,248,  0,248,  0, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "1" */
static const GLubyte Helvetica18_Character_050[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,255,  0,192,  0,224,  0,112,  0, 56,  0, 28,  0, 14,  0,  7,  0,  3,  0,195,  0,254,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "2" */
static const GLubyte Helvetica18_Character_051[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 60,  0,126,  0,199,  0,195,  0,  3,  0,  7,  0, 30,  0, 28,  0,  6,  0,195,  0,195,  0,126,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "3" */
static const GLubyte Helvetica18_Character_052[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  3,  0,  3,  0,255,128,255,128,195,  0, 99,  0, 51,  0, 51,  0, 27,  0, 15,  0,  7,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "4" */
static const GLubyte Helvetica18_Character_053[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,124,  0,254,  0,199,  0,195,  0,  3,  0,  3,  0,199,  0,254,  0,252,  0,192,  0,192,  0,254,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "5" */
static const GLubyte Helvetica18_Character_054[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 60,  0,126,  0,227,  0,195,  0,195,  0,195,  0,254,  0,220,  0,192,  0,192,  0, 99,  0,127,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "6" */
static const GLubyte Helvetica18_Character_055[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 96,  0, 96,  0, 48,  0, 48,  0, 48,  0, 24,  0, 24,  0, 12,  0, 12,  0,  6,  0,  3,  0,255,  0,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "7" */
static const GLubyte Helvetica18_Character_056[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 60,  0,126,  0,231,  0,195,  0,195,  0,102,  0,126,  0,102,  0,195,  0,195,  0,231,  0,126,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "8" */
static const GLubyte Helvetica18_Character_057[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,124,  0,254,  0,198,  0,  3,  0,  3,  0, 59,  0,127,  0,195,  0,195,  0,195,  0,199,  0,126,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "9" */
static const GLubyte Helvetica18_Character_096[] = {  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,192,128,128, 64,  0,  0,  0,  0}; /* "`" */
static const GLubyte Helvetica18_Character_126[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,204,  0,126,  0, 51,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "~" */
static const GLubyte Helvetica18_Character_033[] = {  6,  0,  0,  0,  0,192,192,  0,  0,128,128,192,192,192,192,192,192,192,192,  0,  0,  0,  0}; /* "!" */
static const GLubyte Helvetica18_Character_064[] = { 18,  0,  0,  0,  7,224,  0, 31,240,  0, 56,  0,  0,112,  0,  0,103,112,  0,207,248,  0,204,204,  0,204,102,  0,204,102,  0,204, 99,  0,198, 51,  0,103,115,  0, 99,179,  0, 48,  6,  0, 28, 14,  0, 15,252,  0,  3,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "@" */
static const GLubyte Helvetica18_Character_035[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 36,  0, 36,  0, 36,  0,255,128,255,128, 18,  0, 18,  0, 18,  0,127,192,127,192,  9,  0,  9,  0,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "#" */
static const GLubyte Helvetica18_Character_036[] = { 10,  0,  0,  0,  0,  8,  0,  8,  0, 62,  0,127,  0,235,128,201,128,  9,128, 15,  0, 62,  0,120,  0,232,  0,200,  0,203,  0,127,  0, 62,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "$" */
static const GLubyte Helvetica18_Character_037[] = { 16,  0,  0,  0,  0,  0,  0,  0,  0, 24,120, 24,252, 12,204, 12,204,  6,252,  6,120,  3,  0,123,  0,253,128,205,128,204,192,252,192,120, 96,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "%" */
static const GLubyte Helvetica18_Character_094[] = {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,130,  0,198,  0,108,  0, 56,  0, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "^" */
static const GLubyte Helvetica18_Character_038[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0, 60,112,126,224,231,192,195,128,195,192,198,192,238,192,124,  0, 60,  0,102,  0,102,  0,126,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "&" */
static const GLubyte Helvetica18_Character_042[] = {  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,136,112,112,248, 32, 32,  0,  0,  0,  0}; /* "*" */
static const GLubyte Helvetica18_Character_040[] = {  6, 16, 48, 96, 96,192,192,192,192,192,192,192,192,192,192, 96, 96, 48, 16,  0,  0,  0,  0}; /* "(" */
static const GLubyte Helvetica18_Character_041[] = {  6,128,192, 96, 96, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 96, 96,192,128,  0,  0,  0,  0}; /* ")" */
static const GLubyte Helvetica18_Character_045[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte Helvetica18_Character_095[] = { 10,255,248,255,248,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte Helvetica18_Character_061[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,254,  0,254,  0,  0,  0,  0,  0,254,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte Helvetica18_Character_043[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 24,  0, 24,  0, 24,  0, 24,  0,255,  0,255,  0, 24,  0, 24,  0, 24,  0, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "+" */
static const GLubyte Helvetica18_Character_091[] = {  5,240,240,192,192,192,192,192,192,192,192,192,192,192,192,192,192,240,240,  0,  0,  0,  0}; /* "[" */
static const GLubyte Helvetica18_Character_123[] = {  7, 12, 24, 48, 48, 48, 48, 48, 48, 96,192, 96, 48, 48, 48, 48, 48, 24, 12,  0,  0,  0,  0}; /* "{" */
static const GLubyte Helvetica18_Character_125[] = {  7,192, 96, 48, 48, 48, 48, 48, 48, 24, 12, 24, 48, 48, 48, 48, 48, 96,192,  0,  0,  0,  0}; /* "}" */
static const GLubyte Helvetica18_Character_093[] = {  5,240,240, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,240,240,  0,  0,  0,  0}; /* "]" */
static const GLubyte Helvetica18_Character_059[] = {  5,  0,128, 64, 64,192,192,  0,  0,  0,  0,  0,  0,192,192,  0,  0,  0,  0,  0,  0,  0,  0}; /* ";" */
static const GLubyte Helvetica18_Character_058[] = {  5,  0,  0,  0,  0,192,192,  0,  0,  0,  0,  0,  0,192,192,  0,  0,  0,  0,  0,  0,  0,  0}; /* ":" */
static const GLubyte Helvetica18_Character_044[] = {  5,  0,128, 64, 64,192,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte Helvetica18_Character_046[] = {  5,  0,  0,  0,  0,192,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte Helvetica18_Character_060[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0, 15,  0, 60,  0,112,  0,192,  0,112,  0, 60,  0, 15,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "<" */
static const GLubyte Helvetica18_Character_062[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,192,  0,240,  0, 60,  0, 14,  0,  3,  0, 14,  0, 60,  0,240,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* ">" */
static const GLubyte Helvetica18_Character_047[] = {  5,  0,  0,  0,  0,192,192, 64, 64, 96, 96, 32, 32, 48, 48, 16, 16, 24, 24,  0,  0,  0,  0}; /* "/" */
static const GLubyte Helvetica18_Character_063[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0, 48,  0, 48,  0,  0,  0,  0,  0, 48,  0, 48,  0, 48,  0, 56,  0, 28,  0, 14,  0,198,  0,198,  0,254,  0,124,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "?" */
static const GLubyte Helvetica18_Character_092[] = {  5,  0,  0,  0,  0, 24, 24, 16, 16, 48, 48, 32, 32, 96, 96, 64, 64,192,192,  0,  0,  0,  0}; /* "\" */
static const GLubyte Helvetica18_Character_034[] = {  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,144,144,216,216,216,  0,  0,  0,  0}; /* """ */

/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte Helvetica18_Character_039[] = {  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128, 64, 64,192,192,  0,  0,  0,  0}; /* "'" */
static const GLubyte Helvetica18_Character_124[] = {  4, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96}; /* "|" */
static const GLubyte Helvetica18_Character_131[] = { 10,  0,  0,  0,  0,192,  0,192,  0,243,  0,251,  0,199,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,195,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "mu" */


/* The font characters mapping: */
static const GLubyte* Helvetica18_Character_Map[] = {Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_032,Helvetica18_Character_033,Helvetica18_Character_034,Helvetica18_Character_035,Helvetica18_Character_036,Helvetica18_Character_037,
 Helvetica18_Character_038,Helvetica18_Character_039,Helvetica18_Character_040,Helvetica18_Character_041,Helvetica18_Character_042,Helvetica18_Character_043,Helvetica18_Character_044,Helvetica18_Character_045,Helvetica18_Character_046,Helvetica18_Character_047,Helvetica18_Character_048,Helvetica18_Character_049,Helvetica18_Character_050,Helvetica18_Character_051,Helvetica18_Character_052,Helvetica18_Character_053,Helvetica18_Character_054,Helvetica18_Character_055,Helvetica18_Character_056,Helvetica18_Character_057,Helvetica18_Character_058,Helvetica18_Character_059,Helvetica18_Character_060,Helvetica18_Character_061,Helvetica18_Character_062,Helvetica18_Character_063,Helvetica18_Character_064,Helvetica18_Character_065,Helvetica18_Character_066,Helvetica18_Character_067,Helvetica18_Character_068,Helvetica18_Character_069,Helvetica18_Character_070,Helvetica18_Character_071,Helvetica18_Character_072,Helvetica18_Character_073,Helvetica18_Character_074,Helvetica18_Character_075,Helvetica18_Character_076,
 Helvetica18_Character_077,Helvetica18_Character_078,Helvetica18_Character_079,Helvetica18_Character_080,Helvetica18_Character_081,Helvetica18_Character_082,Helvetica18_Character_083,Helvetica18_Character_084,Helvetica18_Character_085,Helvetica18_Character_086,Helvetica18_Character_087,Helvetica18_Character_088,Helvetica18_Character_089,Helvetica18_Character_090,Helvetica18_Character_091,Helvetica18_Character_092,Helvetica18_Character_093,Helvetica18_Character_094,Helvetica18_Character_095,Helvetica18_Character_096,Helvetica18_Character_097,Helvetica18_Character_098,Helvetica18_Character_099,Helvetica18_Character_100,Helvetica18_Character_101,Helvetica18_Character_102,Helvetica18_Character_103,Helvetica18_Character_104,Helvetica18_Character_105,Helvetica18_Character_106,Helvetica18_Character_107,Helvetica18_Character_108,Helvetica18_Character_109,Helvetica18_Character_110,Helvetica18_Character_111,Helvetica18_Character_112,Helvetica18_Character_113,Helvetica18_Character_114,Helvetica18_Character_115,
 Helvetica18_Character_116,Helvetica18_Character_117,Helvetica18_Character_118,Helvetica18_Character_119,Helvetica18_Character_120,Helvetica18_Character_121,Helvetica18_Character_122,Helvetica18_Character_123,Helvetica18_Character_124,Helvetica18_Character_125,Helvetica18_Character_126,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_131,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,
 Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,
 Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,
 Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,Helvetica18_Character_042,NULL};

/* The font structure: */
const BitmapFontData FontHelvetica18 = { "-adobe-helvetica-medium-r-normal--18-180-75-75-p-98-iso8859-1", 93, 22, Helvetica18_Character_Map, -1.0f, 4.0f };

static const GLubyte TimesRoman10_Character_032[] = {  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* " " */
static const GLubyte TimesRoman10_Character_097[] = {  4,  0,  0,  0,224,160, 96, 32,192,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte TimesRoman10_Character_098[] = {  5,  0,  0,  0,224,144,144,144,224,128,128,  0,  0,  0}; /* "b" */
static const GLubyte TimesRoman10_Character_099[] = {  4,  0,  0,  0, 96,128,128,128, 96,  0,  0,  0,  0,  0}; /* "c" */
static const GLubyte TimesRoman10_Character_100[] = {  5,  0,  0,  0,104,144,144,144,112, 16, 48,  0,  0,  0}; /* "d" */
static const GLubyte TimesRoman10_Character_101[] = {  4,  0,  0,  0, 96,128,192,160, 96,  0,  0,  0,  0,  0}; /* "e" */
static const GLubyte TimesRoman10_Character_102[] = {  4,  0,  0,  0,224, 64, 64, 64,224, 64, 48,  0,  0,  0}; /* "f" */
static const GLubyte TimesRoman10_Character_103[] = {  5,  0,224,144, 96, 64,160,160,112,  0,  0,  0,  0,  0}; /* "g" */
static const GLubyte TimesRoman10_Character_104[] = {  5,  0,  0,  0,216,144,144,144,224,128,128,  0,  0,  0}; /* "h" */
static const GLubyte TimesRoman10_Character_105[] = {  3,  0,  0,  0, 64, 64, 64, 64,192,  0, 64,  0,  0,  0}; /* "i" */
static const GLubyte TimesRoman10_Character_106[] = {  3,  0,128, 64, 64, 64, 64, 64,192,  0, 64,  0,  0,  0}; /* "j" */
static const GLubyte TimesRoman10_Character_107[] = {  5,  0,  0,  0,152,144,224,160,144,128,128,  0,  0,  0}; /* "k" */
static const GLubyte TimesRoman10_Character_108[] = {  4,  0,  0,  0,224, 64, 64, 64, 64, 64,192,  0,  0,  0}; /* "l" */
static const GLubyte TimesRoman10_Character_109[] = {  8,  0,  0,  0,219,146,146,146,236,  0,  0,  0,  0,  0}; /* "m" */
static const GLubyte TimesRoman10_Character_110[] = {  5,  0,  0,  0,216,144,144,144,224,  0,  0,  0,  0,  0}; /* "n" */
static const GLubyte TimesRoman10_Character_111[] = {  5,  0,  0,  0, 96,144,144,144, 96,  0,  0,  0,  0,  0}; /* "o" */
static const GLubyte TimesRoman10_Character_112[] = {  5,  0,192,128,224,144,144,144,224,  0,  0,  0,  0,  0}; /* "p" */
static const GLubyte TimesRoman10_Character_113[] = {  5,  0, 56, 16,112,144,144,144,112,  0,  0,  0,  0,  0}; /* "q" */
static const GLubyte TimesRoman10_Character_114[] = {  4,  0,  0,  0,224, 64, 64, 96,160,  0,  0,  0,  0,  0}; /* "r" */
static const GLubyte TimesRoman10_Character_115[] = {  4,  0,  0,  0,224, 32, 96,128,224,  0,  0,  0,  0,  0}; /* "s" */
static const GLubyte TimesRoman10_Character_116[] = {  4,  0,  0,  0, 48, 64, 64, 64,224, 64,  0,  0,  0,  0}; /* "t" */
static const GLubyte TimesRoman10_Character_117[] = {  5,  0,  0,  0,104,144,144,144,144,  0,  0,  0,  0,  0}; /* "u" */
static const GLubyte TimesRoman10_Character_118[] = {  5,  0,  0,  0, 32, 96, 80,144,216,  0,  0,  0,  0,  0}; /* "v" */
static const GLubyte TimesRoman10_Character_119[] = {  8,  0,  0,  0, 40,108, 84,146,219,  0,  0,  0,  0,  0}; /* "w" */
static const GLubyte TimesRoman10_Character_120[] = {  6,  0,  0,  0,216, 80, 32, 80,216,  0,  0,  0,  0,  0}; /* "x" */
static const GLubyte TimesRoman10_Character_121[] = {  5,  0, 64, 64, 32, 48, 80, 72,220,  0,  0,  0,  0,  0}; /* "y" */
static const GLubyte TimesRoman10_Character_122[] = {  5,  0,  0,  0,240,144, 64, 32,240,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte TimesRoman10_Character_065[] = {  8,  0,  0,  0,238, 68,124, 40, 40, 56, 16,  0,  0,  0}; /* "A" */
static const GLubyte TimesRoman10_Character_066[] = {  6,  0,  0,  0,240, 72, 72,112, 72, 72,240,  0,  0,  0}; /* "B" */
static const GLubyte TimesRoman10_Character_067[] = {  7,  0,  0,  0,120,196,128,128,128,196,124,  0,  0,  0}; /* "C" */
static const GLubyte TimesRoman10_Character_068[] = {  7,  0,  0,  0,248, 76, 68, 68, 68, 76,248,  0,  0,  0}; /* "D" */
static const GLubyte TimesRoman10_Character_069[] = {  6,  0,  0,  0,248, 72, 64,112, 64, 72,248,  0,  0,  0}; /* "E" */
static const GLubyte TimesRoman10_Character_070[] = {  6,  0,  0,  0,224, 64, 64,112, 64, 72,248,  0,  0,  0}; /* "F" */
static const GLubyte TimesRoman10_Character_071[] = {  7,  0,  0,  0,120,196,132,156,128,196,124,  0,  0,  0}; /* "G" */
static const GLubyte TimesRoman10_Character_072[] = {  8,  0,  0,  0,238, 68, 68,124, 68, 68,238,  0,  0,  0}; /* "H" */
static const GLubyte TimesRoman10_Character_073[] = {  4,  0,  0,  0,224, 64, 64, 64, 64, 64,224,  0,  0,  0}; /* "I" */
static const GLubyte TimesRoman10_Character_074[] = {  4,  0,  0,  0,192,160, 32, 32, 32, 32,112,  0,  0,  0}; /* "J" */
static const GLubyte TimesRoman10_Character_075[] = {  7,  0,  0,  0,236, 72, 80, 96, 80, 72,236,  0,  0,  0}; /* "K" */
static const GLubyte TimesRoman10_Character_076[] = {  6,  0,  0,  0,248, 72, 64, 64, 64, 64,224,  0,  0,  0}; /* "L" */
static const GLubyte TimesRoman10_Character_077[] = { 10,  0,  0,  0,  0,  0,  0,235,128, 73,  0, 85,  0, 85,  0, 99,  0, 99,  0,227,128,  0,  0,  0,  0,  0,  0}; /* "M" */
static const GLubyte TimesRoman10_Character_078[] = {  8,  0,  0,  0,228, 76, 76, 84, 84,100,238,  0,  0,  0}; /* "N" */
static const GLubyte TimesRoman10_Character_079[] = {  7,  0,  0,  0,120,204,132,132,132,204,120,  0,  0,  0}; /* "O" */
static const GLubyte TimesRoman10_Character_080[] = {  6,  0,  0,  0,224, 64, 64,112, 72, 72,240,  0,  0,  0}; /* "P" */
static const GLubyte TimesRoman10_Character_081[] = {  7,  0, 12, 24,112,204,132,132,132,204,120,  0,  0,  0}; /* "Q" */
static const GLubyte TimesRoman10_Character_082[] = {  7,  0,  0,  0,236, 72, 80,112, 72, 72,240,  0,  0,  0}; /* "R" */
static const GLubyte TimesRoman10_Character_083[] = {  5,  0,  0,  0,224,144, 16, 96,192,144,112,  0,  0,  0}; /* "S" */
static const GLubyte TimesRoman10_Character_084[] = {  6,  0,  0,  0,112, 32, 32, 32, 32,168,248,  0,  0,  0}; /* "T" */
static const GLubyte TimesRoman10_Character_085[] = {  8,  0,  0,  0, 56,108, 68, 68, 68, 68,238,  0,  0,  0}; /* "U" */
static const GLubyte TimesRoman10_Character_086[] = {  8,  0,  0,  0, 16, 16, 40, 40,108, 68,238,  0,  0,  0}; /* "V" */
static const GLubyte TimesRoman10_Character_087[] = { 10,  0,  0,  0,  0,  0,  0, 34,  0, 34,  0, 85,  0, 85,  0,201,128,136,128,221,192,  0,  0,  0,  0,  0,  0}; /* "W" */
static const GLubyte TimesRoman10_Character_088[] = {  8,  0,  0,  0,238, 68, 40, 16, 40, 68,238,  0,  0,  0}; /* "X" */
static const GLubyte TimesRoman10_Character_089[] = {  8,  0,  0,  0, 56, 16, 16, 40, 40, 68,238,  0,  0,  0}; /* "Y" */
static const GLubyte TimesRoman10_Character_090[] = {  6,  0,  0,  0,248,136, 64, 32, 16,136,248,  0,  0,  0}; /* "Z" */
static const GLubyte TimesRoman10_Character_048[] = {  5,  0,  0,  0, 96,144,144,144,144,144, 96,  0,  0,  0}; /* "0" */
static const GLubyte TimesRoman10_Character_049[] = {  5,  0,  0,  0,224, 64, 64, 64, 64,192, 64,  0,  0,  0}; /* "1" */
static const GLubyte TimesRoman10_Character_050[] = {  5,  0,  0,  0,240, 64, 32, 32, 16,144, 96,  0,  0,  0}; /* "2" */
static const GLubyte TimesRoman10_Character_051[] = {  5,  0,  0,  0,224, 16, 16, 96, 16,144, 96,  0,  0,  0}; /* "3" */
static const GLubyte TimesRoman10_Character_052[] = {  5,  0,  0,  0, 16, 16,248,144, 80, 48, 16,  0,  0,  0}; /* "4" */
static const GLubyte TimesRoman10_Character_053[] = {  5,  0,  0,  0,224,144, 16, 16,224, 64,112,  0,  0,  0}; /* "5" */
static const GLubyte TimesRoman10_Character_054[] = {  5,  0,  0,  0, 96,144,144,144,224, 64, 48,  0,  0,  0}; /* "6" */
static const GLubyte TimesRoman10_Character_055[] = {  5,  0,  0,  0, 64, 64, 64, 32, 32,144,240,  0,  0,  0}; /* "7" */
static const GLubyte TimesRoman10_Character_056[] = {  5,  0,  0,  0, 96,144,144, 96,144,144, 96,  0,  0,  0}; /* "8" */
static const GLubyte TimesRoman10_Character_057[] = {  5,  0,  0,  0,192, 32,112,144,144,144, 96,  0,  0,  0}; /* "9" */
static const GLubyte TimesRoman10_Character_096[] = {  3,  0,  0,  0,  0,  0,  0,  0,  0,192,128,  0,  0,  0}; /* "`" */
static const GLubyte TimesRoman10_Character_126[] = {  7,  0,  0,  0,  0,  0,152,100,  0,  0,  0,  0,  0,  0}; /* "~" */
static const GLubyte TimesRoman10_Character_033[] = {  3,  0,  0,  0,128,  0,128,128,128,128,128,  0,  0,  0}; /* "!" */
static const GLubyte TimesRoman10_Character_064[] = {  9,  0,  0, 62,  0, 64,  0,146,  0,173,  0,165,  0,165,  0,157,  0, 66,  0, 60,  0,  0,  0,  0,  0,  0,  0}; /* "@" */
static const GLubyte TimesRoman10_Character_035[] = {  5,  0,  0,  0, 80, 80,248, 80,248, 80, 80,  0,  0,  0}; /* "#" */
static const GLubyte TimesRoman10_Character_036[] = {  5,  0,  0, 32,224,144, 16, 96,128,144,112, 32,  0,  0}; /* "$" */
static const GLubyte TimesRoman10_Character_037[] = {  8,  0,  0,  0, 68, 42, 42, 86,168,164,126,  0,  0,  0}; /* "%" */
static const GLubyte TimesRoman10_Character_094[] = {  5,  0,  0,  0,  0,  0,  0,  0,160,160, 64,  0,  0,  0}; /* "^" */
static const GLubyte TimesRoman10_Character_038[] = {  8,  0,  0,  0,118,141,152,116,110, 80, 48,  0,  0,  0}; /* "&" */
static const GLubyte TimesRoman10_Character_042[] = {  5,  0,  0,  0,  0,  0,  0,  0,160, 64,160,  0,  0,  0}; /* "*" */
static const GLubyte TimesRoman10_Character_040[] = {  4,  0, 32, 64, 64,128,128,128, 64, 64, 32,  0,  0,  0}; /* "(" */
static const GLubyte TimesRoman10_Character_041[] = {  4,  0,128, 64, 64, 32, 32, 32, 64, 64,128,  0,  0,  0}; /* ")" */
static const GLubyte TimesRoman10_Character_045[] = {  7,  0,  0,  0,  0,  0,240,  0,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte TimesRoman10_Character_095[] = {  5,252,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte TimesRoman10_Character_061[] = {  6,  0,  0,  0,  0,248,  0,248,  0,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte TimesRoman10_Character_043[] = {  6,  0,  0,  0, 32, 32,248, 32, 32,  0,  0,  0,  0,  0}; /* "+" */
static const GLubyte TimesRoman10_Character_091[] = {  3,  0,192,128,128,128,128,128,128,128,192,  0,  0,  0}; /* "[" */
static const GLubyte TimesRoman10_Character_123[] = {  4,  0, 32, 64, 64, 64,128, 64, 64, 64, 32,  0,  0,  0}; /* "{" */
static const GLubyte TimesRoman10_Character_125[] = {  4,  0,128, 64, 64, 64, 32, 64, 64, 64,128,  0,  0,  0}; /* "}" */
static const GLubyte TimesRoman10_Character_093[] = {  3,  0,192, 64, 64, 64, 64, 64, 64, 64,192,  0,  0,  0}; /* "]" */
static const GLubyte TimesRoman10_Character_059[] = {  3,  0,128,128,128,  0,  0,  0,128,  0,  0,  0,  0,  0}; /* ";" */
static const GLubyte TimesRoman10_Character_058[] = {  3,  0,  0,  0,128,  0,  0,  0,128,  0,  0,  0,  0,  0}; /* ":" */
static const GLubyte TimesRoman10_Character_044[] = {  3,  0,128,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte TimesRoman10_Character_046[] = {  3,  0,  0,  0,128,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte TimesRoman10_Character_060[] = {  5,  0,  0,  0, 32, 64,128, 64, 32,  0,  0,  0,  0,  0}; /* "<" */
static const GLubyte TimesRoman10_Character_062[] = {  5,  0,  0,  0,128, 64, 32, 64,128,  0,  0,  0,  0,  0}; /* ">" */
static const GLubyte TimesRoman10_Character_047[] = {  3,  0,  0,  0,128,128, 64, 64, 64, 32, 32,  0,  0,  0}; /* "/" */
static const GLubyte TimesRoman10_Character_063[] = {  4,  0,  0,  0, 64,  0, 64, 64, 32,160,224,  0,  0,  0}; /* "?" */
static const GLubyte TimesRoman10_Character_092[] = {  3,  0,  0,  0, 32, 32, 64, 64, 64,128,128,  0,  0,  0}; /* "\" */
static const GLubyte TimesRoman10_Character_034[] = {  4,  0,  0,  0,  0,  0,  0,  0,  0,160,160,  0,  0,  0}; /* """ */

/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte TimesRoman10_Character_039[] = {  3,  0,  0,  0,  0,  0,  0,  0,  0, 64, 64,192,  0,  0}; /* "'" */
static const GLubyte TimesRoman10_Character_124[] = {  2,128,128,128,128,128,128,128,128,128,128,128,128,128}; /* "|" */
static const GLubyte TimesRoman10_Character_131[] = {  5,  0,  0,128,232,144,144,144,144,  0,  0,  0,  0,  0}; /* "u" */


/* The font characters mapping: */
static const GLubyte* TimesRoman10_Character_Map[] = {TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_032,TimesRoman10_Character_033,TimesRoman10_Character_034,TimesRoman10_Character_035,
TimesRoman10_Character_036,TimesRoman10_Character_037,TimesRoman10_Character_038,TimesRoman10_Character_039,TimesRoman10_Character_040,TimesRoman10_Character_041,TimesRoman10_Character_042,TimesRoman10_Character_043,TimesRoman10_Character_044,TimesRoman10_Character_045,TimesRoman10_Character_046,TimesRoman10_Character_047,TimesRoman10_Character_048,TimesRoman10_Character_049,TimesRoman10_Character_050,TimesRoman10_Character_051,TimesRoman10_Character_052,TimesRoman10_Character_053,TimesRoman10_Character_054,TimesRoman10_Character_055,TimesRoman10_Character_056,TimesRoman10_Character_057,TimesRoman10_Character_058,TimesRoman10_Character_059,TimesRoman10_Character_060,TimesRoman10_Character_061,TimesRoman10_Character_062,TimesRoman10_Character_063,TimesRoman10_Character_064,TimesRoman10_Character_065,TimesRoman10_Character_066,TimesRoman10_Character_067,TimesRoman10_Character_068,TimesRoman10_Character_069,TimesRoman10_Character_070,TimesRoman10_Character_071,TimesRoman10_Character_072,
TimesRoman10_Character_073,TimesRoman10_Character_074,TimesRoman10_Character_075,TimesRoman10_Character_076,TimesRoman10_Character_077,TimesRoman10_Character_078,TimesRoman10_Character_079,TimesRoman10_Character_080,TimesRoman10_Character_081,TimesRoman10_Character_082,TimesRoman10_Character_083,TimesRoman10_Character_084,TimesRoman10_Character_085,TimesRoman10_Character_086,TimesRoman10_Character_087,TimesRoman10_Character_088,TimesRoman10_Character_089,TimesRoman10_Character_090,TimesRoman10_Character_091,TimesRoman10_Character_092,TimesRoman10_Character_093,TimesRoman10_Character_094,TimesRoman10_Character_095,TimesRoman10_Character_096,TimesRoman10_Character_097,TimesRoman10_Character_098,TimesRoman10_Character_099,TimesRoman10_Character_100,TimesRoman10_Character_101,TimesRoman10_Character_102,TimesRoman10_Character_103,TimesRoman10_Character_104,TimesRoman10_Character_105,TimesRoman10_Character_106,TimesRoman10_Character_107,TimesRoman10_Character_108,TimesRoman10_Character_109,
TimesRoman10_Character_110,TimesRoman10_Character_111,TimesRoman10_Character_112,TimesRoman10_Character_113,TimesRoman10_Character_114,TimesRoman10_Character_115,TimesRoman10_Character_116,TimesRoman10_Character_117,TimesRoman10_Character_118,TimesRoman10_Character_119,TimesRoman10_Character_120,TimesRoman10_Character_121,TimesRoman10_Character_122,TimesRoman10_Character_123,TimesRoman10_Character_124,TimesRoman10_Character_125,TimesRoman10_Character_126,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_131,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,
TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,
TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,
TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,TimesRoman10_Character_042,NULL};

/* The font structure: */
const BitmapFontData FontTimesRoman10 = { "-adobe-times-medium-r-normal--10-100-75-75-p-54-iso8859-1", 93, 13, TimesRoman10_Character_Map, 0.0f, 3.0f };

static const GLubyte TimesRoman24_Character_032[] = {  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* " " */
static const GLubyte TimesRoman24_Character_097[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,113,128,251,  0,199,  0,195,  0,195,  0, 99,  0, 59,  0, 15,  0,  3,  0, 99,  0,103,  0, 62,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte TimesRoman24_Character_098[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 94,  0,115,128, 97,128, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 97,128,115,128,110,  0, 96,  0, 96,  0, 96,  0, 96,  0,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "b" */
static const GLubyte TimesRoman24_Character_099[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 30,  0,127,  0,112,128,224,  0,192,  0,192,  0,192,  0,192,  0,192,  0, 65,128, 99,128, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "c" */
static const GLubyte TimesRoman24_Character_100[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 30,192,115,128, 97,128,193,128,193,128,193,128,193,128,193,128,193,128, 97,128,115,128, 29,128,  1,128,  1,128,  1,128,  1,128,  3,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "d" */
static const GLubyte TimesRoman24_Character_101[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 30,  0,127,  0,112,128,224,  0,192,  0,192,  0,192,  0,255,128,193,128, 65,128, 99,  0, 30,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "e" */
static const GLubyte TimesRoman24_Character_102[] = {  7,  0,  0,  0,  0,  0,  0,120, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,254, 48, 48, 48, 22, 14,  0,  0,  0,  0,  0}; /* "f" */
static const GLubyte TimesRoman24_Character_103[] = { 12,  0,  0, 63,  0,241,192,192, 96,192, 32, 96, 96, 63,192,127,  0, 96,  0, 48,  0, 62,  0, 51,  0, 97,128, 97,128, 97,128, 97,128, 51,  0, 31,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "g" */
static const GLubyte TimesRoman24_Character_104[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,241,224, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192,113,192,111,128,103,  0, 96,  0, 96,  0, 96,  0, 96,  0,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "h" */
static const GLubyte TimesRoman24_Character_105[] = {  6,  0,  0,  0,  0,  0,  0,240, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,224,  0,  0,  0, 96, 96,  0,  0,  0,  0,  0}; /* "i" */
static const GLubyte TimesRoman24_Character_106[] = {  6,  0,192,224, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,112,  0,  0,  0, 48, 48,  0,  0,  0,  0,  0}; /* "j" */
static const GLubyte TimesRoman24_Character_107[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,243,224, 97,192, 99,128,103,  0,110,  0,108,  0,120,  0,104,  0,100,  0,102,  0, 99,  0,103,192, 96,  0, 96,  0, 96,  0, 96,  0,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "k" */
static const GLubyte TimesRoman24_Character_108[] = {  6,  0,  0,  0,  0,  0,  0,240, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,224,  0,  0,  0,  0,  0}; /* "l" */
static const GLubyte TimesRoman24_Character_109[] = { 20,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,241,227,192, 96,193,128, 96,193,128, 96,193,128, 96,193,128, 96,193,128, 96,193,128, 96,193,128, 96,193,128,113,227,128,111,159,  0,231, 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "m" */
static const GLubyte TimesRoman24_Character_110[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,241,224, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192,113,192,111,128,231,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "n" */
static const GLubyte TimesRoman24_Character_111[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 30,  0,115,128, 97,128,192,192,192,192,192,192,192,192,192,192,192,192, 97,128,115,128, 30,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "o" */
static const GLubyte TimesRoman24_Character_112[] = { 12,  0,  0,240,  0, 96,  0, 96,  0, 96,  0, 96,  0,110,  0,115,128, 97,128, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 97,128,115,128,238,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "p" */
static const GLubyte TimesRoman24_Character_113[] = { 12,  0,  0,  3,192,  1,128,  1,128,  1,128,  1,128, 29,128,115,128, 97,128,193,128,193,128,193,128,193,128,193,128,193,128, 97,128,115,128, 29,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "q" */
static const GLubyte TimesRoman24_Character_114[] = {  8,  0,  0,  0,  0,  0,  0,240, 96, 96, 96, 96, 96, 96, 96, 96,118,110,230,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "r" */
static const GLubyte TimesRoman24_Character_115[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,248,  0,198,  0,131,  0,  3,  0,  7,  0, 30,  0,124,  0,112,  0,224,  0,194,  0,102,  0, 62,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "s" */
static const GLubyte TimesRoman24_Character_116[] = {  7,  0,  0,  0,  0,  0,  0, 28, 50, 48, 48, 48, 48, 48, 48, 48, 48, 48,254,112, 48, 16,  0,  0,  0,  0,  0,  0,  0}; /* "t" */
static const GLubyte TimesRoman24_Character_117[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 28,224, 62,192,113,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192,225,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "u" */
static const GLubyte TimesRoman24_Character_118[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0, 14,  0, 14,  0, 26,  0, 25,  0, 25,  0, 49,  0, 48,128, 48,128, 96,128, 96,192,241,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "v" */
static const GLubyte TimesRoman24_Character_119[] = { 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4, 16,  0, 14, 56,  0, 14, 56,  0, 26, 40,  0, 26,100,  0, 25,100,  0, 49,100,  0, 48,194,  0, 48,194,  0, 96,194,  0, 96,195,  0,241,231,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "w" */
static const GLubyte TimesRoman24_Character_120[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,241,224, 96,192, 33,128, 51,128, 27,  0, 14,  0, 12,  0, 26,  0, 57,  0, 49,128, 96,192,241,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "x" */
static const GLubyte TimesRoman24_Character_121[] = { 11,  0,  0,224,  0,240,  0, 24,  0,  8,  0, 12,  0,  4,  0, 14,  0, 14,  0, 26,  0, 25,  0, 25,  0, 49,  0, 48,128, 48,128, 96,128, 96,192,241,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "y" */
static const GLubyte TimesRoman24_Character_122[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,195,  0, 97,  0,112,  0, 48,  0, 56,  0, 24,  0, 28,  0, 14,  0,134,  0,195,  0,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte TimesRoman24_Character_065[] = { 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252, 31,128, 48,  6,  0, 16,  6,  0, 16, 12,  0, 24, 12,  0,  8, 12,  0, 15,248,  0, 12, 24,  0,  4, 24,  0,  4, 48,  0,  6, 48,  0,  2, 48,  0,  2, 96,  0,  1, 96,  0,  1,192,  0,  1,192,  0,  0,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "A" */
static const GLubyte TimesRoman24_Character_066[] = { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,224, 48,120, 48, 24, 48, 12, 48, 12, 48, 12, 48, 24, 48, 56, 63,224, 48, 64, 48, 48, 48, 24, 48, 24, 48, 24, 48, 48, 48,112,255,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "B" */
static const GLubyte TimesRoman24_Character_067[] = { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,224, 30, 56, 56,  8, 96,  4, 96,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0,192,  0, 96,  4, 96,  4, 56, 12, 28, 60,  7,228,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "C" */
static const GLubyte TimesRoman24_Character_068[] = { 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,192,  0, 48,112,  0, 48, 56,  0, 48, 12,  0, 48, 12,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48, 12,  0, 48, 12,  0, 48, 56,  0, 48,112,  0,255,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "D" */
static const GLubyte TimesRoman24_Character_069[] = { 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,248, 48, 24, 48,  8, 48,  8, 48,  0, 48,  0, 48, 64, 48, 64, 63,192, 48, 64, 48, 64, 48,  0, 48,  0, 48, 16, 48, 16, 48, 48,255,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "E" */
static const GLubyte TimesRoman24_Character_070[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48, 32, 48, 32, 63,224, 48, 32, 48, 32, 48,  0, 48,  0, 48, 16, 48, 16, 48, 48,255,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "F" */
static const GLubyte TimesRoman24_Character_071[] = { 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,224,  0, 30, 56,  0, 56, 28,  0, 96, 12,  0, 96, 12,  0,192, 12,  0,192, 12,  0,192, 63,  0,192,  0,  0,192,  0,  0,192,  0,  0,192,  0,  0, 96,  4,  0, 96,  4,  0, 56, 12,  0, 28, 60,  0,  7,228,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "G" */
static const GLubyte TimesRoman24_Character_072[] = { 19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252, 31,128, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 63,254,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0, 48,  6,  0,252, 31,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "H" */
static const GLubyte TimesRoman24_Character_073[] = {  8,  0,  0,  0,  0,  0,  0,252, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,252,  0,  0,  0,  0,  0}; /* "I" */
static const GLubyte TimesRoman24_Character_074[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,120,  0,204,  0,198,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0, 31,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "J" */
static const GLubyte TimesRoman24_Character_075[] = { 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252, 31,  0, 48, 14,  0, 48, 28,  0, 48, 56,  0, 48,112,  0, 48,224,  0, 49,192,  0, 51,128,  0, 63,  0,  0, 62,  0,  0, 51,  0,  0, 49,128,  0, 48,192,  0, 48, 96,  0, 48, 48,  0, 48, 24,  0,252,126,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "K" */
static const GLubyte TimesRoman24_Character_076[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,248, 48, 24, 48,  8, 48,  8, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0,252,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "L" */
static const GLubyte TimesRoman24_Character_077[] = { 22,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,248, 33,248, 32, 96, 96, 32, 96, 96, 32,208, 96, 32,208, 96, 33,136, 96, 33,136, 96, 35,  8, 96, 35,  4, 96, 38,  4, 96, 38,  2, 96, 44,  2, 96, 44,  2, 96, 56,  1, 96, 56,  1, 96, 48,  0,224,240,  0,248,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "M" */
static const GLubyte TimesRoman24_Character_078[] = { 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,248, 12,  0, 32, 28,  0, 32, 28,  0, 32, 52,  0, 32,100,  0, 32,100,  0, 32,196,  0, 33,132,  0, 33,132,  0, 35,  4,  0, 38,  4,  0, 38,  4,  0, 44,  4,  0, 56,  4,  0, 56,  4,  0, 48,  4,  0,240, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "N" */
static const GLubyte TimesRoman24_Character_079[] = { 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,224,  0, 28, 56,  0, 56, 28,  0, 96,  6,  0, 96,  6,  0,192,  3,  0,192,  3,  0,192,  3,  0,192,  3,  0,192,  3,  0,192,  3,  0,192,  3,  0, 96,  6,  0, 96,  6,  0, 56, 28,  0, 28, 56,  0,  7,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "O" */
static const GLubyte TimesRoman24_Character_080[] = { 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 48,  0, 63,192, 48,112, 48, 48, 48, 24, 48, 24, 48, 24, 48, 48, 48,112,255,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "P" */
static const GLubyte TimesRoman24_Character_081[] = { 18,  0,  0,  0,  0, 15,  0,  0, 56,  0,  0,112,  0,  0,224,  0,  1,192,  0,  7,224,  0, 28, 56,  0, 56, 28,  0, 96,  6,  0, 96,  6,  0,192,  3,  0,192,  3,  0,192,  3,  0,192,  3,  0,192,  3,  0,192,  3,  0,192,  3,  0, 96,  6,  0, 96,  6,  0, 56, 28,  0, 28, 56,  0,  7,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "Q" */
static const GLubyte TimesRoman24_Character_082[] = { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252, 30, 48, 28, 48, 56, 48,112, 48, 96, 48,192, 49,192, 51,128, 63,192, 48,112, 48, 48, 48, 56, 48, 24, 48, 56, 48, 48, 48,112,255,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "R" */
static const GLubyte TimesRoman24_Character_083[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,158,  0,241,128,192,192,128, 96,128, 96,  0, 96,  0,224,  3,192, 15,128, 30,  0,120,  0,224,  0,192, 64,192, 64,192,192, 99,192, 30, 64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "S" */
static const GLubyte TimesRoman24_Character_084[] = { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15,192,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,  3,  0,131,  4,131,  4,195, 12,255,252,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "T" */
static const GLubyte TimesRoman24_Character_085[] = { 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,224,  0, 28, 48,  0, 24,  8,  0, 48,  8,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0, 48,  4,  0,252, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "U" */
static const GLubyte TimesRoman24_Character_086[] = { 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,128,  0,  1,128,  0,  1,128,  0,  3,192,  0,  3, 64,  0,  3, 96,  0,  6, 32,  0,  6, 32,  0,  6, 48,  0, 12, 16,  0, 12, 24,  0, 24,  8,  0, 24,  8,  0, 24, 12,  0, 48,  4,  0, 48,  6,  0,252, 31,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "V" */
static const GLubyte TimesRoman24_Character_087[] = { 23,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,131,  0,  1,131,  0,  1,131,128,  3,135,128,  3, 70,128,  3, 70,192,  6, 70, 64,  6, 76, 64,  6, 76, 96, 12, 44, 96, 12, 44, 32, 24, 44, 32, 24, 24, 48, 24, 24, 16, 48, 24, 16, 48, 24, 24,252,126,126,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "W" */
static const GLubyte TimesRoman24_Character_088[] = { 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252, 15,192, 48,  3,128, 24,  7,  0,  8, 14,  0,  4, 12,  0,  6, 24,  0,  2, 56,  0,  1,112,  0,  0,224,  0,  0,192,  0,  1,192,  0,  3,160,  0,  3, 16,  0,  6,  8,  0, 14, 12,  0, 28,  6,  0,126, 15,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "X" */
static const GLubyte TimesRoman24_Character_089[] = { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,224,  1,128,  1,128,  1,128,  1,128,  1,128,  1,128,  3,192,  3, 64,  6, 96,  6, 32, 12, 48, 28, 16, 24, 24, 56,  8, 48, 12,252, 63,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "Y" */
static const GLubyte TimesRoman24_Character_090[] = { 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,248,224, 24,112,  8, 48,  8, 56,  0, 24,  0, 28,  0, 14,  0,  6,  0,  7,  0,  3,  0,  3,128,  1,192,128,192,128,224,192,112,255,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "Z" */
static const GLubyte TimesRoman24_Character_048[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 30,  0, 51,  0, 97,128, 97,128,225,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192, 97,128, 97,128, 51,  0, 30,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "0" */
static const GLubyte TimesRoman24_Character_049[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0,120,  0, 24,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "1" */
static const GLubyte TimesRoman24_Character_050[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,128,255,192, 96, 64, 48,  0, 24,  0, 12,  0,  4,  0,  6,  0,  3,  0,  3,  0,  1,128,  1,128,129,128,129,128, 67,128,127,  0, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "2" */
static const GLubyte TimesRoman24_Character_051[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,120,  0,230,  0,195,  0,  1,  0,  1,128,  1,128,  1,128,  3,128,  7,  0, 30,  0, 12,  0,  6,  0,131,  0,131,  0, 71,  0,126,  0, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "3" */
static const GLubyte TimesRoman24_Character_052[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  3,  0,  3,  0,  3,  0,255,192,255,192,195,  0, 67,  0, 99,  0, 35,  0, 51,  0, 19,  0, 27,  0, 11,  0,  7,  0,  7,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "4" */
static const GLubyte TimesRoman24_Character_053[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,126,  0,227,128,193,128,  0,192,  0,192,  0,192,  0,192,  1,192,  3,128, 15,128,126,  0,120,  0, 96,  0, 32,  0, 32,  0, 31,128, 31,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "5" */
static const GLubyte TimesRoman24_Character_054[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 30,  0,123,128, 97,128,224,192,192,192,192,192,192,192,192,192,193,128,243,128,238,  0, 96,  0,112,  0, 48,  0, 24,  0, 14,  0,  3,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "6" */
static const GLubyte TimesRoman24_Character_055[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 24,  0, 24,  0, 12,  0, 12,  0, 12,  0,  4,  0,  6,  0,  6,  0,  2,  0,  3,  0,  3,  0,  1,  0,  1,128,129,128,192,192,255,192,127,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "7" */
static const GLubyte TimesRoman24_Character_056[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 30,  0,115,128,225,128,192,192,192,192,192,192, 65,192, 97,128, 55,  0, 30,  0, 30,  0, 51,  0, 97,128, 97,128, 97,128, 51,  0, 30,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "8" */
static const GLubyte TimesRoman24_Character_057[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,240,  0, 28,  0,  6,  0,  3,  0,  3,128,  1,128, 29,128,115,192, 97,192,192,192,192,192,192,192,192,192,193,192, 97,128,119,128, 30,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "9" */
static const GLubyte TimesRoman24_Character_096[] = {  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 96,224,128,192, 96,  0,  0,  0,  0,  0}; /* "`" */
static const GLubyte TimesRoman24_Character_126[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,131,128,199,192,124, 96, 56, 32,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "~" */
static const GLubyte TimesRoman24_Character_033[] = {  8,  0,  0,  0,  0,  0,  0,192,192,  0,  0,  0,192,192,192,192,192,192,192,192,192,192,192,192,  0,  0,  0,  0,  0}; /* "!" */
static const GLubyte TimesRoman24_Character_064[] = { 22,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,240,  0, 14, 12,  0, 24,  0,  0, 48,  0,  0, 97,222,  0, 99,123,  0,198, 57,128,198, 24,128,198, 24,192,198, 24, 64,198, 12, 64,195, 12, 64,195,140, 64,225,252, 64, 96,236,192,112,  0,128, 56,  1,128, 28,  3,  0, 15, 14,  0,  3,248,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "@" */
static const GLubyte TimesRoman24_Character_035[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 34,  0, 34,  0, 34,  0, 34,  0, 34,  0,255,192,255,192, 17,  0, 17,  0, 17,  0,127,224,127,224,  8,128,  8,128,  8,128,  8,128,  8,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "#" */
static const GLubyte TimesRoman24_Character_036[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  4,  0, 63,  0,229,192,196,192,132, 96,132, 96,  4, 96,  4,224,  7,192,  7,128, 30,  0, 60,  0,116,  0,100,  0,100, 32,100, 96, 52,224, 31,128,  4,  0,  4,  0,  0,  0,  0,  0,  0,  0}; /* "$" */
static const GLubyte TimesRoman24_Character_037[] = { 19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 48, 60,  0, 24,114,  0, 12, 97,  0,  4, 96,128,  6, 96,128,  3, 48,128,  1, 25,128,  1,143,  0,120,192,  0,228, 64,  0,194, 96,  0,193, 48,  0,193, 16,  0, 97, 24,  0, 51,252,  0, 30, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "%" */
static const GLubyte TimesRoman24_Character_094[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,193,128, 65,  0, 99,  0, 34,  0, 54,  0, 20,  0, 28,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "^" */
static const GLubyte TimesRoman24_Character_038[] = { 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 60, 60,  0,127,126,  0,225,225,  0,192,192,  0,193,192,  0,193,160,  0, 99, 32,  0, 55, 16,  0, 30, 24,  0, 14, 62,  0, 15,  0,  0, 29,128,  0, 24,192,  0, 24, 64,  0, 24, 64,  0, 12,192,  0,  7,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "&" */
static const GLubyte TimesRoman24_Character_042[] = { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0, 28,  0,201,128,235,128, 28,  0,235,128,201,128, 28,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "*" */
static const GLubyte TimesRoman24_Character_040[] = {  8,  0,  4,  8, 16, 48, 32, 96, 96,192,192,192,192,192,192,192,192, 96, 96, 32, 48, 16,  8,  4,  0,  0,  0,  0,  0}; /* "(" */
static const GLubyte TimesRoman24_Character_041[] = {  8,  0,128, 64, 32, 48, 16, 24, 24, 12, 12, 12, 12, 12, 12, 12, 12, 24, 24, 16, 48, 32, 64,128,  0,  0,  0,  0,  0}; /* ")" */
static const GLubyte TimesRoman24_Character_045[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,240,255,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte TimesRoman24_Character_095[] = { 13,  0,  0,255,248,255,248,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte TimesRoman24_Character_061[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,240,255,240,  0,  0,  0,  0,255,240,255,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte TimesRoman24_Character_043[] = { 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,255,240,255,240,  6,  0,  6,  0,  6,  0,  6,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "+" */
static const GLubyte TimesRoman24_Character_091[] = {  8,  0,  0,248,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,248,  0,  0,  0,  0,  0}; /* "[" */
static const GLubyte TimesRoman24_Character_123[] = { 10,  0,  0,  7,  0, 12,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 16,  0, 48,  0, 32,  0,192,  0, 32,  0, 48,  0, 16,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 12,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "{" */
static const GLubyte TimesRoman24_Character_125[] = { 10,  0,  0,224,  0, 48,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0,  8,  0, 12,  0,  4,  0,  3,  0,  4,  0, 12,  0,  8,  0, 24,  0, 24,  0, 24,  0, 24,  0, 24,  0, 48,  0,224,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "}" */
static const GLubyte TimesRoman24_Character_093[] = {  8,  0,  0,248, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,248,  0,  0,  0,  0,  0}; /* "]" */
static const GLubyte TimesRoman24_Character_059[] = {  7,  0,  0,  0,192, 96, 32,224,192,  0,  0,  0,  0,  0,  0,  0,192,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* ";" */
static const GLubyte TimesRoman24_Character_058[] = {  6,  0,  0,  0,  0,  0,  0,192,192,  0,  0,  0,  0,  0,  0,  0,192,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* ":" */
static const GLubyte TimesRoman24_Character_044[] = {  7,  0,  0,  0,192, 96, 32,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte TimesRoman24_Character_046[] = {  6,  0,  0,  0,  0,  0,  0,192,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte TimesRoman24_Character_060[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 96,  1,192,  7,  0, 28,  0,112,  0,192,  0,112,  0, 28,  0,  7,  0,  1,192,  0, 96,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "<" */
static const GLubyte TimesRoman24_Character_062[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,  0,112,  0, 28,  0,  7,  0,  1,192,  0, 96,  1,192,  7,  0, 28,  0,112,  0,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* ">" */
static const GLubyte TimesRoman24_Character_047[] = {  7,  0,  0,  0,192,192,192, 96, 96, 32, 48, 48, 16, 24, 24,  8, 12, 12,  4,  6,  6,  3,  3,  3,  0,  0,  0,  0,  0}; /* "/" */
static const GLubyte TimesRoman24_Character_063[] = { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 48,  0, 48,  0,  0,  0,  0,  0, 16,  0, 16,  0, 16,  0, 24,  0, 24,  0, 12,  0, 14,  0,  7,  0,195,  0,195,  0,131,  0,198,  0,124,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "?" */
static const GLubyte TimesRoman24_Character_092[] = {  7,  0,  0,  0,  0,  0,  0,  6,  6,  4, 12, 12,  8, 24, 24, 16, 48, 48, 32, 96, 96, 64,192,192,  0,  0,  0,  0,  0}; /* "\" */
static const GLubyte TimesRoman24_Character_034[] = { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,136,  0,204,  0,204,  0,204,  0,204,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* """ */

/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte TimesRoman24_Character_039[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192, 96, 32,224,192,  0,  0,  0,  0,  0}; /* "'" */
static const GLubyte TimesRoman24_Character_124[] = {  6, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,  0,  0}; /* "|" */
static const GLubyte TimesRoman24_Character_131[] = { 13,  0,  0,  0,  0,  0,  0,  0,  0, 96,  0, 96,  0,124,224, 126,192,113,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192, 96,192,225,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "u" */


/* The font characters mapping: */
static const GLubyte* TimesRoman24_Character_Map[] = {TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_032,TimesRoman24_Character_033,TimesRoman24_Character_034,TimesRoman24_Character_035,
TimesRoman24_Character_036,TimesRoman24_Character_037,TimesRoman24_Character_038,TimesRoman24_Character_039,TimesRoman24_Character_040,TimesRoman24_Character_041,TimesRoman24_Character_042,TimesRoman24_Character_043,TimesRoman24_Character_044,TimesRoman24_Character_045,TimesRoman24_Character_046,TimesRoman24_Character_047,TimesRoman24_Character_048,TimesRoman24_Character_049,TimesRoman24_Character_050,TimesRoman24_Character_051,TimesRoman24_Character_052,TimesRoman24_Character_053,TimesRoman24_Character_054,TimesRoman24_Character_055,TimesRoman24_Character_056,TimesRoman24_Character_057,TimesRoman24_Character_058,TimesRoman24_Character_059,TimesRoman24_Character_060,TimesRoman24_Character_061,TimesRoman24_Character_062,TimesRoman24_Character_063,TimesRoman24_Character_064,TimesRoman24_Character_065,TimesRoman24_Character_066,TimesRoman24_Character_067,TimesRoman24_Character_068,TimesRoman24_Character_069,TimesRoman24_Character_070,TimesRoman24_Character_071,TimesRoman24_Character_072,
TimesRoman24_Character_073,TimesRoman24_Character_074,TimesRoman24_Character_075,TimesRoman24_Character_076,TimesRoman24_Character_077,TimesRoman24_Character_078,TimesRoman24_Character_079,TimesRoman24_Character_080,TimesRoman24_Character_081,TimesRoman24_Character_082,TimesRoman24_Character_083,TimesRoman24_Character_084,TimesRoman24_Character_085,TimesRoman24_Character_086,TimesRoman24_Character_087,TimesRoman24_Character_088,TimesRoman24_Character_089,TimesRoman24_Character_090,TimesRoman24_Character_091,TimesRoman24_Character_092,TimesRoman24_Character_093,TimesRoman24_Character_094,TimesRoman24_Character_095,TimesRoman24_Character_096,TimesRoman24_Character_097,TimesRoman24_Character_098,TimesRoman24_Character_099,TimesRoman24_Character_100,TimesRoman24_Character_101,TimesRoman24_Character_102,TimesRoman24_Character_103,TimesRoman24_Character_104,TimesRoman24_Character_105,TimesRoman24_Character_106,TimesRoman24_Character_107,TimesRoman24_Character_108,TimesRoman24_Character_109,
TimesRoman24_Character_110,TimesRoman24_Character_111,TimesRoman24_Character_112,TimesRoman24_Character_113,TimesRoman24_Character_114,TimesRoman24_Character_115,TimesRoman24_Character_116,TimesRoman24_Character_117,TimesRoman24_Character_118,TimesRoman24_Character_119,TimesRoman24_Character_120,TimesRoman24_Character_121,TimesRoman24_Character_122,TimesRoman24_Character_123,TimesRoman24_Character_124,TimesRoman24_Character_125,TimesRoman24_Character_126,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_131,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,
TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,
TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,
TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,TimesRoman24_Character_042,NULL};

/* The font structure: */
const BitmapFontData FontTimesRoman24 = { "-adobe-times-medium-r-normal--24-240-75-75-p-124-iso8859-1", 93, 28, TimesRoman24_Character_Map, -1.0f, 6.0f };

enum BitmapFontType
{
    BITMAP_FONT_TYPE_8_BY_13,
    BITMAP_FONT_TYPE_9_BY_15,
    BITMAP_FONT_TYPE_HELVETICA_10,
    BITMAP_FONT_TYPE_HELVETICA_12,
    BITMAP_FONT_TYPE_HELVETICA_18,
    BITMAP_FONT_TYPE_TIMES_ROMAN_10,
    BITMAP_FONT_TYPE_TIMES_ROMAN_24
};

//-----------------------------------------------------------------------------
// Name: getBitmapFontDataByType()
// Desc: Matches a BitmapFontType with a BitmapFontData structure pointer.
//-----------------------------------------------------------------------------
const BitmapFontData* getBitmapFontDataByType( BitmapFontType font );

//-----------------------------------------------------------------------------
// Name: beginRenderText()
// Desc: Utility function for using the bitmap-based character fonts defined 
//       above. Call this function to begin rendering text. Call the function 
//       endRenderText to stop.
//-----------------------------------------------------------------------------
void beginRenderText( int nWindowWidth, int nWindowHeight, bool blend=false);

//-----------------------------------------------------------------------------
// Name: endRenderText()
// Desc: Utility function for using the bitmap-based character fonts defined 
//       above. Call this function to stop rendering text. The call to 
//       beginRenderText should come first and be paired with this function.
//-----------------------------------------------------------------------------
void endRenderText( void );

//-----------------------------------------------------------------------------
// Name: renderText()
// Desc: Utility function for using the bitmap-based character fonts defined 
//       above. This function must be called in between a call to 
//       beginRenderText and endRenderText. See the example below:
//
//       beginRenderText( nWindowWidth, nWindowHeight );
//       {
//           renderText( 5, 10, "This is a test!", BITMAP_FONT_TYPE_HELVETICA_12 );
//       }
//       endRenderText();
//
//-----------------------------------------------------------------------------
void renderText( float x, float y, BitmapFontType fontType, const char *string );

double renderTextLen(BitmapFontType fontType, const char *string );

#endif // _BITMAP_FONTS_H_
