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
******** nrrdHisto()
**
** makes a 1D histogram of a given size and type
**
** pre-NrrdRange policy:
** this looks at nin->min and nin->max to see if they are both non-NaN.
** If so, it uses these as the range of the histogram, otherwise it
** finds the min and max present in the volume.  If nin->min and nin->max
** are being used as the histogram range, then values which fall outside
** this are ignored (they don't contribute to the histogram).
**
** post-NrrdRange policy:
*/
int
nrrdHisto(Nrrd *nout, const Nrrd *nin, const NrrdRange *_range,
          const Nrrd *nwght, size_t bins, int type) {
  static const char me[]="nrrdHisto", func[]="histo";
  size_t I, num, idx;
  airArray *mop;
  NrrdRange *range;
  double min, max, eps, val, count, incr, (*lup)(const void *v, size_t I);

  if (!(nin && nout)) {
    /* _range and nwght can be NULL */
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == nin) {
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  if (!(bins > 0)) {
    char stmp[AIR_STRLEN_SMALL];
    biffAddf(NRRD, "%s: bins value (%s) invalid", me,
             airSprintSize_t(stmp, bins));
    return 1;
  }
  if (airEnumValCheck(nrrdType, type) || nrrdTypeBlock == type) {
    biffAddf(NRRD, "%s: invalid nrrd type %d", me, type);
    return 1;
  }
  if (nwght) {
    if (nout==nwght) {
      biffAddf(NRRD, "%s: nout==nwght disallowed", me);
      return 1;
    }
    if (nrrdTypeBlock == nwght->type) {
      biffAddf(NRRD, "%s: nwght type %s invalid", me,
               airEnumStr(nrrdType, nrrdTypeBlock));
      return 1;
    }
    if (!nrrdSameSize(nin, nwght, AIR_TRUE)) {
      biffAddf(NRRD, "%s: nwght size mismatch with nin", me);
      return 1;
    }
    lup = nrrdDLookup[nwght->type];
  } else {
    lup = NULL;
  }

  if (nrrdMaybeAlloc_va(nout, type, 1, bins)) {
    char stmp[AIR_STRLEN_SMALL];
    biffAddf(NRRD, "%s: failed to alloc histo array (len %s)", me,
             airSprintSize_t(stmp, bins));
    return 1;
  }
  mop = airMopNew();
  /* nout->axis[0].size set */
  nout->axis[0].spacing = AIR_NAN;
  nout->axis[0].thickness = AIR_NAN;
  if (nout && AIR_EXISTS(nout->axis[0].min) && AIR_EXISTS(nout->axis[0].max)) {
    /* HEY: total hack to externally nail down min and max of histogram:
       use the min and max already set on axis[0] */
    /* HEY: shouldn't this blatent hack be further restricted by also
       checking the existence of range->min and range->max ? */
    min = nout->axis[0].min;
    max = nout->axis[0].max;
  } else {
    if (_range) {
      range = nrrdRangeCopy(_range);
      nrrdRangeSafeSet(range, nin, nrrdBlind8BitRangeState);
    } else {
      range = nrrdRangeNewSet(nin, nrrdBlind8BitRangeState);
    }
    airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    min = range->min;
    max = range->max;
    nout->axis[0].min = min;
    nout->axis[0].max = max;
  }
  eps = (min == max ? 1.0 : 0.0);
  nout->axis[0].center = nrrdCenterCell;
  /* nout->axis[0].label set below */

  /* make histogram */
  num = nrrdElementNumber(nin);
  for (I=0; I<num; I++) {
    val = nrrdDLookup[nin->type](nin->data, I);
    if (AIR_EXISTS(val)) {
      if (val < min || val > max+eps) {
        /* value is outside range; ignore it */
        continue;
      }
      if (AIR_IN_CL(min, val, max)) {
        idx = airIndex(min, val, max+eps, AIR_CAST(unsigned int, bins));
        /*
        printf("!%s: %d: index(%g, %g, %g, %d) = %d\n",
               me, (int)I, min, val, max, bins, idx);
        */
        /* count is a double in order to simplify clamping the
           hit values to the representable range for nout->type */
        count = nrrdDLookup[nout->type](nout->data, idx);
        incr = nwght ? lup(nwght->data, I) : 1;
        count = nrrdDClamp[nout->type](count + incr);
        nrrdDInsert[nout->type](nout->data, idx, count);
      }
    }
  }

  if (nrrdContentSet_va(nout, func, nin, "%d", bins)) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  nout->axis[0].label = (char *)airFree(nout->axis[0].label);
  nout->axis[0].label = (char *)airStrdup(nout->content);
  if (!nrrdStateKindNoop) {
    nout->axis[0].kind = nrrdKindDomain;
  }

  airMopOkay(mop);
  return 0;
}

int
nrrdHistoCheck(const Nrrd *nhist) {
  static const char me[]="nrrdHistoCheck";

  if (!nhist) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdTypeBlock == nhist->type) {
    biffAddf(NRRD, "%s: has non-scalar %s type",
             me, airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }
  if (nrrdHasNonExist(nhist)) {
    biffAddf(NRRD, "%s: has non-existent values", me);
    return 1;
  }
  if (1 != nhist->dim) {
    biffAddf(NRRD, "%s: dim == %u != 1",
             me, nhist->dim);
    return 1;
  }
  if (!(nhist->axis[0].size > 1)) {
    biffAddf(NRRD, "%s: has single sample along sole axis", me);
    return 1;
  }

  return 0;
}

int
nrrdHistoDraw(Nrrd *nout, const Nrrd *nin,
              size_t sy, int showLog, double max) {
  static const char me[]="nrrdHistoDraw", func[]="dhisto";
  char cmt[AIR_STRLEN_MED], stmp[AIR_STRLEN_SMALL];
  unsigned int ki, numticks, *linY, *logY, tick, *ticks;
  double hits, maxhits, usemaxhits;
  unsigned char *pgmData;
  airArray *mop;
  int E;
  size_t sx, xi, yi, maxhitidx;

  if (!(nin && nout && sy > 0)) {
    biffAddf(NRRD, "%s: invalid args", me);
    return 1;
  }
  if (nout == nin) {
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  if (nrrdHistoCheck(nin)) {
    biffAddf(NRRD, "%s: input nrrd not a histogram", me);
    return 1;
  }
  sx = nin->axis[0].size;
  nrrdBasicInfoInit(nout, NRRD_BASIC_INFO_DATA_BIT);
  if (nrrdMaybeAlloc_va(nout, nrrdTypeUChar, 2, sx, sy)) {
    biffAddf(NRRD, "%s: failed to allocate histogram image", me);
    return 1;
  }
  /* perhaps I should be using nrrdAxisInfoCopy */
  nout->axis[0].spacing = nout->axis[1].spacing = AIR_NAN;
  nout->axis[0].thickness = nout->axis[1].thickness = AIR_NAN;
  nout->axis[0].min = nin->axis[0].min;
  nout->axis[0].max = nin->axis[0].max;
  nout->axis[0].center = nout->axis[1].center = nrrdCenterCell;
  nout->axis[0].label = (char *)airStrdup(nin->axis[0].label);
  nout->axis[1].label = (char *)airFree(nout->axis[1].label);
  pgmData = (unsigned char *)nout->data;
  maxhits = 0.0;
  maxhitidx = 0;
  for (xi=0; xi<sx; xi++) {
    hits = nrrdDLookup[nin->type](nin->data, xi);
    if (maxhits < hits) {
      maxhits = hits;
      maxhitidx = xi;
    }
  }
  if (AIR_EXISTS(max) && max > 0) {
    usemaxhits = max;
  } else {
    usemaxhits = maxhits;
  }
  nout->axis[1].min = usemaxhits;
  nout->axis[1].max = 0;
  numticks = AIR_CAST(unsigned int, log10(usemaxhits + 1));
  mop = airMopNew();
  ticks = AIR_CALLOC(numticks, unsigned int);
  airMopMem(mop, &ticks, airMopAlways);
  linY = AIR_CALLOC(sx, unsigned int);
  airMopMem(mop, &linY, airMopAlways);
  logY = AIR_CALLOC(sx, unsigned int);
  airMopMem(mop, &logY, airMopAlways);
  if (!(ticks && linY && logY)) {
    biffAddf(NRRD, "%s: failed to allocate temp arrays", me);
    airMopError(mop); return 1;
  }
  for (ki=0; ki<numticks; ki++) {
    ticks[ki] = airIndex(0, log10(pow(10,ki+1) + 1), log10(usemaxhits+1),
                         AIR_CAST(unsigned int, sy));
  }
  for (xi=0; xi<sx; xi++) {
    hits = nrrdDLookup[nin->type](nin->data, xi);
    linY[xi] = airIndex(0, hits, usemaxhits,
                        AIR_CAST(unsigned int, sy));
    logY[xi] = airIndex(0, log10(hits+1), log10(usemaxhits+1),
                        AIR_CAST(unsigned int, sy));
    /* printf("%d -> %d,%d", x, linY[x], logY[x]); */
  }
  for (yi=0; yi<sy; yi++) {
    tick = 0;
    for (ki=0; ki<numticks; ki++)
      tick |= ticks[ki] == yi;
    for (xi=0; xi<sx; xi++) {
      pgmData[xi + sx*(sy-1-yi)] =
        (2 == showLog    /* HACK: draw log curve, but not log tick marks */
         ? (yi >= logY[xi]
            ? 0                   /* above log curve                     */
            : (yi >= linY[xi]
               ? 128              /* below log curve, above normal curve */
               : 255              /* below log curve, below normal curve */
               )
            )
         : (!showLog
            ? (yi >= linY[xi] ? 0 : 255)
            : (yi >= logY[xi]     /* above log curve                     */
               ? (!tick ? 0       /*                    not on tick mark */
                  : 255)          /*                    on tick mark     */
               : (yi >= linY[xi]  /* below log curve, above normal curve */
                  ? (!tick ? 128  /*                    not on tick mark */
                     : 0)         /*                    on tick mark     */
                  :255            /* below log curve, below normal curve */
                  )
               )
            )
         );
    }
  }
  E = 0;
  sprintf(cmt, "min value: %g\n", nout->axis[0].min);
  if (!E) E |= nrrdCommentAdd(nout, cmt);
  sprintf(cmt, "max value: %g\n", nout->axis[0].max);
  if (!E) E |= nrrdCommentAdd(nout, cmt);
  sprintf(cmt, "max hits: %g, in bin %s, around value %g\n", maxhits,
          airSprintSize_t(stmp, maxhitidx),
          nrrdAxisInfoPos(nout, 0, AIR_CAST(double, maxhitidx)));
  if (!E) E |= nrrdCommentAdd(nout, cmt);
  if (!E) E |= nrrdContentSet_va(nout, func, nin, "%s",
                                 airSprintSize_t(stmp, sy));
  if (E) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }

  /* bye */
  airMopOkay(mop);
  return 0;
}

/*
******** nrrdHistoAxis
**
** replace scanlines along one scanline with a histogram of the scanline
**
** this looks at nin->min and nin->max to see if they are both non-NaN.
** If so, it uses these as the range of the histogram, otherwise it
** finds the min and max present in the volume
**
** By its very nature, and by the simplicity of this implemention,
** this can be a slow process due to terrible memory locality.  User
** may want to permute axes before and after this, but that can be
** slow too...
*/
int
nrrdHistoAxis(Nrrd *nout, const Nrrd *nin, const NrrdRange *_range,
              unsigned int hax, size_t bins, int type) {
  static const char me[]="nrrdHistoAxis", func[]="histax";
  int map[NRRD_DIM_MAX];
  unsigned int ai, hidx;
  size_t szIn[NRRD_DIM_MAX], szOut[NRRD_DIM_MAX], size[NRRD_DIM_MAX],
    coordIn[NRRD_DIM_MAX], coordOut[NRRD_DIM_MAX];
  size_t I, hI, num;
  double val, count;
  airArray *mop;
  NrrdRange *range;

  if (!(nin && nout)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == nin) {
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  if (!(bins > 0)) {
    char stmp[AIR_STRLEN_SMALL];
    biffAddf(NRRD, "%s: bins value (%s) invalid", me,
             airSprintSize_t(stmp, bins));
    return 1;
  }
  if (airEnumValCheck(nrrdType, type) || nrrdTypeBlock == type) {
    biffAddf(NRRD, "%s: invalid nrrd type %d", me, type);
    return 1;
  }
  if (!( hax <= nin->dim-1 )) {
    biffAddf(NRRD, "%s: axis %d is not in range [0,%d]",
             me, hax, nin->dim-1);
    return 1;
  }
  mop = airMopNew();
  if (_range) {
    range = nrrdRangeCopy(_range);
    nrrdRangeSafeSet(range, nin, nrrdBlind8BitRangeState);
  } else {
    range = nrrdRangeNewSet(nin, nrrdBlind8BitRangeState);
  }
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  size[hax] = bins;
  if (nrrdMaybeAlloc_nva(nout, type, nin->dim, size)) {
    biffAddf(NRRD, "%s: failed to alloc output nrrd", me);
    airMopError(mop); return 1;
  }

  /* copy axis information */
  for (ai=0; ai<nin->dim; ai++) {
    map[ai] = ai != hax ? (int)ai : -1;
  }
  nrrdAxisInfoCopy(nout, nin, map, NRRD_AXIS_INFO_NONE);
  /* axis hax now has to be set manually */
  nout->axis[hax].size = bins;
  nout->axis[hax].spacing = AIR_NAN; /* min and max convey the information */
  nout->axis[hax].thickness = AIR_NAN;
  nout->axis[hax].min = range->min;
  nout->axis[hax].max = range->max;
  nout->axis[hax].center = nrrdCenterCell;
  if (nin->axis[hax].label) {
    nout->axis[hax].label = AIR_CALLOC(strlen("histax()")
                                       + strlen(nin->axis[hax].label)
                                       + 1, char);
    if (nout->axis[hax].label) {
      sprintf(nout->axis[hax].label, "histax(%s)", nin->axis[hax].label);
    } else {
      biffAddf(NRRD, "%s: couldn't allocate output label", me);
      airMopError(mop); return 1;
    }
  } else {
    nout->axis[hax].label = NULL;
  }
  if (!nrrdStateKindNoop) {
    nout->axis[hax].kind = nrrdKindDomain;
  }

  /* the skinny: we traverse the input samples in linear order, and
     increment the bin in the histogram for the scanline we're in.
     This is not terribly clever, and the memory locality is a
     disaster */
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  nrrdAxisInfoGet_nva(nout, nrrdAxisInfoSize, szOut);
  memset(coordIn, 0, NRRD_DIM_MAX*sizeof(size_t));
  num = nrrdElementNumber(nin);
  for (I=0; I<num; I++) {
    /* get input nrrd value and compute its histogram index */
    val = nrrdDLookup[nin->type](nin->data, I);
    if (AIR_EXISTS(val) && AIR_IN_CL(range->min, val, range->max)) {
      hidx = airIndex(range->min, val, range->max, AIR_CAST(unsigned int, bins));
      memcpy(coordOut, coordIn, nin->dim*sizeof(size_t));
      coordOut[hax] = (unsigned int)hidx;
      NRRD_INDEX_GEN(hI, coordOut, szOut, nout->dim);
      count = nrrdDLookup[nout->type](nout->data, hI);
      count = nrrdDClamp[nout->type](count + 1);
      nrrdDInsert[nout->type](nout->data, hI, count);
    }
    NRRD_COORD_INCR(coordIn, szIn, nin->dim, 0);
  }

  if (nrrdContentSet_va(nout, func, nin, "%d,%d", hax, bins)) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  nrrdBasicInfoInit(nout, (NRRD_BASIC_INFO_DATA_BIT
                           | NRRD_BASIC_INFO_TYPE_BIT
                           | NRRD_BASIC_INFO_DIMENSION_BIT
                           | 0 /* what? */ ));
  airMopOkay(mop);
  return 0;
}

int
nrrdHistoJoint(Nrrd *nout, const Nrrd *const *nin,
               const NrrdRange *const *_range, unsigned int numNin,
               const Nrrd *nwght, const size_t *bins,
               int type, const int *clamp) {
  static const char me[]="nrrdHistoJoint", func[]="jhisto";
  int skip, hadContent;
  double val, count, incr, (*lup)(const void *v, size_t I);
  size_t Iin, Iout, numEl, coord[NRRD_DIM_MAX], totalContentStrlen;
  airArray *mop;
  NrrdRange **range;
  unsigned int nii, ai;

  /* error checking */
  /* nwght can be NULL -> weighting is constant 1.0 */
  if (!(nout && nin && bins && clamp)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(numNin >= 1)) {
    biffAddf(NRRD, "%s: need numNin >= 1 (not %d)", me, numNin);
    return 1;
  }
  if (numNin > NRRD_DIM_MAX) {
    biffAddf(NRRD, "%s: can only deal with up to %d nrrds (not %d)", me,
             NRRD_DIM_MAX, numNin);
    return 1;
  }
  for (nii=0; nii<numNin; nii++) {
    if (!(nin[nii])) {
      biffAddf(NRRD, "%s: input nrrd #%u NULL", me, nii);
      return 1;
    }
    if (nout==nin[nii]) {
      biffAddf(NRRD, "%s: nout==nin[%d] disallowed", me, nii);
      return 1;
    }
    if (nrrdTypeBlock == nin[nii]->type) {
      biffAddf(NRRD, "%s: nin[%d] type %s invalid", me, nii,
               airEnumStr(nrrdType, nrrdTypeBlock));
      return 1;
    }
  }
  if (airEnumValCheck(nrrdType, type) || nrrdTypeBlock == type) {
    biffAddf(NRRD, "%s: invalid nrrd type %d", me, type);
    return 1;
  }
  mop = airMopNew();
  range = AIR_CALLOC(numNin, NrrdRange*);
  airMopAdd(mop, range, airFree, airMopAlways);
  for (ai=0; ai<numNin; ai++) {
    if (_range && _range[ai]) {
      range[ai] = nrrdRangeCopy(_range[ai]);
      nrrdRangeSafeSet(range[ai], nin[ai], nrrdBlind8BitRangeState);
    } else {
      range[ai] = nrrdRangeNewSet(nin[ai], nrrdBlind8BitRangeState);
    }
    airMopAdd(mop, range[ai], (airMopper)nrrdRangeNix, airMopAlways);
  }
  for (ai=0; ai<numNin; ai++) {
    if (!nin[ai]) {
      biffAddf(NRRD, "%s: input nrrd[%d] NULL", me, ai);
      return 1;
    }
    if (!(bins[ai] >= 1)) {
      char stmp[AIR_STRLEN_SMALL];
      biffAddf(NRRD, "%s: need bins[%u] >= 1 (not %s)", me, ai,
               airSprintSize_t(stmp, bins[ai]));
      return 1;
    }
    if (ai && !nrrdSameSize(nin[0], nin[ai], AIR_TRUE)) {
      biffAddf(NRRD, "%s: nin[0] (n1) mismatch with nin[%u] (n2)", me, ai);
      return 1;
    }
  }

  /* check nwght */
  if (nwght) {
    if (nout==nwght) {
      biffAddf(NRRD, "%s: nout==nwght disallowed", me);
      return 1;
    }
    if (nrrdTypeBlock == nwght->type) {
      biffAddf(NRRD, "%s: nwght type %s invalid", me,
               airEnumStr(nrrdType, nrrdTypeBlock));
      return 1;
    }
    if (!nrrdSameSize(nin[0], nwght, AIR_TRUE)) {
      biffAddf(NRRD, "%s: nwght size mismatch with nin[0]", me);
      return 1;
    }
    lup = nrrdDLookup[nwght->type];
  } else {
    lup = NULL;
  }

  /* allocate output nrrd */
  if (nrrdMaybeAlloc_nva(nout, type, numNin, bins)) {
    biffAddf(NRRD, "%s: couldn't allocate output histogram", me);
    return 1;
  }
  hadContent = 0;
  totalContentStrlen = 0;
  for (ai=0; ai<numNin; ai++) {
    nout->axis[ai].size = bins[ai];
    nout->axis[ai].spacing = AIR_NAN;
    nout->axis[ai].thickness = AIR_NAN;
    nout->axis[ai].min = range[ai]->min;
    nout->axis[ai].max = range[ai]->max;
    nout->axis[ai].center = nrrdCenterCell;
    if (!nrrdStateKindNoop) {
      nout->axis[ai].kind = nrrdKindDomain;
    }
    if (nin[ai]->content) {
      hadContent = 1;
      totalContentStrlen += strlen(nin[ai]->content);
      nout->axis[ai].label = AIR_CALLOC(strlen("histo(,)")
                                        + strlen(nin[ai]->content)
                                        + 11
                                        + 1, char);
      if (nout->axis[ai].label) {
        char stmp[AIR_STRLEN_SMALL];
        sprintf(nout->axis[ai].label, "histo(%s,%s)", nin[ai]->content,
                airSprintSize_t(stmp, bins[ai]));
      } else {
        biffAddf(NRRD, "%s: couldn't allocate output label #%u", me, ai);
        return 1;
      }
    } else {
      nout->axis[ai].label = (char *)airFree(nout->axis[ai].label);
      totalContentStrlen += 2;
    }
  }

  /* the skinny */
  numEl = nrrdElementNumber(nin[0]);
  for (Iin=0; Iin<numEl; Iin++) {
    skip = 0;
    for (ai=0; ai<numNin; ai++) {
      val = nrrdDLookup[nin[ai]->type](nin[ai]->data, Iin);
      if (!AIR_EXISTS(val)) {
        /* coordinate d in the joint histo can't be determined
           if nin[ai] has a non-existent value here */
        skip = 1;
        break;
      }
      if (!AIR_IN_CL(range[ai]->min, val, range[ai]->max)) {
        if (clamp[ai]) {
          val = AIR_CLAMP(range[ai]->min, val, range[ai]->max);
        } else {
          skip = 1;
          break;
        }
      }
      coord[ai] = AIR_CAST(size_t, airIndexClampULL(range[ai]->min,
                                                    val,
                                                    range[ai]->max,
                                                    bins[ai]));
      /* printf(" -> coord = %d; ", coord[d]); fflush(stdout); */
    }
    if (skip) {
      continue;
    }
    NRRD_INDEX_GEN(Iout, coord, bins, numNin);
    count = nrrdDLookup[nout->type](nout->data, Iout);
    incr = nwght ? lup(nwght->data, Iin) : 1.0;
    count = nrrdDClamp[nout->type](count + incr);
    nrrdDInsert[nout->type](nout->data, Iout, count);
  }

  /* HEY: switch to nrrdContentSet_va? */
  if (hadContent) {
    nout->content = AIR_CALLOC(strlen(func) + strlen("()")
                               + numNin*strlen(",")
                               + totalContentStrlen
                               + 1, char);
    if (nout->content) {
      size_t len;
      sprintf(nout->content, "%s(", func);
      for (ai=0; ai<numNin; ai++) {
        len = strlen(nout->content);
        strcpy(nout->content + len,
               nin[ai]->content ? nin[ai]->content : "?");
        len = strlen(nout->content);
        nout->content[len] = ai < numNin-1 ? ',' : ')';
      }
      nout->content[len+1] = '\0';
    } else {
      biffAddf(NRRD, "%s: couldn't allocate output content", me);
      return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
******** nrrdHistoThresholdOtsu
**
** does simple Otsu tresholding of a histogram, with a variable exponont.
** When "expo" is 2.0, it computes variance; lower values probably represent
** greater insensitivities to outliers. Idea from ...
*/
int
nrrdHistoThresholdOtsu(double *threshP, const Nrrd *_nhist, double expo) {
  static const char me[]="nrrdHistoThresholdOtsu";
  unsigned int histLen, histIdx, maxIdx;
  Nrrd *nhist, *nbvar;
  double *hist, *bvar, thresh, num0, num1, mean0, mean1,
    onum0, onum1, omean0, omean1, max;
  airArray *mop;

  if (!(threshP && _nhist)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdHistoCheck(_nhist)) {
    biffAddf(NRRD, "%s: input nrrd not a histogram", me);
    return 1;
  }

  mop = airMopNew();
  airMopAdd(mop, nhist = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nbvar = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nhist, _nhist, nrrdTypeDouble)
      || nrrdCopy(nbvar, nhist)) {
    biffAddf(NRRD, "%s: making local copies", me);
    airMopError(mop); return 1;
  }
  hist = AIR_CAST(double*, nhist->data);
  bvar = AIR_CAST(double*, nbvar->data);

  histLen = AIR_CAST(unsigned int, nhist->axis[0].size);
  num1 = mean1 = 0;
  for (histIdx=0; histIdx<histLen; histIdx++) {
    num1 += hist[histIdx];
    mean1 += hist[histIdx]*histIdx;
  }
  if (num1) {
    num0 = 0;
    mean0 = 0;
    mean1 /= num1;
    for (histIdx=0; histIdx<histLen; histIdx++) {
      if (histIdx) {
        onum0 = num0;
        onum1 = num1;
        omean0 = mean0;
        omean1 = mean1;
        num0 = onum0 + hist[histIdx-1];
        num1 = onum1 - hist[histIdx-1];
        mean0 = (omean0*onum0 + hist[histIdx-1]*(histIdx-1)) / num0;
        mean1 = (omean1*onum1 - hist[histIdx-1]*(histIdx-1)) / num1;
      }
      bvar[histIdx] = num0*num1*airSgnPow(mean1 - mean0, expo);
    }
    max = bvar[0];
    maxIdx = 0;
    for (histIdx=1; histIdx<histLen; histIdx++) {
      if (bvar[histIdx] > max) {
        max = bvar[histIdx];
        maxIdx = histIdx;
      }
    }
    thresh = maxIdx;
  } else {
    thresh = histLen/2;
  }

  if (AIR_EXISTS(nhist->axis[0].min) && AIR_EXISTS(nhist->axis[0].max)) {
    thresh = NRRD_CELL_POS(nhist->axis[0].min, nhist->axis[0].max,
                           histLen, thresh);
  }
  *threshP = thresh;
  airMopOkay(mop);
  return 0;
}
