/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#ifndef _NRRD_UTILITY_H_
#define _NRRD_UTILITY_H_

#include <RawData.h>
#include <nrrd.h>

static int ToNrrdType(fluo::DataFormat fmt)
{
	switch (fmt)
	{
	case fluo::DataFormat::UInt8:    return nrrdTypeUChar;
	case fluo::DataFormat::Int8:     return nrrdTypeChar;
	case fluo::DataFormat::UInt16:   return nrrdTypeUShort;
	case fluo::DataFormat::Int16:    return nrrdTypeShort;
	case fluo::DataFormat::UInt32:   return nrrdTypeUInt;
	case fluo::DataFormat::Int32:    return nrrdTypeInt;
	case fluo::DataFormat::Float32:  return nrrdTypeFloat;
	case fluo::DataFormat::Float64:  return nrrdTypeDouble;
	default:                         return nrrdTypeUnknown;
	}
}

static fluo::DataFormat FromNrrdScalar(int nrrd_type)
{
	switch (nrrd_type)
	{
	case nrrdTypeUChar:  return fluo::DataFormat::UInt8;
	case nrrdTypeChar:   return fluo::DataFormat::Int8;
	case nrrdTypeUShort: return fluo::DataFormat::UInt16;
	case nrrdTypeShort:  return fluo::DataFormat::Int16;
	case nrrdTypeUInt:   return fluo::DataFormat::UInt32;
	case nrrdTypeInt:    return fluo::DataFormat::Int32;
	case nrrdTypeFloat:  return fluo::DataFormat::Float32;
	case nrrdTypeDouble: return fluo::DataFormat::Float64;
	default:             return fluo::DataFormat::Unknown;
	}
}

#endif//_NRRD_UTILITY_H_