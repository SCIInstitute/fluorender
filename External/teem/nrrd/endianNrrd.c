/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "nrrd.h"

static void
_nrrdSwap16Endian(void *_data, size_t N) {
  unsigned short *data, dd, fix, mask;
  size_t I;

  if (!_data) {
    return;
  }
  data = AIR_CAST(unsigned short *, _data);
  mask = AIR_CAST(unsigned short, 0x00FFu);
  for (I=0; I<N; I++) {
    dd = data[I];
    fix = (dd & mask); dd >>= 0x08;
    fix = (dd & mask) | AIR_CAST(unsigned short, fix << 0x08);
    data[I] = fix;
  }
}

static void
_nrrdSwap32Endian(void *_data, size_t N) {
  unsigned int *data, dd, fix, mask;
  size_t I;

  if (!_data) {
    return;
  }
  data = AIR_CAST(unsigned int *, _data);
  mask = 0x000000FFu;
  for (I=0; I<N; I++) {
    dd = data[I];
    fix = (dd & mask);                 dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08); dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08); dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08);
    data[I] = fix;
  }
}

static void
_nrrdSwap64Endian(void *_data, size_t N) {
  airULLong *data, dd, fix, mask;
  size_t I;

  if (!_data) {
    return;
  }
  data = AIR_CAST(airULLong *, _data);
  mask = AIR_ULLONG(0x00000000000000FF);
  for (I=0; I<N; I++) {
    dd = data[I];
    fix = (dd & mask);                 dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08); dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08); dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08); dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08); dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08); dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08); dd >>= 0x08;
    fix = (dd & mask) | (fix << 0x08);
    data[I] = fix;
  }
}

static void
_nrrdNoopEndian(void *data, size_t N) {
  AIR_UNUSED(data);
  AIR_UNUSED(N);
  return;
}

static void
_nrrdBlockEndian(void *data, size_t N) {
  char me[]="_nrrdBlockEndian";

  AIR_UNUSED(data);
  AIR_UNUSED(N);
  fprintf(stderr, "%s: WARNING: can't fix endiannes of nrrd type %s\n", me,
          airEnumStr(nrrdType, nrrdTypeBlock));
}

static void
(*_nrrdSwapEndian[])(void *, size_t) = {
  _nrrdNoopEndian,         /*  0: nobody knows! */
  _nrrdNoopEndian,         /*  1:   signed 1-byte integer */
  _nrrdNoopEndian,         /*  2: unsigned 1-byte integer */
  _nrrdSwap16Endian,       /*  3:   signed 2-byte integer */
  _nrrdSwap16Endian,       /*  4: unsigned 2-byte integer */
  _nrrdSwap32Endian,       /*  5:   signed 4-byte integer */
  _nrrdSwap32Endian,       /*  6: unsigned 4-byte integer */
  _nrrdSwap64Endian,       /*  7:   signed 8-byte integer */
  _nrrdSwap64Endian,       /*  8: unsigned 8-byte integer */
  _nrrdSwap32Endian,       /*  9:          4-byte floating point */
  _nrrdSwap64Endian,       /* 10:          8-byte floating point */
  _nrrdBlockEndian         /* 11: size user defined at run time */
};

void
nrrdSwapEndian(Nrrd *nrrd) {

  if (nrrd
      && nrrd->data
      && !airEnumValCheck(nrrdType, nrrd->type)) {
    _nrrdSwapEndian[nrrd->type](nrrd->data, nrrdElementNumber(nrrd));
  }
  return;
}
