#include "bitmap_fonts.h"

//-----------------------------------------------------------------------------
// Name: getBitmapFontDataByType()
// Desc: Matches a BitmapFontType with a BitmapFontData structure pointer.
//-----------------------------------------------------------------------------
const BitmapFontData* getBitmapFontDataByType( BitmapFontType font )
{
	if( font == BITMAP_FONT_TYPE_8_BY_13 )
		return &FontFixed8x13;
	if( font == BITMAP_FONT_TYPE_9_BY_15 )
		return &FontFixed9x15;
	if( font == BITMAP_FONT_TYPE_HELVETICA_10 )
		return &FontHelvetica10;
	if( font == BITMAP_FONT_TYPE_HELVETICA_12 )
		return &FontHelvetica12;
	if( font == BITMAP_FONT_TYPE_HELVETICA_18 )
		return &FontHelvetica18;
	if( font == BITMAP_FONT_TYPE_TIMES_ROMAN_10 )
		return &FontTimesRoman10;
	if( font == BITMAP_FONT_TYPE_TIMES_ROMAN_24 )
		return &FontTimesRoman24;

	return 0;
}

//-----------------------------------------------------------------------------
// Name: beginRenderText()
// Desc: Utility function for using the bitmap-based character fonts defined 
//       above. Call this function to begin rendering text. Call the function 
//       endRenderText to stop.
//-----------------------------------------------------------------------------
void beginRenderText( int nWindowWidth, int nWindowHeight, bool blend )
{
	// Push back and cache the current state of pixel alignment.
	glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
	glPixelStorei( GL_UNPACK_SWAP_BYTES, GL_FALSE );
	glPixelStorei( GL_UNPACK_LSB_FIRST, GL_FALSE );
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
	glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
	glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Push back and cache the current state of depth testing and lighting
	// and then disable them.
	glPushAttrib( GL_TEXTURE_BIT | GL_DEPTH_TEST | GL_LIGHTING | GL_COLOR_BUFFER_BIT);

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	if (blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	}
	else
		glDisable(GL_BLEND);

	// Push back the current matrices and go orthographic for text rendering.
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, (double)nWindowWidth, (double)nWindowHeight, 0, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
}

//-----------------------------------------------------------------------------
// Name: endRenderText()
// Desc: Utility function for using the bitmap-based character fonts defined 
//       above. Call this function to stop rendering text. The call to 
//       beginRenderText should come first and be paired with this function.
//-----------------------------------------------------------------------------
void endRenderText( void )
{
	// Pop everything back to what ever it was set to before we started 
	// rendering text to the screen.

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glPopClientAttrib();

	glPopAttrib();
}

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
void renderText( float x, float y, BitmapFontType fontType, const char *string )
{
	glRasterPos2f( x, y );

	const BitmapFontData* font = getBitmapFontDataByType( fontType );
	const unsigned char* face;
	unsigned char *str_temp = (unsigned char*) string;

	for(unsigned char * c = str_temp; *c != '\0'; ++c )
	{
		// Find the character face that we want to draw.
		face = font->Characters[*c-1];

		glBitmap( face[0], font->Height,    // The bitmap's width and height
			font->xorig, font->yorig, // The origin in the font glyph
			(float)(face[0]), 0.0,    // The raster advance -- inc. x,y
			(face+1) );               // The packed bitmap data...
	}
}

double renderTextLen(BitmapFontType fontType, const char *string )
{
	double length = 0.0;
	const BitmapFontData* font = getBitmapFontDataByType( fontType );
	const unsigned char* face;
	unsigned char *str_temp = (unsigned char*) string;

	for(unsigned char * c = str_temp; *c != '\0'; ++c )
	{
		// Find the character face that we want to draw.
		face = font->Characters[*c-1];
		length += (float)(face[0]);
	}

	return length;
}
