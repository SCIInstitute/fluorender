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

#include "air.h"

/*
******** airMyEndian()
**
** determine at run-time if we are little (1234) or big (4321) endian
*/
int
airMyEndian(void) {
  int tmpI, ret;
  char leastbyte;

  /* set int to 1:
     least signficant byte will be 1,
     most signficant byte will be 0 */
  tmpI = 1;
  /* cast address of (4-byte) int to char*, and dereference,
     which retrieves the byte at the low-address-end of int
     (the "first" byte in memory ordering).
     On big endian, we're getting the most significant byte (0);
     on little endian, we're getting least significant byte (1) */
  leastbyte = *(AIR_CAST(char*, &tmpI));
  if (leastbyte) {
    ret = airEndianLittle;
  } else {
    ret = airEndianBig;
  }
  return ret;
}

static const char *
_airEndianStr[] = {
  "(unknown endian)",
  "little",
  "big"
};

static const char *
_airEndianDesc[] = {
  "unknown endianness",
  "Intel and compatible",
  "Everyone besides Intel and compatible"
};

static const int
_airEndianVal[] = {
  airEndianUnknown,
  airEndianLittle,
  airEndianBig,
};

static const airEnum
_airEndian = {
  "endian",
  2,
  _airEndianStr, _airEndianVal,
  _airEndianDesc,
  NULL, NULL,
  AIR_FALSE
};

const airEnum *const
airEndian = &_airEndian;

