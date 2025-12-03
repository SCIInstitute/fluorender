/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#ifndef _BASE_VOL_READER_H_
#define _BASE_VOL_READER_H_

#include <base_reader.h>
#include <string>
#include <nrrd.h>
#include <sstream>
#include <deque>
#include <set>

namespace fluo
{
	class Vector;
}
class BaseVolReader : public BaseReader
{
public:
	BaseVolReader();
	virtual ~BaseVolReader() {};

	//set reader flag to read a 3D stack as a file sequence, each file being a slice
	//in UI, it is set in the open file dialog as "read a sequence as z slices..."
	//not all reader types use this flag, basically for a tiff sequence
	virtual void SetSliceSeq(bool ss) { m_slice_seq = ss; }
	//get the reader flag to read z stack sequence
	virtual bool GetSliceSeq() { return m_slice_seq; }
	//read channels
	virtual void SetChannSeq(bool cs) { m_chann_seq = cs; }
	virtual bool GetChannSeq() { return m_chann_seq; }
	//digit order
	virtual void SetDigitOrder(int order) { m_digit_order = order; }
	virtual int GetDigitOrder() { return m_digit_order; }

	//read the acutual volume data from file and convert it to a nrrd
	//if get_max is true, it will find the maximum value within the volume
	//the max value is used to change the silder range in UI
	virtual Nrrd* Convert(bool get_max);
	//c is the channel index to load
	virtual Nrrd* Convert(int c, bool get_max);
	//t is the time point value to load
	virtual Nrrd* Convert(int t, int c, bool get_max) = 0;

	//for a time sequence, get the file name for specified time and channel
	virtual std::wstring GetCurDataName(int t, int c) = 0;
	//for a 4d sequence, get the file name for the mask of specified time and channel
	virtual std::wstring GetCurMaskName(int t, int c) = 0;
	//for a 4d sequence, get the file name for the label of specified time and channel
	virtual std::wstring GetCurLabelName(int t, int c) = 0;

	//get the total number of channels
	virtual int GetChanNum() = 0;
	//get the excitation wave length for a channel, read from meta data
	//the wave length value can be used to set the color of a channel
	//the settings are set in the setting dialog
	//not all file formats can provide this information
	virtual double GetExcitationWavelength(int chan) = 0;
	//get the total number of z slices in a stack
	virtual fluo::Vector GetResolution() = 0;
	//sometimes the spacing info in the file is incorrect (too small or big)
	//if the flag is set to false, default spacing values will be applied after reading
	virtual bool IsSpcInfoValid() = 0;
	//get the x spacing value, the physical size of the voxel
	virtual fluo::Vector GetSpacing() = 0;
	//get the max intensity value
	//for 8-bit data, the max value is usually 255
	//for 16-bit data, the microscope sensor may not generate full range, a smaller number is usually used
	virtual double GetMinValue() = 0;
	virtual double GetMaxValue() = 0;
	//intensity values (0-255 for 8-bit, 0-max value for 16-bit) are normalized to (0.0-1.0) for OpenGL
	//the scalar-value scaling factor is usually the inverse of the max value (1/max) for 16-bit data
	virtual double GetScalarScale() = 0;

	//a volume can be resized using a sampler
	//this is usually used when the mask of a volume has a different size
	//it can also change the size of a volume when it is resaved
	//this sets the type of resizing
	void SetResize(int type)
	{
		m_resize_type = type;
	}
	//get the type of resizing
	int GetResize()
	{
		return m_resize_type;
	}
	//specify the sampler to use for resizing
	void SetResample(int type)
	{
		m_resample_type = type;
	}
	//get the current smapler's type
	int GetResample()
	{
		return m_resample_type;
	}
	void SetAlignment(int alignment)
	{
		m_alignment = alignment;
	}
	int GetAlignment()
	{
		return m_alignment;
	}

	//range for fp32
	bool GetFpConvert() { return m_fp_convert; }
	void GetFpRange(double& min_val, double& max_val) { min_val = m_fp_min; max_val = m_fp_max; }
	void SetFpRange(double min_val, double max_val) { m_fp_min = min_val; m_fp_max = max_val; }

protected:
	//resizing
	int m_resize_type;		//0: no resizing; 1: padding; 2: resampling
	int m_resample_type;	//0: nearest neighbour; 1: linear
	int m_alignment;		//padding alignment

	bool m_fp_convert;
	double m_fp_min;
	double m_fp_max;

	//sequence type
	bool m_slice_seq = false;
	bool m_chann_seq = false;
	int m_digit_order = 0;

	//name pattern stuff
	struct NamePattern
	{
		size_t start;
		size_t end;
		size_t len;//0:indefinite
		int type;//0:string; 1:digits
		int use;//0:z sections; 1:channels; 2:time
		std::wstring str;//content
	};
	std::deque<NamePattern> m_name_patterns;
	std::set<int> m_slice_count;
	std::set<int> m_chann_count;//counting total numbers in preprocessing

	//name pattern
	void AnalyzeNamePattern(const std::wstring &path_name);
	void AddPatternR(wchar_t c, size_t pos);//add backwards
	std::wstring GetSearchString(int mode, int t);
	int GetPatternNumber(std::wstring &path_name, int mode, bool count=false);

	//all the lzw decoding stuff
	#define MAXCODE(n)	((1L<<(n))-1)
	#define	BITS_MIN	9		/* start with 9 bits */
	#define	BITS_MAX	12		/* max of 12 bit strings */
	#define	CODE_CLEAR	256		/* code to clear string table */
	#define	CODE_EOI	257		/* end-of-information code */
	#define CODE_FIRST	258		/* first free code entry */
	#define	CSIZE		(MAXCODE(BITS_MAX)+1024L)
	typedef	int tsize_t;		/* i/o size in bytes */
	typedef	unsigned char tidataval_t;	/* internal image data value type */
	typedef	tidataval_t* tidata_t;		/* reference to internal image data */
	typedef	uint16_t tsample_t;			/* sample number */
	typedef uint16_t hcode_t;			/* codes fit in 16 bits */
	typedef struct code_ent
	{
		struct code_ent *next;
		unsigned short	length;		/* string len, including this token */
		unsigned char	value;		/* data value */
		unsigned char	firstchar;	/* first token of string */
	} code_t;

	typedef struct
	{
		long	hash;
		hcode_t	code;
	} hash_t;

	typedef struct
	{
		int		predictor;	/* predictor tag value */
		int		stride;		/* sample stride over data */
		tsize_t		rowsize;	/* tile/strip row size */

		unsigned short	nbits;		/* # of bits/code */
		unsigned short	maxcode;	/* maximum code for lzw_nbits */
		unsigned short	free_ent;	/* next free entry in hash table */
		long		nextdata;	/* next bits of i/o */
		long		nextbits;	/* # of valid bits in lzw_nextdata */

		/* Decoding specific data */
		long	dec_nbitsmask;		/* lzw_nbits 1 bits, right adjusted */
		long	dec_restart;		/* restart count */
		long	dec_bitsleft;		/* available bits in raw data */
		code_t*	dec_codep;		/* current recognized code */
		code_t*	dec_oldcodep;		/* previously recognized code */
		code_t*	dec_free_entp;		/* next free entry */
		code_t*	dec_maxcodep;		/* max available entry */
		code_t*	dec_codetab;		/* kept separate for small machines */
	} LZWCodecState;
	#define	NextCode(_tif, _sp, _bp, _code, _get) {				\
		if ((_sp)->dec_bitsleft < nbits) {				\
			_code = CODE_EOI;					\
		} else {							\
			_get(_sp,_bp,_code);					\
			(_sp)->dec_bitsleft -= nbits;				\
		}								\
	}
	#define	GetNextCode(sp, bp, code) {				\
		nextdata = (nextdata<<8) | *(bp)++;			\
		nextbits += 8;						\
		if (nextbits < nbits) {					\
			nextdata = (nextdata<<8) | *(bp)++;		\
			nextbits += 8;					\
		}							\
		code = (hcode_t)((nextdata >> (nextbits-nbits)) & nbitsmask);	\
		nextbits -= nbits;					\
	}
	#define REPEAT4(n, op)		\
		switch (n) {		\
		default: { int i; for (i = n-4; i > 0; i--) { op; } } \
		case 4:  op;		\
		case 3:  op;		\
		case 2:  op;		\
		case 1:  op;		\
		case 0:  ;			\
	}

	int LZWDecode(tidata_t tif, tidata_t op0, tsize_t occ0);
	void DecodeAcc8(tidata_t cp0, tsize_t cc, tsize_t stride);
	void DecodeAcc16(tidata_t cp0, tsize_t cc, tsize_t stride);

	//read number after a position in a string
	int get_number(const std::string &str, int64_t pos);
	double get_double(const std::string& str, int64_t pos);
};

#endif//_BASE_VOL_READER_H_