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
nrrdSlice(Nrrd *nout, const Nrrd *cnin, unsigned int saxi, size_t pos) {
  static const char me[]="nrrdSlice", func[]="slice";
  size_t
    I,
    rowLen,                  /* length of segment */
    colStep,                 /* distance between start of each segment */
    colLen,                  /* number of periods */
    szOut[NRRD_DIM_MAX];
  unsigned int ai, outdim;
  int map[NRRD_DIM_MAX];
  const char *src;
  char *dest, stmp[2][AIR_STRLEN_SMALL];
  airArray *mop;
  Nrrd *nin;

  if (!(cnin && nout)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == cnin) {
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  if (1 == cnin->dim) {
    if (0 != saxi) {
      biffAddf(NRRD, "%s: slice axis must be 0, not %u, for 1-D array",
               me, saxi);
      return 1;
    }
  } else {
    if (!( saxi < cnin->dim )) {
      biffAddf(NRRD, "%s: slice axis %d out of bounds (0 to %d)",
               me, saxi, cnin->dim-1);
      return 1;
    }
  }
  if (!( pos < cnin->axis[saxi].size )) {
    biffAddf(NRRD, "%s: position %s out of bounds (0 to %s)", me,
             airSprintSize_t(stmp[0], pos),
             airSprintSize_t(stmp[1], cnin->axis[saxi].size-1));
    return 1;
  }
  /* this shouldn't actually be necessary .. */
  if (!nrrdElementSize(cnin)) {
    biffAddf(NRRD, "%s: nrrd reports zero element size!", me);
    return 1;
  }

  /* HEY: copy and paste from measure.c/nrrdProject */
  mop = airMopNew();
  if (1 == cnin->dim) {
    /* There are more efficient ways of dealing with this case; this way is
       easy to implement because it leaves most of the established code below
       only superficially changed; uniformly replacing nin with (nin ? nin :
       cnin), even if pointlessly so; this expression that can't be assigned
       to a new variable because of the difference in const. */
    nin = nrrdNew();
    airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdAxesInsert(nin, cnin, 1)) {
      biffAddf(NRRD, "%s: trouble inserting axis on 1-D array", me);
      airMopError(mop); return 1;
    }
  } else {
    nin = NULL;
  }

  /* set up control variables */
  rowLen = colLen = 1;
  for (ai=0; ai<(nin ? nin : cnin)->dim; ai++) {
    if (ai < saxi) {
      rowLen *= (nin ? nin : cnin)->axis[ai].size;
    } else if (ai > saxi) {
      colLen *= (nin ? nin : cnin)->axis[ai].size;
    }
  }
  rowLen *= nrrdElementSize(nin ? nin : cnin);
  colStep = rowLen*(nin ? nin : cnin)->axis[saxi].size;

  outdim = (nin ? nin : cnin)->dim-1;
  for (ai=0; ai<outdim; ai++) {
    map[ai] = AIR_INT(ai) + (ai >= saxi);
    szOut[ai] = (nin ? nin : cnin)->axis[map[ai]].size;
  }
  nout->blockSize = (nin ? nin : cnin)->blockSize;
  if (nrrdMaybeAlloc_nva(nout, (nin ? nin : cnin)->type, outdim, szOut)) {
    biffAddf(NRRD, "%s: failed to create slice", me);
    airMopError(mop); return 1;
  }

  /* the skinny */
  src = AIR_CAST(const char *, (nin ? nin : cnin)->data);
  dest = AIR_CAST(char *, nout->data);
  src += rowLen*pos;
  for (I=0; I<colLen; I++) {
    /* HEY: replace with AIR_MEMCPY() or similar, when applicable */
    memcpy(dest, src, rowLen);
    src += colStep;
    dest += rowLen;
  }

  /* copy the peripheral information */
  if (nrrdAxisInfoCopy(nout, (nin ? nin : cnin), map, NRRD_AXIS_INFO_NONE)) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  if (nrrdContentSet_va(nout, func, cnin /* hide possible axinsert*/,
                        "%d,%d", saxi, pos)) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  if (nrrdBasicInfoCopy(nout, (nin ? nin : cnin),
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
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  /* translate origin if this was a spatial axis, otherwise copy */
  /* note that if there is no spatial info at all, this is all harmless */
  if (AIR_EXISTS((nin ? nin : cnin)->axis[saxi].spaceDirection[0])) {
    nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                          1.0, (nin ? nin : cnin)->spaceOrigin,
                          AIR_CAST(double, pos),
                          (nin ? nin : cnin)->axis[saxi].spaceDirection);
  } else {
    nrrdSpaceVecCopy(nout->spaceOrigin, (nin ? nin : cnin)->spaceOrigin);
  }
  airMopOkay(mop);
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
  static const char me[]="nrrdCrop", func[] = "crop";
  char buff1[NRRD_DIM_MAX*30], buff2[AIR_STRLEN_SMALL];
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
  char *dataIn, *dataOut, stmp[3][AIR_STRLEN_SMALL];

  /* errors */
  if (!(nout && nin && min && max)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == nin) {
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    if (!(min[ai] <= max[ai])) {
      biffAddf(NRRD, "%s: axis %d min (%s) not <= max (%s)", me, ai,
               airSprintSize_t(stmp[0], min[ai]),
               airSprintSize_t(stmp[1], max[ai]));
      return 1;
    }
    if (!( min[ai] < nin->axis[ai].size && max[ai] < nin->axis[ai].size )) {
      biffAddf(NRRD, "%s: axis %d min (%s) or max (%s) out of bounds [0,%s]",
               me, ai, airSprintSize_t(stmp[0], min[ai]),
               airSprintSize_t(stmp[1], max[ai]),
               airSprintSize_t(stmp[2], nin->axis[ai].size-1));
      return 1;
    }
  }
  /* this shouldn't actually be necessary .. */
  if (!nrrdElementSize(nin)) {
    biffAddf(NRRD, "%s: nrrd reports zero element size!", me);
    return 1;
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
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  lineSize = szOut[0]*nrrdElementSize(nin);

  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = (char *)nin->data;
  dataOut = (char *)nout->data;
  memset(cOut, 0, NRRD_DIM_MAX*sizeof(*cOut));
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
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    nrrdAxisInfoPosRange(&(nout->axis[ai].min), &(nout->axis[ai].max),
                         nin, ai, AIR_CAST(double, min[ai]),
                         AIR_CAST(double, max[ai]));
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
    sprintf(buff2, "%s[%s,%s]", (ai ? "x" : ""),
            airSprintSize_t(stmp[0], min[ai]),
            airSprintSize_t(stmp[1], max[ai]));
    strcat(buff1, buff2);
  }
  if (nrrdContentSet_va(nout, func, nin, "%s", buff1)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
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
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  /* copy origin, then shift it along the spatial axes */
  nrrdSpaceVecCopy(nout->spaceOrigin, nin->spaceOrigin);
  for (ai=0; ai<nin->dim; ai++) {
    if (AIR_EXISTS(nin->axis[ai].spaceDirection[0])) {
      nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                            1.0, nout->spaceOrigin,
                            AIR_CAST(double, min[ai]),
                            nin->axis[ai].spaceDirection);
    }
  }


  return 0;
}

/* ---- BEGIN non-NrrdIO */

/*
******** nrrdSliceSelect
**
** selects slices along axis "axi" from "nin", according to whether
** line[i] is above or below thresh:
**
** line[i] >= thresh: slice i goes into noutAbove
** line[i] < thresh:  slice i goes into noutBelow
**
** Either noutAbove or noutBelow (but not both) can be passed
** as NULL if you aren't interested in the output.  It is a
** biff-able error if the threshold is outside the range of
** errors and a non-Null nrrd was passed for the correspondingly
** empty output.
*/
int
nrrdSliceSelect(Nrrd *noutAbove, Nrrd *noutBelow, const Nrrd *nin,
                unsigned int saxi, Nrrd *_nline, double thresh) {
  static const char me[]="nrrdSliceSelect";
  airArray *mop;
  Nrrd *nline, *nslice;
  NrrdRange *rng;
  double *line;
  size_t II, LL, numAbove, numBelow, stride,
    sizeAbove[NRRD_DIM_MAX], sizeBelow[NRRD_DIM_MAX];
  unsigned int aa, bb;
  int axmap[NRRD_DIM_MAX];
  char *above, *below, stmp[2][AIR_STRLEN_SMALL];

  if (!( (noutAbove || noutBelow) && nin && _nline )) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_EXISTS(thresh)) {
    biffAddf(NRRD, "%s: thresh %g doesn't exist", me, thresh);
    return 1;
  }
  if (!(saxi < nin->dim)) {
    biffAddf(NRRD, "%s: can't select axis %u of a %u-D input nrrd", me,
             saxi, nin->dim);
    return 1;
  }
  if (nrrdCheck(nin) || nrrdCheck(_nline)) {
    biffAddf(NRRD, "%s: basic problems with nin or nline", me);
    return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    /* no good reason for this except that GLK forgets out to
       set the blocksize in output */
    biffAddf(NRRD, "%s: sorry, can't handle type %s input", me,
             airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }
  if (!( nrrdTypeBlock != _nline->type
         && 1 == _nline->dim)) {
    biffAddf(NRRD, "%s: nline must be 1-D array of scalars (not %u-D of %s)",
             me, _nline->dim, airEnumStr(nrrdType, _nline->type));
    return 1;
  }
  if (!( _nline->axis[0].size == nin->axis[saxi].size )) {
    biffAddf(NRRD, "%s: line length (%s) != axis[%u].size (%s)", me,
             airSprintSize_t(stmp[0], _nline->axis[0].size), saxi,
             airSprintSize_t(stmp[1], nin->axis[saxi].size));
    return 1;
  }
  if (1 == nin->dim) {
    biffAddf(NRRD, "%s: sorry, slice-based implementation requires input "
             "dimension > 1", me);
    return 1;
  }

  mop = airMopNew();
  rng = nrrdRangeNewSet(_nline, AIR_FALSE);
  airMopAdd(mop, rng, (airMopper)nrrdRangeNix, airMopAlways);
  if (rng->hasNonExist) {
    biffAddf(NRRD, "%s: had non-existent values in line", me);
    airMopError(mop); return 1;
  }

  nslice = nrrdNew();
  airMopAdd(mop, nslice, (airMopper)nrrdNix, airMopAlways);
  nline = nrrdNew();
  airMopAdd(mop, nline, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nline, _nline, nrrdTypeDouble)) {
    biffAddf(NRRD, "%s: problem copying line", me);
    airMopError(mop); return 1;
  }

  line = AIR_CAST(double *, nline->data);
  LL = nline->axis[0].size;
  numAbove = numBelow = 0;
  for (II=0; II<LL; II++) {
    if (line[II] >= thresh) {
      numAbove++;
    } else {
      numBelow++;
    }
  }
  if (noutAbove && !numAbove) {
    biffAddf(NRRD, "%s: want slices for val >= thresh %g, "
             "but highest value is %g < %g\n", me, thresh,
             rng->max, thresh);
    airMopError(mop); return 1;
  }
  if (noutBelow && !numBelow) {
    biffAddf(NRRD, "%s: want slices for val < thresh %g, "
             "but lowest value is %g >= %g\n", me, thresh,
             rng->min, thresh);
    airMopError(mop); return 1;
  }

  nslice->dim = nin->dim-1;
  nslice->type = nin->type;
  bb = 0;
  stride = nrrdElementSize(nin);
  for (aa=0; aa<nin->dim; aa++) {
    sizeAbove[aa] = sizeBelow[aa] = nin->axis[aa].size;
    if (aa != saxi) {
      axmap[aa] = aa;
      nslice->axis[bb].size = nin->axis[aa].size;
      if (aa < saxi) {
        stride *= nin->axis[aa].size;
      }
      bb++;
    } else {
      axmap[aa] = -1;
    }
  }
  sizeAbove[saxi] = numAbove;
  sizeBelow[saxi] = numBelow;
  if ((noutAbove
       && nrrdMaybeAlloc_nva(noutAbove, nin->type, nin->dim, sizeAbove))
      ||
      (noutBelow
       && nrrdMaybeAlloc_nva(noutBelow, nin->type, nin->dim, sizeBelow))) {
    biffAddf(NRRD, "%s: trouble allocating output", me);
    airMopError(mop); return 1;
  }
  if (noutAbove) {
    above = AIR_CAST(char *, noutAbove->data);
  } else {
    above = NULL;
  }
  if (noutBelow) {
    below = AIR_CAST(char *, noutBelow->data);
  } else {
    below = NULL;
  }

  /* the skinny */
  for (II=0; II<LL; II++) {
    if (line[II] >= thresh) {
      if (noutAbove) {
        nslice->data = above;
        if (nrrdSlice(nslice, nin, saxi, II)) {
          biffAddf(NRRD, "%s: trouble slicing (above) at %s", me,
                   airSprintSize_t(stmp[0], II));
          airMopError(mop); return 1;
        }
        above += stride;
      }
    } else {
      if (noutBelow) {
        nslice->data = below;
        if (nrrdSlice(nslice, nin, saxi, II)) {
          biffAddf(NRRD, "%s: trouble slicing (below) at %s", me,
                   airSprintSize_t(stmp[0], II));
          airMopError(mop); return 1;
        }
        below += stride;
      }
    }
  }

  if (noutAbove) {
    nrrdAxisInfoCopy(noutAbove, nin, axmap, NRRD_AXIS_INFO_NONE);
    if (nrrdBasicInfoCopy(noutAbove, nin,
                          NRRD_BASIC_INFO_DATA_BIT
                          | NRRD_BASIC_INFO_TYPE_BIT
                          | NRRD_BASIC_INFO_DIMENSION_BIT
                          | NRRD_BASIC_INFO_CONTENT_BIT
                          | NRRD_BASIC_INFO_COMMENTS_BIT
                          | (nrrdStateKeyValuePairsPropagate
                             ? 0
                             : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
  }
  if (noutBelow) {
    nrrdAxisInfoCopy(noutBelow, nin, axmap, NRRD_AXIS_INFO_NONE);
    if (nrrdBasicInfoCopy(noutBelow, nin,
                          NRRD_BASIC_INFO_DATA_BIT
                          | NRRD_BASIC_INFO_TYPE_BIT
                          | NRRD_BASIC_INFO_DIMENSION_BIT
                          | NRRD_BASIC_INFO_CONTENT_BIT
                          | NRRD_BASIC_INFO_COMMENTS_BIT
                          | (nrrdStateKeyValuePairsPropagate
                             ? 0
                             : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
******** nrrdSample_nva()
**
** given coordinates within a nrrd, copies the
** single element into given *val
*/
int
nrrdSample_nva(void *val, const Nrrd *nrrd, const size_t *coord) {
  static const char me[]="nrrdSample_nva";
  size_t I, size[NRRD_DIM_MAX], typeSize;
  unsigned int ai;
  char stmp[2][AIR_STRLEN_SMALL];

  if (!(nrrd && coord && val)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  /* this shouldn't actually be necessary .. */
  if (!nrrdElementSize(nrrd)) {
    biffAddf(NRRD, "%s: nrrd reports zero element size!", me);
    return 1;
  }

  typeSize = nrrdElementSize(nrrd);
  nrrdAxisInfoGet_nva(nrrd, nrrdAxisInfoSize, size);
  for (ai=0; ai<nrrd->dim; ai++) {
    if (!( coord[ai] < size[ai] )) {
      biffAddf(NRRD, "%s: coordinate %s on axis %d out of bounds (0 to %s)",
               me, airSprintSize_t(stmp[0], coord[ai]),
               ai, airSprintSize_t(stmp[1], size[ai]-1));
      return 1;
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
  static const char me[]="nrrdSample_va";
  unsigned int ai;
  size_t coord[NRRD_DIM_MAX];
  va_list ap;

  if (!(nrrd && val)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }

  va_start(ap, nrrd);
  for (ai=0; ai<nrrd->dim; ai++) {
    coord[ai] = va_arg(ap, size_t);
  }
  va_end(ap);

  if (nrrdSample_nva(val, nrrd, coord)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  return 0;
}

/*
******** nrrdSimpleCrop()
**
*/
int
nrrdSimpleCrop(Nrrd *nout, const Nrrd *nin, unsigned int crop) {
  static const char me[]="nrrdSimpleCrop";
  unsigned int ai;
  size_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];

  if (!(nout && nin)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  for (ai=0; ai<nin->dim; ai++) {
    min[ai] = crop;
    max[ai] = nin->axis[ai].size-1 - crop;
  }
  if (nrrdCrop(nout, nin, min, max)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  return 0;
}

int
nrrdCropAuto(Nrrd *nout, const Nrrd *nin,
             size_t _min[NRRD_DIM_MAX], size_t _max[NRRD_DIM_MAX],
             const unsigned int *keep, unsigned int keepNum,
             int measr, double frac, int offset) {
  static const char me[]="nrrdCropAuto";
  size_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX], NN, II;
  int cropdo[NRRD_DIM_MAX];
  airArray *mop;
  Nrrd *nperm, *nline;
  unsigned int axi;
  double *line;

  if (!( nout && nin )) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (keepNum && !keep) {
    biffAddf(NRRD, "%s: non-zero keepNum %u but NULL keep", me, keepNum);
    return 1;
  }
  if (airEnumValCheck(nrrdMeasure, measr)) {
    biffAddf(NRRD, "%s: invalid %s measr %d", me,
             nrrdMeasure->name, measr);
    return 1;
  }
  if (!( AIR_EXISTS(frac) && frac >= 0.0 && frac < 0.5 )) {
    biffAddf(NRRD, "%s: frac %g not in interval [0.0,0.5)", me, frac);
    return 1;
  }
  for (axi=0; axi<nin->dim; axi++) {
    cropdo[axi] = AIR_TRUE;
  }
  for (axi=0; axi<keepNum; axi++) {
    if (!( keep[axi] < nin->dim )) {
      biffAddf(NRRD, "%s: keep[%u] %u out of range [0,%u]", me,
               axi, keep[axi], nin->dim-1);
      return 1;
    }
    if (!cropdo[keep[axi]]) {
      biffAddf(NRRD, "%s: keep[%u] %u redundant", me, axi, keep[axi]);
      return 1;
    }
    cropdo[keep[axi]] = AIR_FALSE;
  }
  if (keepNum == nin->dim) {
    /* weird- wanted to keep all axes and crop none; that's easy */
    if (nrrdCopy(nout, nin)) {
      biffAddf(NRRD, "%s: trouble copying for trivial case", me);
      return 1;
    }
    return 0;
  }

  /* else there's work to do */
  mop = airMopNew();
  nperm = nrrdNew();
  airMopAdd(mop, nperm, (airMopper)nrrdNuke, airMopAlways);
  nline = nrrdNew();
  airMopAdd(mop, nline, (airMopper)nrrdNuke, airMopAlways);
  for (axi=0; axi<nin->dim; axi++) {
    double wsum, part;
    min[axi] = 0;
    max[axi] = nin->axis[axi].size-1;
    if (!cropdo[axi]) {
      continue;
    }
    /* else some analysis is required for this axis */
    /* NN is product of axes NOT being cropped */
    NN = nrrdElementNumber(nin)/nin->axis[axi].size;
    if (nrrdAxesSwap(nperm, nin, axi, nin->dim-1)
        || nrrdReshape_va(nperm, nperm, 2, NN, nin->axis[axi].size)
        || nrrdProject(nline, nperm, 0, measr, nrrdTypeDouble)) {
      biffAddf(NRRD, "%s: trouble forming projection line", me);
      airMopError(mop); return 1;
    }
    /* find sum of array */
    line = AIR_CAST(double *, nline->data);
    wsum = part = 0.0;
    for (II=0; II<nin->axis[axi].size; II++) {
      wsum += line[II];
    }
    /* sum bottom of array until hit fraction */
    for (II=0; II<nin->axis[axi].size; II++) {
      part += line[II];
      if (part/wsum > frac) {
        min[axi] = II;
        break;
      }
    }
    if (II == nin->axis[axi].size) {
      biffAddf(NRRD, "%s: confusion on bottom of axis %u", me, axi);
      airMopError(mop); return 1;
    }
    /* sum top of array until hit fraction */
    part = 0.0;
    for (II=nin->axis[axi].size; II>0; II--) {
      part += line[II-1];
      if (part/wsum > frac) {
        max[axi] = II-1;
        break;
      }
    }
    if (II == 0) {
      biffAddf(NRRD, "%s: confusion on top of axis %u", me, axi);
      airMopError(mop); return 1;
    }
    /*
    fprintf(stderr, "!%s: axis %u [%u,%u] --> ", me, axi,
            AIR_CAST(unsigned int, min[axi]),
            AIR_CAST(unsigned int, max[axi]));
    */
    /* adjust based on offset */
    if (offset > 0) {
      if (min[axi] < AIR_CAST(size_t, offset)) {
        /* desired outwards offset is more than cropping set */
        min[axi] = 0;
      } else {
        min[axi] -= offset;
      }
      max[axi] += offset;
      max[axi] = AIR_MIN(max[axi], nin->axis[axi].size-1);
    }
    /*
    fprintf(stderr, "[%u,%u]\n",
            AIR_CAST(unsigned int, min[axi]),
            AIR_CAST(unsigned int, max[axi]));
    */
  }

  /* can now do the crop */
  if (nrrdCrop(nout, nin, min, max)) {
    biffAddf(NRRD, "%s: trouble doing the crop", me);
    return 1;
  }
  /* save the extents */
  for (axi=0; axi<nin->dim; axi++) {
    if (_min) {
      _min[axi] = min[axi];
    }
    if (_max) {
      _max[axi] = max[axi];
    }
  }
  airMopOkay(mop);
  return 0;
}

/* ---- END non-NrrdIO */
