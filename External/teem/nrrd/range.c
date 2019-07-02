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

NrrdRange *
nrrdRangeNew(double min, double max) {
  NrrdRange *range;

  range = (NrrdRange *)calloc(1, sizeof(NrrdRange));
  if (range) {
    range->min = min;
    range->max = max;
    range->hasNonExist = nrrdHasNonExistUnknown;
  }
  return range;
}

NrrdRange *
nrrdRangeCopy(const NrrdRange *rin) {
  NrrdRange *rout=NULL;

  if (rin) {
    rout = nrrdRangeNew(rin->min, rin->max);
    rout->hasNonExist = rin->hasNonExist;
  }
  return rout;
}

NrrdRange *
nrrdRangeNix(NrrdRange *range) {

  airFree(range);
  return NULL;
}

void
nrrdRangeReset(NrrdRange *range) {

  if (range) {
    range->min = AIR_NAN;
    range->max = AIR_NAN;
    range->hasNonExist = nrrdHasNonExistUnknown;
  }
}

/*
** not using biff (obviously)
*/
void
nrrdRangeSet(NrrdRange *range, const Nrrd *nrrd, int blind8BitRange) {
  NRRD_TYPE_BIGGEST _min, _max;
  int blind;

  if (!range) {
    return;
  }
  if (nrrd
      && !airEnumValCheck(nrrdType, nrrd->type)
      && nrrdTypeBlock != nrrd->type) {
    blind = (nrrdBlind8BitRangeTrue == blind8BitRange
             || (nrrdBlind8BitRangeState == blind8BitRange
                 && nrrdStateBlind8BitRange));
    if (blind && 1 == nrrdTypeSize[nrrd->type]) {
      if (nrrdTypeChar == nrrd->type) {
        range->min = SCHAR_MIN;
        range->max = SCHAR_MAX;
      } else {
        range->min = 0;
        range->max = UCHAR_MAX;
      }
      range->hasNonExist = nrrdHasNonExistFalse;
    } else {
      nrrdMinMaxExactFind[nrrd->type](&_min, &_max, &(range->hasNonExist),
                                      nrrd);
      range->min = nrrdDLoad[nrrd->type](&_min);
      range->max = nrrdDLoad[nrrd->type](&_max);
    }
  } else {
    range->min = range->max = AIR_NAN;
    range->hasNonExist = nrrdHasNonExistUnknown;
  }
  return;
}

/*
******** nrrdRangePercentileSet
**
** this is called when information about the range of values in the
** nrrd is requested; and the learned information is put into "range"
** (overwriting whatever is there!)
**
** uses biff
*/
int
nrrdRangePercentileSet(NrrdRange *range, const Nrrd *nrrd,
                       double minPerc, double maxPerc,
                       unsigned int hbins, int blind8BitRange) {
  static const char me[]="nrrdRangePercentileSet";
  airArray *mop;
  Nrrd *nhist;
  double allmin, allmax, minval, maxval, *hist, sum, total, sumPerc;
  unsigned int hi;

  if (!(range && nrrd)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 0;
  }
  nrrdRangeSet(range, nrrd, blind8BitRange);
  /* range->min and range->max
     are the full range of nrrd (except maybe for blind8) */
  if (!minPerc && !maxPerc) {
    /* wanted full range; there is nothing more to do */
    return 0;
  }
  if (!hbins) {
    biffAddf(NRRD, "%s: sorry, non-histogram-based percentiles not "
             "currently implemented (need hbins > 0)", me);
    return 1;
  }
  if (!(hbins >= 5)) {
    biffAddf(NRRD, "%s: # histogram bins %u unreasonably small", me, hbins);
    return 1;
  }
  if (range->hasNonExist) {
    biffAddf(NRRD, "%s: sorry, can currently do histogram-based percentiles "
             "only in arrays with no non-existent values", me);
    return 1;
  }

  mop = airMopNew();
  allmin = range->min;
  allmax = range->max;

  nhist = nrrdNew();
  airMopAdd(mop, nhist, (airMopper)nrrdNuke, airMopAlways);
  /* the histogram is over the entire range of values */
  if (nrrdHisto(nhist, nrrd, range, NULL, hbins, nrrdTypeDouble)) {
    biffAddf(NRRD, "%s: trouble making histogram", me);
    airMopError(mop);
    return 1;
  }
  hist = AIR_CAST(double *, nhist->data);
  total = AIR_CAST(double, nrrdElementNumber(nrrd));
  if (minPerc) {
    minval = AIR_NAN;
    sumPerc = AIR_ABS(minPerc)*total/100.0;
    sum = hist[0];
    for (hi=1; hi<hbins; hi++) {
      sum += hist[hi];
      if (sum >= sumPerc) {
        minval = AIR_AFFINE(0, hi-1, hbins-1,
                            nhist->axis[0].min, nhist->axis[0].max);
        break;
      }
    }
    if (hi == hbins || !AIR_EXISTS(minval)) {
      biffAddf(NRRD, "%s: failed to find lower %g-percentile value",
               me, minPerc);
      airMopError(mop);
      return 1;
    }
    range->min = (minPerc > 0
                  ? minval
                  : 2*allmin - minval);
    /* fprintf(stderr, "!%s: %g-%% min = %g\n", me, min, minval); */
  }
  if (maxPerc) {
    maxval = AIR_NAN;
    sumPerc = AIR_ABS(maxPerc)*total/100.0;
    sum = hist[hbins-1];
    for (hi=hbins-1; hi; hi--) {
      sum += hist[hi-1];
      if (sum >= sumPerc) {
        maxval = AIR_AFFINE(0, hi, hbins-1,
                            nhist->axis[0].min, nhist->axis[0].max);
        break;
      }
    }
    if (!hi || !AIR_EXISTS(maxval)) {
      biffAddf(NRRD, "%s: failed to find upper %g-percentile value", me,
               maxPerc);
      airMopError(mop);
      return 1;
    }
    range->max = (maxPerc > 0
                  ? maxval
                  : 2*allmax - maxval);
    /* fprintf(stderr, "!%s: %g-%% max = %g\n", me, max, maxval); */
  }

  airMopOkay(mop);
  return 0;
}

/*
******** nrrdRangePercentileFromStringSet
**
** Implements smarts of figuring out a range from no info ("nan"),
** a known percentile (e.g. "3%") or a known explicit value (e.g. "3"),
** for both min and max.  Used by "unu quantize" and others.
*/
int
nrrdRangePercentileFromStringSet(NrrdRange *range, const Nrrd *nrrd,
                                 const char *_minStr, const char *_maxStr,
                                 unsigned int hbins, int blind8BitRange) {
  static const char me[]="nrrdRangePercentileFromStringSet";
  double minVal, maxVal, minPerc, maxPerc;
  char *minStr, *maxStr;
  unsigned int mmIdx;
  airArray *mop;

  if (!(range && nrrd && _minStr && _maxStr)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  minStr = airStrdup(_minStr);
  airMopAdd(mop, minStr, airFree, airMopAlways);
  maxStr = airStrdup(_maxStr);
  airMopAdd(mop, maxStr, airFree, airMopAlways);

  /* parse min and max */
  minVal = maxVal = minPerc = maxPerc = AIR_NAN;
  for (mmIdx=0; mmIdx<=1; mmIdx++) {
    int percwant;
    double val, *mmv, *mmp;
    char *mmStr;
    if (0 == mmIdx) {
      mmv = &minVal;
      mmp = &minPerc;
      mmStr = minStr;
    } else {
      mmv = &maxVal;
      mmp = &maxPerc;
      mmStr = maxStr;
    }
    if (airEndsWith(mmStr, NRRD_MINMAX_PERC_SUFF)) {
      percwant = AIR_TRUE;
      mmStr[strlen(mmStr)-strlen(NRRD_MINMAX_PERC_SUFF)] = '\0';
    } else {
      percwant = AIR_FALSE;
    }
    if (1 != airSingleSscanf(mmStr, "%lf", &val)) {
      biffAddf(NRRD, "%s: couldn't parse \"%s\" for %s", me,
               !mmIdx ? _minStr : _maxStr,
               !mmIdx ? "minimum" : "maximum");
      airMopError(mop);
      return 1;
    }
    if (percwant) {
      if (!AIR_EXISTS(val)) {
        biffAddf(NRRD, "%s: %s percentile must exist", me,
                 !mmIdx ? "minimum" : "maximum");
        airMopError(mop);
        return 1;
      }
      /* setting a finite percentile */
      *mmp = val;
    } else if (!AIR_EXISTS(val)) {
      /* don't want a percentile, and given value is nan
         => same as full range == zero percentile */
      *mmp = 0.0;
    } else {
      /* value exists: given explicitly */
      *mmv = val;
    }
  }
  /* so whenever one end of the range is not given explicitly,
     it has been mapped to a statement about the percentile,
     which does require learning about the nrrd's values */
  if (AIR_EXISTS(minPerc) || AIR_EXISTS(maxPerc)) {
    if (nrrdRangePercentileSet(range, nrrd,
                               AIR_EXISTS(minPerc) ? minPerc : 0.0,
                               AIR_EXISTS(maxPerc) ? maxPerc : 0.0,
                               hbins, blind8BitRange)) {
      biffAddf(NRRD, "%s: trouble finding percentile range", me);
      airMopError(mop);
      return 1;
    }
  }
  /* whatever explicit values were given are now stored */
  if (AIR_EXISTS(minVal)) {
    range->min = minVal;
  }
  if (AIR_EXISTS(maxVal)) {
    range->max = maxVal;
  }

  airMopOkay(mop);
  return 0;
}

/*
** wrapper around nrrdRangeSet that (effectively) sets range->min
** and range->min only if they didn't already exist
*/
void
nrrdRangeSafeSet(NrrdRange *range, const Nrrd *nrrd, int blind8BitRange) {
  double minIn, maxIn;

  if (!range) {
    return;
  }
  minIn = range->min;
  maxIn = range->max;
  nrrdRangeSet(range, nrrd, blind8BitRange);
  if (AIR_EXISTS(minIn)) {
    range->min = minIn;
  }
  if (AIR_EXISTS(maxIn)) {
    range->max = maxIn;
  }
  return;
}

/*
** does not use biff
*/
NrrdRange *
nrrdRangeNewSet(const Nrrd *nrrd, int blind8BitRange) {
  NrrdRange *range;

  range = nrrdRangeNew(0, 0);  /* doesn't matter what values are used here */
  nrrdRangeSet(range, nrrd, blind8BitRange);
  return range;
}

/*
******** nrrdHasNonExist
**
** returns the nrrdHasNonExist* enum value appropriate for a given nrrd.
** By cleverness, this value can be used as a regular C boolean, so that
** the function will act as you expect.
**
** (the existence of this function implies that I'll never have an airEnum
** of the same name, which would be the usual thing to do with a C enum,
** but I don't think an airEnum for this would be useful)
*/
int
nrrdHasNonExist(const Nrrd *nrrd) {
  NRRD_TYPE_BIGGEST _min, _max;
  int ret;

  if (nrrd
      && !airEnumValCheck(nrrdType, nrrd->type)
      && nrrdTypeBlock != nrrd->type) {
    if (nrrdTypeIsIntegral[nrrd->type]) {
      ret = nrrdHasNonExistFalse;
    } else {
      /* HEY: this could be optimized by being more specialized */
      nrrdMinMaxExactFind[nrrd->type](&_min, &_max, &ret, nrrd);
    }
  } else {
    ret = nrrdHasNonExistUnknown;
  }
  return ret;
}
