/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#ifndef _BASE_READER_H_
#define _BASE_READER_H_

#include <string>
#include <nrrd.h>
#include <vector>

using namespace std;

#ifdef STATIC_COMPILE
	#define nrrdWrap nrrdWrap_va
	#define nrrdAxisInfoSet nrrdAxisInfoSet_va
#endif

class BaseReader
{
public:
	//BaseReader();
	virtual ~BaseReader() {};

	virtual void SetFile(string &file) = 0;	//set the file name
	virtual void SetFile(wstring &file) = 0;//set the file name in wide string
	virtual void SetSliceSeq(bool ss) = 0;	//slices are stored as a file sequence
	virtual bool GetSliceSeq() = 0;			//get slice sequence
	virtual void SetTimeId(wstring &id) = 0;	//time sequence identifier
	virtual wstring GetTimeId() = 0;			//get time id
	virtual void Preprocess() = 0;			//preprocess
	virtual void SetBatch(bool batch) = 0;	//set batch mode
	virtual bool GetBatch() = 0;			//get batch mode
	virtual int LoadBatch(int index) = 0;	//load file for 3D batch mode
	virtual int LoadOffset(int offset);	//load offset index for 3D batch
	virtual int GetOffset();	//load offset index for 3D batch
	virtual Nrrd* Convert(bool get_max);			//Convert the data to nrrd
	virtual Nrrd* Convert(int c, bool get_max);		//convert the specified channel to nrrd
	virtual Nrrd* Convert(int t, int c, bool get_max) = 0;//convert the specified channel and time point to nrrd
	virtual wstring GetCurName(int t, int c) = 0;//for a 4d sequence, get the file name for specified time and channel

	virtual wstring GetPathName() = 0;
	virtual wstring GetDataName() = 0;
	virtual int GetTimeNum() = 0;
	virtual int GetCurTime() = 0;
	virtual int GetChanNum() = 0;
	virtual double GetExcitationWavelength(int chan) = 0;
	virtual int GetSliceNum() = 0;
	virtual int GetXSize() = 0;
	virtual int GetYSize() = 0;
	virtual bool IsSpcInfoValid() = 0;
	virtual double GetXSpc() = 0;
	virtual double GetYSpc() = 0;
	virtual double GetZSpc() = 0;
	virtual double GetMaxValue() = 0;
	virtual double GetScalarScale() = 0;
	virtual int GetBatchNum() = 0;
	virtual int GetCurBatch() = 0;

	bool operator==(BaseReader& reader)
	{
		return m_id_string == reader.m_id_string;
	}
	bool Match(wstring &id_string)
	{
		return m_id_string == id_string;
	}

	//resizing
	void SetResize(int type)
	{
		m_resize_type = type;
	}
	int GetResize()
	{
		return m_resize_type;
	}
	void SetResample(int type)
	{
		m_resample_type = type;
	}
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

protected:
	wstring m_id_string;	//the path and file name used to read files
	//resizing
	int m_resize_type;		//0: no resizing; 1: padding; 2: resampling
	int m_resample_type;	//0: nearest neighbour; 1: linear
	int m_alignment;		//padding alignment

	//3d batch
	bool m_batch;
	vector<wstring> m_batch_list;
	int m_cur_batch;
	
	wstring m_path_name;

	//all the decoding stuff
	#define MAXCODE(n)	((1L<<(n))-1)
	#define	BITS_MIN	9		/* start with 9 bits */
	#define	BITS_MAX	12		/* max of 12 bit strings */
	#define	CODE_CLEAR	256		/* code to clear string table */
	#define	CODE_EOI	257		/* end-of-information code */
	#define CODE_FIRST	258		/* first free code entry */
	#define	CSIZE		(MAXCODE(BITS_MAX)+1024L)
	typedef	unsigned short uint16;	/* sizeof (uint16) must == 2 */
	typedef	int int32;
	typedef	int32 tsize_t;		/* i/o size in bytes */
	typedef	unsigned char tidataval_t;	/* internal image data value type */
	typedef	tidataval_t* tidata_t;		/* reference to internal image data */
	typedef	uint16 tsample_t;			/* sample number */
	typedef uint16 hcode_t;			/* codes fit in 16 bits */
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
};

#endif//_BASE_READER_H_