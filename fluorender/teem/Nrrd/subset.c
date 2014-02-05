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

/*
******** nrrdSlice()
**
** slices a nrrd along a given axis, at a given position.
**
** This is a newer version of the procedure, which is simpler, faster,
** and requires less memory overhead than the first one.  It is based
** on the observation that any slice is a periodic square-wave pattern
** in the original data (viewed as a one- dimensional array).  The
** characteristics of that periodic pattern are how far from the
** beginning it starts (offset), the length of the "on" part (length),
** the period (period), and the number of periods (numper). 
*/
int
nrrdSlice(Nrrd *nout, const Nrrd *nin, unsigned int saxi, size_t pos) {
  char me[]="nrrdSlice", func[]="slice", err[BIFF_STRLEN];
  size_t 
    I, 
    rowLen,                  /* length of segment */
    colStep,                 /* distance between start of each segment */
    colLen,                  /* number of periods */
    szOut[NRRD_DIM_MAX];
  unsigned int ai, outdim;
  int map[NRRD_DIM_MAX];
  char *src, *dest;

  if (!(nin && nout)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (1 == nin->dim) {
    sprintf(err, "%s: can't slice a 1-D nrrd; use nrrd{I,F,D}Lookup[]", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!( saxi < nin->dim )) {
    sprintf(err, "%s: slice axis %d out of bounds (0 to %d)", 
            me, saxi, nin->dim-1);
    biffAdd(NRRD, err); return 1;
  }
  if (!( pos < nin->axis[saxi].size )) {
    sprintf(err, "%s: position " _AIR_SIZE_T_CNV 
            " out of bounds (0 to " _AIR_SIZE_T_CNV  ")", 
            me, pos, nin->axis[saxi].size-1);
    biffAdd(NRRD, err); return 1;
  }
  /* this shouldn't actually be necessary .. */
  if (!nrrdElementSize(nin)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }

  /* set up control variables */
  rowLen = colLen = 1;
  for (ai=0; ai<nin->dim; ai++) {
    if (ai < saxi) {
      rowLen *= nin->axis[ai].size;
    } else if (ai > saxi) {
      colLen *= nin->axis[ai].size;
    }
  }
  rowLen *= nrrdElementSize(nin);
  colStep = rowLen*nin->axis[saxi].size;

  outdim = nin->dim-1;
  for (ai=0; ai<outdim; ai++) {
    map[ai] = ai + (ai >= saxi);
    szOut[ai] = nin->axis[map[ai]].size;
  }
  nout->blockSize = nin->blockSize;
  if (nrrdMaybeAlloc_nva(nout, nin->type, outdim, szOut)) {
    sprintf(err, "%s: failed to create slice", me);
    biffAdd(NRRD, err); return 1;
  }

  /* the skinny */
  src = (char *)nin->data;
  dest = (char *)nout->data;
  src += rowLen*pos;
  for (I=0; I<colLen; I++) {
    /* HEY: replace with AIR_MEMCPY() or similar, when applicable */
    memcpy(dest, src, rowLen);
    src += colStep;
    dest += rowLen;
  }

  /* copy the peripheral information */
  if (nrrdAxisInfoCopy(nout, nin, map, NRRD_AXIS_INFO_NONE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdContentSet_va(nout, func, nin, "%d,%d", saxi, pos)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_SPACEORIGIN_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  /* translate origin if this was a spatial axis, otherwise copy */
  /* note that if there is no spatial info at all, this is all harmless */
  if (AIR_EXISTS(nin->axis[saxi].spaceDirection[0])) {
    nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                          1.0, nin->spaceOrigin,
                          pos, nin->axis[saxi].spaceDirection);
  } else {
    nrrdSpaceVecCopy(nout->spaceOrigin, nin->spaceOrigin);
  }
  return 0;
}

/*
******** nrrdCrop()
**
** select some sub-volume inside a given nrrd, producing an output
** nrrd with the same dimensions, but with equal or smaller sizes
** along each axis.
*/
int
nrrdCrop(Nrrd *nout, const Nrrd *nin, size_t *min, size_t *max) {
  char me[]="nrrdCrop", func[] = "crop", err[BIFF_STRLEN],
    buff1[NRRD_DIM_MAX*30], buff2[AIR_STRLEN_SMALL];
  unsigned int ai;
  size_t I,
    lineSize,                /* #bytes in one scanline to be copied */
    typeSize,                /* size of data type */
    cIn[NRRD_DIM_MAX],       /* coords for line start, in input */
    cOut[NRRD_DIM_MAX],      /* coords for line start, in output */
    szIn[NRRD_DIM_MAX],
    szOut[NRRD_DIM_MAX],
    idxIn, idxOut,           /* linear indices for input and output */
    numLines;                /* number of scanlines in output nrrd */
  char *dataIn, *dataOut;

  /* errors */
  if (!(nout && nin && min && max)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    if (!(min[ai] <= max[ai])) {
      sprintf(err, "%s: axis %d min (" _AIR_SIZE_T_CNV 
              ") not <= max (" _AIR_SIZE_T_CNV ")", 
              me, ai, min[ai], max[ai]);
      biffAdd(NRRD, err); return 1;
    }
    if (!( min[ai] < nin->axis[ai].size && max[ai] < nin->axis[ai].size )) {
      sprintf(err, "%s: axis %d min (" _AIR_SIZE_T_CNV  
              ") or max (" _AIR_SIZE_T_CNV  ") out of bounds [0," 
              _AIR_SIZE_T_CNV  "]",
              me, ai, min[ai], max[ai], nin->axis[ai].size-1);
      biffAdd(NRRD, err); return 1;
    }
  }
  /* this shouldn't actually be necessary .. */
  if (!nrrdElementSize(nin)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }

  /* allocate */
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  numLines = 1;
  for (ai=0; ai<nin->dim; ai++) {
    szOut[ai] = max[ai] - min[ai] + 1;
    if (ai) {
      numLines *= szOut[ai];
    }
  }
  nout->blockSize = nin->blockSize;
  if (nrrdMaybeAlloc_nva(nout, nin->type, nin->dim, szOut)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  lineSize = szOut[0]*nrrdElementSize(nin);
  
  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = (char *)nin->data;
  dataOut = (char *)nout->data;
  memset(cOut, 0, NRRD_DIM_MAX*sizeof(unsigned int));
  /*
  printf("!%s: nin->dim = %d\n", me, nin->dim);
  printf("!%s: min  = %d %d %d\n", me, min[0], min[1], min[2]);
  printf("!%s: szIn = %d %d %d\n", me, szIn[0], szIn[1], szIn[2]);
  printf("!%s: szOut = %d %d %d\n", me, szOut[0], szOut[1], szOut[2]);
  printf("!%s: lineSize = %d\n", me, lineSize);
  printf("!%s: typeSize = %d\n", me, typeSize);
  printf("!%s: numLines = %d\n", me, (int)numLines);
  */
  for (I=0; I<numLines; I++) {
    for (ai=0; ai<nin->dim; ai++) {
      cIn[ai] = cOut[ai] + min[ai];
    }
    NRRD_INDEX_GEN(idxOut, cOut, szOut, nin->dim);
    NRRD_INDEX_GEN(idxIn, cIn, szIn, nin->dim);
    /*
    printf("!%s: %5d: cOut=(%3d,%3d,%3d) --> idxOut = %5d\n",
           me, (int)I, cOut[0], cOut[1], cOut[2], (int)idxOut);
    printf("!%s: %5d:  cIn=(%3d,%3d,%3d) -->  idxIn = %5d\n",
           me, (int)I, cIn[0], cIn[1], cIn[2], (int)idxIn);
    */
    memcpy(dataOut + idxOut*typeSize, dataIn + idxIn*typeSize, lineSize);
    /* the lowest coordinate in cOut[] will stay zero, since we are 
       copying one (1-D) scanline at a time */
    NRRD_COORD_INCR(cOut, szOut, nin->dim, 1);
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL, (NRRD_AXIS_INFO_SIZE_BIT |
                                         NRRD_AXIS_INFO_MIN_BIT |
                                         NRRD_AXIS_INFO_MAX_BIT ))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    nrrdAxisInfoPosRange(&(nout->axis[ai].min), &(nout->axis[ai].max),
                         nin, ai, min[ai], max[ai]);
    /* do the safe thing first */
    nout->axis[ai].kind = _nrrdKindAltered(nin->axis[ai].kind, AIR_FALSE);
    /* try cleverness */
    if (!nrrdStateKindNoop) {
      if (nout->axis[ai].size == nin->axis[ai].size) {
        /* we can safely copy kind; the samples didn't change */
        nout->axis[ai].kind = nin->axis[ai].kind;
      } else if (nrrdKind4Color == nin->axis[ai].kind
                 && 3 == szOut[ai]) {
        nout->axis[ai].kind = nrrdKind3Color;
      } else if (nrrdKind4Vector == nin->axis[ai].kind
                 && 3 == szOut[ai]) {
        nout->axis[ai].kind = nrrdKind3Vector;
      } else if ((nrrdKind4Vector == nin->axis[ai].kind
                  || nrrdKind3Vector == nin->axis[ai].kind)
                 && 2 == szOut[ai]) {
        nout->axis[ai].kind = nrrdKind2Vector;
      } else if (nrrdKindRGBAColor == nin->axis[ai].kind
                 && 0 == min[ai]
                 && 2 == max[ai]) {
        nout->axis[ai].kind = nrrdKindRGBColor;
      } else if (nrrdKind2DMaskedSymMatrix == nin->axis[ai].kind
                 && 1 == min[ai]
                 && max[ai] == szIn[ai]-1) {
        nout->axis[ai].kind = nrrdKind2DSymMatrix;
      } else if (nrrdKind2DMaskedMatrix == nin->axis[ai].kind
                 && 1 == min[ai]
                 && max[ai] == szIn[ai]-1) {
        nout->axis[ai].kind = nrrdKind2DMatrix;
      } else if (nrrdKind3DMaskedSymMatrix == nin->axis[ai].kind
                 && 1 == min[ai]
                 && max[ai] == szIn[ai]-1) {
        nout->axis[ai].kind = nrrdKind3DSymMatrix;
      } else if (nrrdKind3DMaskedMatrix == nin->axis[ai].kind
                 && 1 == min[ai]
                 && max[ai] == szIn[ai]-1) {
        nout->axis[ai].kind = nrrdKind3DMatrix;
      }
    }
  }
  strcpy(buff1, "");
  for (ai=0; ai<nin->dim; ai++) {
    sprintf(buff2, "%s[" _AIR_SIZE_T_CNV  "," _AIR_SIZE_T_CNV  "]",
            (ai ? "x" : ""), min[ai], max[ai]);
    strcat(buff1, buff2);
  }
  if (nrrdContentSet_va(nout, func, nin, "%s", buff1)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_SPACEORIGIN_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  /* copy origin, then shift it along the spatial axes */
  nrrdSpaceVecCopy(nout->spaceOrigin, nin->spaceOrigin);
  for (ai=0; ai<nin->dim; ai++) {
    if (AIR_EXISTS(nin->axis[ai].spaceDirection[0])) {
      nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                            1.0, nout->spaceOrigin,
                            min[ai], nin->axis[ai].spaceDirection);
    }
  }
                         

  return 0;
}

/* ---- BEGIN non-NrrdIO */

/*
******** nrrdSample_nva()
**
** given coordinates within a nrrd, copies the 
** single element into given *val
*/
int
nrrdSample_nva(void *val, const Nrrd *nrrd, const size_t *coord) {
  char me[]="nrrdSample_nva", err[BIFF_STRLEN];
  size_t I, size[NRRD_DIM_MAX], typeSize;
  unsigned int ai;
  
  if (!(nrrd && coord && val)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  /* this shouldn't actually be necessary .. */
  if (!nrrdElementSize(nrrd)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }
  
  typeSize = nrrdElementSize(nrrd);
  nrrdAxisInfoGet_nva(nrrd, nrrdAxisInfoSize, size);
  for (ai=0; ai<nrrd->dim; ai++) {
    if (!( coord[ai] < size[ai] )) {
      sprintf(err, "%s: coordinate " _AIR_SIZE_T_CNV 
              " on axis %d out of bounds (0 to " _AIR_SIZE_T_CNV  ")", 
              me, coord[ai], ai, size[ai]-1);
      biffAdd(NRRD, err); return 1;
    }
  }

  NRRD_INDEX_GEN(I, coord, size, nrrd->dim);

  memcpy(val, (char*)(nrrd->data) + I*typeSize, typeSize);
  return 0;
}

/*
******** nrrdSample_va()
**
** var-args version of nrrdSample_nva()
*/
int
nrrdSample_va(void *val, const Nrrd *nrrd, ...) {
  char me[]="nrrdSample_va", err[BIFF_STRLEN];
  unsigned int ai;
  size_t coord[NRRD_DIM_MAX];
  va_list ap;
  
  if (!(nrrd && val)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }

  va_start(ap, nrrd);
  for (ai=0; ai<nrrd->dim; ai++) {
    coord[ai] = va_arg(ap, size_t);
  }
  va_end(ap);
  
  if (nrrdSample_nva(val, nrrd, coord)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  return 0;
}

/*
******** nrrdSimpleCrop()
**
*/
int
nrrdSimpleCrop(Nrrd *nout, const Nrrd *nin, unsigned int crop) {
  char me[]="nrrdSimpleCrop", err[BIFF_STRLEN];
  unsigned int ai;
  size_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    min[ai] = crop;
    max[ai] = nin->axis[ai].size-1 - crop;
  }
  if (nrrdCrop(nout, nin, min, max)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  return 0;
}

/* ---- END non-NrrdIO */
