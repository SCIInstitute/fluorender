/*
  Teem: Tools to process and visualize scientific data and images              
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
#include "privateNrrd.h"

int
_nrrdEncodingRaw_available(void) {

  return AIR_TRUE;
}

int
_nrrdEncodingRaw_read(FILE *file, void *data, size_t elementNum,
                      Nrrd *nrrd, NrrdIoState *nio) {
  char me[]="_nrrdEncodingRaw_read", err[BIFF_STRLEN];
  size_t ret, bsize;
  int fd, dio, car;
  long savePos;

  bsize = nrrdElementSize(nrrd)*elementNum;
  if (nio->format->usesDIO) {
    fd = fileno(file);
    dio = airDioTest(fd, data, bsize);
  } else {
    fd = -1;
    dio = airNoDio_format;
  }
  if (airNoDio_okay == dio) {
    if (2 <= nrrdStateVerboseIO) {
      fprintf(stderr, "with direct I/O ... ");
    }
    ret = airDioRead(fd, data, bsize);
    if (ret != bsize) {
      sprintf(err, "%s: airDioRead got read only "
              _AIR_SIZE_T_CNV " of " _AIR_SIZE_T_CNV " bytes "
              "(%g%% of expected)", me,
              ret, bsize, 100.0*ret/bsize);
      biffAdd(NRRD, err); return 1;
    }
  } else {
    if (2 <= nrrdStateVerboseIO) {
      if (AIR_DIO && nio->format->usesDIO) {
        fprintf(stderr, "with fread(), not DIO: %s ...", airNoDioErr(dio));
      }
    }
    ret = fread(data, nrrdElementSize(nrrd), elementNum, file);
    if (ret != elementNum) {
      sprintf(err, "%s: fread got read only "
              _AIR_SIZE_T_CNV " " _AIR_SIZE_T_CNV "-sized things, not "
              _AIR_SIZE_T_CNV " (%g%% of expected)", me,
              ret, nrrdElementSize(nrrd), elementNum,
              100.0*ret/elementNum);
      biffAdd(NRRD, err); return 1;
    }
    car = fgetc(file);
    if (1 <= nrrdStateVerboseIO && EOF != car) {
      fprintf(stderr, "%s: WARNING: finished reading raw data, "
              "but file not at EOF\n", me);
      ungetc(car, file);
    }
    if (2 <= nrrdStateVerboseIO && nio->byteSkip && stdin != file) {
      savePos = ftell(file);
      if (!fseek(file, 0, SEEK_END)) {
        fprintf(stderr, "(%s: used %g%% of file for nrrd data)\n",
                me, 100.0*bsize/(ftell(file) + 1));
        fseek(file, savePos, SEEK_SET);
      }
    }
  }

  return 0;
}

int
_nrrdEncodingRaw_write(FILE *file, const void *data, size_t elementNum,
                       const Nrrd *nrrd, NrrdIoState *nio) {
  char me[]="_nrrdEncodingRaw_write", err[BIFF_STRLEN];
  int fd, dio;
  size_t ret, bsize;
  
  bsize = nrrdElementSize(nrrd)*elementNum;
  if (nio->format->usesDIO) {
    fd = fileno(file);
    dio = airDioTest(fd, data, bsize);
  } else {
    fd = -1;
    dio = airNoDio_format;
  }
  if (airNoDio_okay == dio) {
    if (2 <= nrrdStateVerboseIO) {
      fprintf(stderr, "with direct I/O ... ");
    }
    ret = airDioWrite(fd, data, bsize);
    if (ret != bsize) {
      sprintf(err, "%s: airDioWrite wrote only "
              _AIR_SIZE_T_CNV " of " _AIR_SIZE_T_CNV " bytes "
              "(%g%% of expected)", me,
              ret, bsize, 100.0*ret/bsize);
      biffAdd(NRRD, err); return 1;
    }
  } else {
    if (2 <= nrrdStateVerboseIO) {
      if (AIR_DIO && nio->format->usesDIO) {
        fprintf(stderr, "with fread(), not DIO: %s ...", airNoDioErr(dio));
      }
    }
    ret = fwrite(data, nrrdElementSize(nrrd), elementNum, file);
    if (ret != elementNum) {
      sprintf(err, "%s: fwrite wrote read only "
              _AIR_SIZE_T_CNV " " _AIR_SIZE_T_CNV "-sized things, not " 
              _AIR_SIZE_T_CNV " (%g%% of expected)", me,
              ret, nrrdElementSize(nrrd), elementNum,
              100.0*ret/elementNum);
      biffAdd(NRRD, err); return 1;
    }
    fflush(file);
    /*
    if (ferror(file)) {
      sprintf(err, "%s: ferror returned non-zero", me);
      biffAdd(NRRD, err); return 1;
    }
    */
  }
  return 0;
}

const NrrdEncoding
_nrrdEncodingRaw = {
  "raw",      /* name */
  "raw",      /* suffix */
  AIR_TRUE,   /* endianMatters */
  AIR_FALSE,  /* isCompression */
  _nrrdEncodingRaw_available,
  _nrrdEncodingRaw_read,
  _nrrdEncodingRaw_write
};

const NrrdEncoding *const
nrrdEncodingRaw = &_nrrdEncodingRaw;
