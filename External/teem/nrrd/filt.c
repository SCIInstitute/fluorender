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

int
_nrrdCM_median(const float *hist, float half) {
  float sum = 0;
  const float *hpt;

  hpt = hist;

  while(sum < half)
    sum += *hpt++;

  return AIR_CAST(int, hpt - 1 - hist);
}

int
_nrrdCM_mode(const float *hist, int bins) {
  float max;
  int i, mi;

  mi = -1;
  max = 0;
  for (i=0; i<bins; i++) {
    if (hist[i] && (!max || hist[i] > max) ) {
      max = hist[i];
      mi = i;
    }
  }

  return mi;
}

#define INDEX(nin, range, lup, idxIn, bins, val) ( \
  val = (lup)((nin)->data, (idxIn)), \
  airIndex((range)->min, (val), (range)->max, bins) \
)

void
_nrrdCM_printhist(const float *hist, int bins, const char *desc) {
  int i;

  printf("%s:\n", desc);
  for (i=0; i<bins; i++) {
    if (hist[i]) {
      printf("   %d: %g\n", i, hist[i]);
    }
  }
}

float *
_nrrdCM_wtAlloc(int radius, float wght) {
  float *wt, sum;
  int diam, r;

  diam = 2*radius + 1;
  wt = (float *)calloc(diam, sizeof(float));
  wt[radius] = 1.0;
  for (r=1; r<=radius; r++) {
    wt[radius+r] = AIR_CAST(float, pow(1.0/wght, r));
    wt[radius-r] = AIR_CAST(float, pow(1.0/wght, r));
  }
  sum = 0.0;
  for (r=0; r<diam; r++) {
    sum += wt[r];
  }
  for (r=0; r<diam; r++) {
    wt[r] /= sum;
  }
  /*
  for (r=0; r<diam; r++) {
    fprintf(stderr, "!%s: wt[%d] = %g\n", "_nrrdCM_wtAlloc", r, wt[r]);
  }
  */
  return wt;
}

void
_nrrdCheapMedian1D(Nrrd *nout, const Nrrd *nin, const NrrdRange *range,
                   int radius, float wght,
                   int bins, int mode, float *hist) {
  /* static const char me[]="_nrrdCheapMedian1D"; */
  size_t num;
  int X, I, idx, diam;
  float half, *wt;
  double val, (*lup)(const void *, size_t);

  diam = 2*radius + 1;
  lup = nrrdDLookup[nin->type];
  num = nrrdElementNumber(nin);
  if (1 == wght) {
    /* uniform weighting-> can do sliding histogram optimization */
    /* initialize histogram */
    half = AIR_CAST(float, diam/2 + 1);
    memset(hist, 0, bins*sizeof(float));
    for (X=0; X<diam; X++) {
      hist[INDEX(nin, range, lup, X, bins, val)]++;
    }
    /* _nrrdCM_printhist(hist, bins, "after init"); */
    /* find median at each point using existing histogram */
    for (X=radius; X<(int)num-radius; X++) {
      /* _nrrdCM_printhist(hist, bins, "----------"); */
      idx = mode ? _nrrdCM_mode(hist, bins) : _nrrdCM_median(hist, half);
      val = NRRD_NODE_POS(range->min, range->max, bins, idx);
      /* printf(" median idx = %d -> val = %g\n", idx, val); */
      nrrdDInsert[nout->type](nout->data, X, val);
      /* probably update histogram for next iteration */
      if (X < (int)num-radius-1) {
        hist[INDEX(nin, range, lup, X+radius+1, bins, val)]++;
        hist[INDEX(nin, range, lup, X-radius, bins, val)]--;
      }
    }
  } else {
    /* non-uniform weighting --> slow and stupid */
    wt = _nrrdCM_wtAlloc(radius, wght);
    half = 0.5;
    for (X=radius; X<(int)num-radius; X++) {
      memset(hist, 0, bins*sizeof(float));
      for (I=-radius; I<=radius; I++) {
        hist[INDEX(nin, range, lup, I+X, bins, val)] += wt[I+radius];
      }
      idx = mode ? _nrrdCM_mode(hist, bins) : _nrrdCM_median(hist, half);
      val = NRRD_NODE_POS(range->min, range->max, bins, idx);
      nrrdDInsert[nout->type](nout->data, X, val);
    }
    free(wt);
  }
}

void
_nrrdCheapMedian2D(Nrrd *nout, const Nrrd *nin, const NrrdRange *range,
                   int radius, float wght,
                   int bins, int mode, float *hist) {
  /* static const char me[]="_nrrdCheapMedian2D"; */
  int X, Y, I, J;
  int sx, sy, idx, diam;
  float half, *wt;
  double val, (*lup)(const void *, size_t);

  diam = 2*radius + 1;
  sx = AIR_CAST(unsigned int, nin->axis[0].size);
  sy = AIR_CAST(unsigned int, nin->axis[1].size);
  lup = nrrdDLookup[nin->type];
  if (1 == wght) {
    /* uniform weighting-> can do sliding histogram optimization */
    half = AIR_CAST(float, diam*diam/2 + 1);
    for (Y=radius; Y<sy-radius; Y++) {
      /* initialize histogram */
      memset(hist, 0, bins*sizeof(float));
      X = radius;
      for (J=-radius; J<=radius; J++) {
        for (I=-radius; I<=radius; I++) {
          hist[INDEX(nin, range, lup, X+I + sx*(J+Y), bins, val)]++;
        }
      }
      /* _nrrdCM_printhist(hist, bins, "after init"); */
      /* find median at each point using existing histogram */
      for (X=radius; X<sx-radius; X++) {
        idx = mode ? _nrrdCM_mode(hist, bins) : _nrrdCM_median(hist, half);
        val = NRRD_NODE_POS(range->min, range->max, bins, idx);
        nrrdDInsert[nout->type](nout->data, X + sx*Y, val);
        /* probably update histogram for next iteration */
        if (X < sx-radius-1) {
          for (J=-radius; J<=radius; J++) {
            hist[INDEX(nin, range, lup, X+radius+1 + sx*(J+Y), bins, val)]++;
            hist[INDEX(nin, range, lup, X-radius + sx*(J+Y), bins, val)]--;
          }
        }
      }
    }
  } else {
    /* non-uniform weighting --> slow and stupid */
    wt = _nrrdCM_wtAlloc(radius, wght);
    half = 0.5;
    for (Y=radius; Y<sy-radius; Y++) {
      for (X=radius; X<sx-radius; X++) {
        memset(hist, 0, bins*sizeof(float));
        for (J=-radius; J<=radius; J++) {
          for (I=-radius; I<=radius; I++) {
            hist[INDEX(nin, range, lup, I+X + sx*(J+Y),
                       bins, val)] += wt[I+radius]*wt[J+radius];
          }
        }
        idx = mode ? _nrrdCM_mode(hist, bins) : _nrrdCM_median(hist, half);
        val = NRRD_NODE_POS(range->min, range->max, bins, idx);
        nrrdDInsert[nout->type](nout->data, X + sx*Y, val);
      }
    }
    free(wt);
  }
}

void
_nrrdCheapMedian3D(Nrrd *nout, const Nrrd *nin, const NrrdRange *range,
                   int radius, float wght,
                   int bins, int mode, float *hist) {
  static const char me[]="_nrrdCheapMedian3D";
  char done[13];
  int X, Y, Z, I, J, K;
  int sx, sy, sz, idx, diam;
  float half, *wt;
  double val, (*lup)(const void *, size_t);

  diam = 2*radius + 1;
  sx = AIR_CAST(int, nin->axis[0].size);
  sy = AIR_CAST(int, nin->axis[1].size);
  sz = AIR_CAST(int, nin->axis[2].size);
  lup = nrrdDLookup[nin->type];
  fprintf(stderr, "%s: ...       ", me);
  if (1 == wght) {
    /* uniform weighting-> can do sliding histogram optimization */
    half = AIR_CAST(float, diam*diam*diam/2 + 1);
    fflush(stderr);
    for (Z=radius; Z<sz-radius; Z++) {
      fprintf(stderr, "%s", airDoneStr(radius, Z, sz-radius-1, done));
      fflush(stderr);
      for (Y=radius; Y<sy-radius; Y++) {
        /* initialize histogram */
        memset(hist, 0, bins*sizeof(float));
        X = radius;
        for (K=-radius; K<=radius; K++) {
          for (J=-radius; J<=radius; J++) {
            for (I=-radius; I<=radius; I++) {
              hist[INDEX(nin, range, lup, I+X + sx*(J+Y + sy*(K+Z)),
                         bins, val)]++;
            }
          }
        }
        /* find median at each point using existing histogram */
        for (X=radius; X<sx-radius; X++) {
          idx = mode ? _nrrdCM_mode(hist, bins) : _nrrdCM_median(hist, half);
          val = NRRD_NODE_POS(range->min, range->max, bins, idx);
          nrrdDInsert[nout->type](nout->data, X + sx*(Y + sy*Z), val);
          /* probably update histogram for next iteration */
          if (X < sx-radius-1) {
            for (K=-radius; K<=radius; K++) {
              for (J=-radius; J<=radius; J++) {
                hist[INDEX(nin, range, lup, X+radius+1 + sx*(J+Y + sy*(K+Z)),
                           bins, val)]++;
                hist[INDEX(nin, range, lup, X-radius + sx*(J+Y + sy*(K+Z)),
                           bins, val)]--;
              }
            }
          }
        }
      }
    }
  } else {
    /* non-uniform weighting --> slow and stupid */
    wt = _nrrdCM_wtAlloc(radius, wght);
    half = 0.5;
    for (Z=radius; Z<sz-radius; Z++) {
      fprintf(stderr, "%s", airDoneStr(radius, Z, sz-radius-1, done));
      fflush(stderr);
      for (Y=radius; Y<sy-radius; Y++) {
        for (X=radius; X<sx-radius; X++) {
          memset(hist, 0, bins*sizeof(float));
          for (K=-radius; K<=radius; K++) {
            for (J=-radius; J<=radius; J++) {
              for (I=-radius; I<=radius; I++) {
                hist[INDEX(nin, range, lup, I+X + sx*(J+Y + sy*(K+Z)),
                           bins, val)]
                  += wt[I+radius]*wt[J+radius]*wt[K+radius];
              }
            }
          }
          idx = mode ? _nrrdCM_mode(hist, bins) : _nrrdCM_median(hist, half);
          val = NRRD_NODE_POS(range->min, range->max, bins, idx);
          nrrdDInsert[nout->type](nout->data, X + sx*(Y + sy*Z), val);
        }
      }
    }
    free(wt);
  }
  fprintf(stderr, "\b\b\b\b\b\b  done\n");
}

/*
******** nrrdCheapMedian
**
** histogram-based median or mode filtering
** !mode: median filtering
** mode: mode filtering
*/
int
nrrdCheapMedian(Nrrd *_nout, const Nrrd *_nin,
                int pad, int mode,
                unsigned int radius, float wght, unsigned int bins) {
  static const char me[]="nrrdCheapMedian", func[]="cmedian";
  NrrdRange *range;
  float *hist;
  Nrrd *nout, *nin;
  airArray *mop;
  unsigned int minsize;

  if (!(_nin && _nout)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(radius >= 1)) {
    biffAddf(NRRD, "%s: need radius >= 1 (got %d)", me, radius);
    return 1;
  }
  if (!(bins >= 1)) {
    biffAddf(NRRD, "%s: need bins >= 1 (got %d)", me, bins);
    return 1;
  }
  if (!(AIR_IN_CL(1, _nin->dim, 3))) {
    biffAddf(NRRD, "%s: sorry, can only handle dim 1, 2, 3 (not %d)",
             me, _nin->dim);
    return 1;
  }
  minsize = AIR_CAST(unsigned int, _nin->axis[0].size);
  if (_nin->dim > 1) {
    minsize = AIR_MIN(minsize, AIR_CAST(unsigned int, _nin->axis[1].size));
  }
  if (_nin->dim > 2) {
    minsize = AIR_MIN(minsize, AIR_CAST(unsigned int, _nin->axis[2].size));
  }
  if (!pad && minsize < 2*radius+1) {
    biffAddf(NRRD, "%s: minimum nrrd size (%d) smaller than filtering "
             "window size (%d) with radius %d; must enable padding", me,
             minsize, 2*radius+1, radius);
    return 1;
  }
  if (_nout == _nin) {
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  if (nrrdTypeBlock == _nin->type) {
    biffAddf(NRRD, "%s: can't filter nrrd type %s", me,
             airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }

  mop = airMopNew();
  /* set nin based on _nin */
  airMopAdd(mop, nin=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (pad) {
    airMopAdd(mop, nout=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdSimplePad_va(nin, _nin, radius, nrrdBoundaryBleed)) {
      biffAddf(NRRD, "%s: trouble padding input", me);
      airMopError(mop); return 1;
    }
  } else {
    if (nrrdCopy(nin, _nin)) {
      biffAddf(NRRD, "%s: trouble copying input", me);
      airMopError(mop); return 1;
    }
    nout = _nout;
  }
  if (nrrdCopy(nout, nin)) {
    biffAddf(NRRD, "%s: failed to create initial copy of input", me);
    airMopError(mop); return 1;
  }
  range = nrrdRangeNewSet(nin, nrrdBlind8BitRangeFalse);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  if (!(hist = (float*)calloc(bins, sizeof(float)))) {
    biffAddf(NRRD, "%s: couldn't allocate histogram (%d bins)", me, bins);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, hist, airFree, airMopAlways);
  if (!AIR_EXISTS(wght)) {
    wght = 1.0;
  }
  switch (nin->dim) {
  case 1:
    _nrrdCheapMedian1D(nout, nin, range, radius, wght, bins, mode, hist);
    break;
  case 2:
    _nrrdCheapMedian2D(nout, nin, range, radius, wght, bins, mode, hist);
    break;
  case 3:
    _nrrdCheapMedian3D(nout, nin, range, radius, wght, bins, mode, hist);
    break;
  default:
    biffAddf(NRRD, "%s: sorry, %d-dimensional median unimplemented",
             me, nin->dim);
    airMopError(mop); return 1;
  }

  nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
  if (nrrdContentSet_va(nout, func, nin, "%d,%d,%g,%d",
                        mode, radius, wght, bins)) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  /* basic info handled by nrrdCopy above */

  /* set _nout based on nout */
  if (pad) {
    if (nrrdSimpleCrop(_nout, nout, radius)) {
      biffAddf(NRRD, "%s: trouble cropping output", me);
      airMopError(mop); return 1;
    }
  } else {
    /* we've already set output in _nout == nout */
  }

  airMopOkay(mop);
  return 0;
}

/*
** returns intersection of parabolas c(x) = spc^2 (x - xi)^2 + yi
*/
static double
intx(double xx0, double yy0, double xx1, double yy1, double spc) {
  double ss;

  ss = spc*spc;
  return (yy1/ss + xx1*xx1 - (yy0/ss + xx0*xx0))/(2*(xx1 - xx0));
}

/*
** squared-distance transform of single scanline
**
** based on code published with:
** Distance Transforms of Sampled Functions
** Pedro F. Felzenszwalb and Daniel P. Huttenlocher
** Cornell Computing and Information Science TR2004-1963
**
** dd: output (pre-allocated for "len")
** ff: input function (pre-allocated for "len")
** zz: buffer (pre-allocated for "len"+1) for locations of
**     boundaries between parabolas
** vv: buffer (pre-allocated for "len") for locations of
**     parabolas in lower envelope
**
** The major modification from the published method is the inclusion
** of the "spc" parameter that gives the inter-sample spacing, so that
** the multi-dimensional version can work on non-isotropic samples.
**
** FLT_MIN and FLT_MAX are used to be consistent with the
** initialization in nrrdDistanceL2(), which uses FLT_MAX
** to be compatible with the case of using floats
*/
static void
distanceL2Sqrd1D(double *dd, const double *ff,
                 double *zz, unsigned int *vv,
                 size_t len, double spc) {
  size_t kk, qq;

  if (!( dd && ff && zz && vv && len > 0 )) {
    /* error */
    return;
  }

  kk = 0;
  vv[0] = 0;
  zz[0] = -FLT_MAX;
  zz[1] = +FLT_MAX;
  for (qq=1; qq<len; qq++) {
    double ss;
    ss = intx(AIR_CAST(double, qq), ff[qq], vv[kk], ff[vv[kk]], spc);
    /* while (ss <= zz[kk]) {
    ** HEY this can have kk going to -1 and into memory errors!
    */
    while (ss <= zz[kk] && kk) {
      kk--;
      ss = intx(AIR_CAST(double, qq), ff[qq], vv[kk], ff[vv[kk]], spc);
    }
    kk++;
    vv[kk] = AIR_CAST(unsigned int, qq);
    zz[kk] = ss;
    zz[kk+1] = +FLT_MAX;
  }

  kk = 0;
  for (qq=00; qq<len; qq++) {
    double dx;
    while (zz[kk+1] < qq) {
      kk++;
    }
    /* cast to avoid overflow weirdness on the unsigned ints */
    dx = AIR_CAST(double, qq) - vv[kk];
    dd[qq] = spc*spc*dx*dx + ff[vv[kk]];
  }

  return;
}

static int
distanceL2Sqrd(Nrrd *ndist, double *spcMean) {
  static const char me[]="distanceL2Sqrd";
  size_t sizeMax;           /* max size of all axes */
  Nrrd *ntmpA, *ntmpB, *npass[NRRD_DIM_MAX+1];
  int spcSomeExist, spcSomeNonExist;
  unsigned int di, *vv;
  double *dd, *ff, *zz;
  double spc[NRRD_DIM_MAX], vector[NRRD_SPACE_DIM_MAX];
  double (*lup)(const void *, size_t), (*ins)(void *, size_t, double);
  airArray *mop;

  if (!( nrrdTypeFloat == ndist->type || nrrdTypeDouble == ndist->type )) {
    /* MWC: This error condition was/is being ignored. */
    /*    biffAddf(NRRD, "%s: sorry, can only process type %s or %s (not %s)",
                  me,
                  airEnumStr(nrrdType, nrrdTypeFloat),
                  airEnumStr(nrrdType, nrrdTypeDouble),
                  airEnumStr(nrrdType, ndist->type));
    */
  }

  spcSomeExist = AIR_FALSE;
  spcSomeNonExist = AIR_FALSE;
  for (di=0; di<ndist->dim; di++) {
    nrrdSpacingCalculate(ndist, di, spc + di, vector);
    spcSomeExist |= AIR_EXISTS(spc[di]);
    spcSomeNonExist |= !AIR_EXISTS(spc[di]);
  }
  if (spcSomeExist && spcSomeNonExist) {
    biffAddf(NRRD, "%s: axis spacings must all exist or all non-exist", me);
    return 1;
  }
  if (!spcSomeExist) {
    for (di=0; di<ndist->dim; di++) {
      spc[di] = 1.0;
    }
  }
  *spcMean = 0;
  for (di=0; di<ndist->dim; di++) {
    *spcMean += spc[di];
  }
  *spcMean /= ndist->dim;

  sizeMax = 0;
  for (di=0; di<ndist->dim; di++) {
    sizeMax = AIR_MAX(sizeMax, ndist->axis[di].size);
  }

  /* create mop and allocate tmp buffers */
  mop = airMopNew();
  ntmpA = nrrdNew();
  airMopAdd(mop, ntmpA, (airMopper)nrrdNuke, airMopAlways);
  if (ndist->dim > 2) {
    ntmpB = nrrdNew();
    airMopAdd(mop, ntmpB, (airMopper)nrrdNuke, airMopAlways);
  } else {
    ntmpB = NULL;
  }
  if (nrrdCopy(ntmpA, ndist)
      || (ndist->dim > 2 && nrrdCopy(ntmpB, ndist))) {
    biffAddf(NRRD, "%s: couldn't allocate image buffers", me);
  }
  dd = AIR_CAST(double *, calloc(sizeMax, sizeof(double)));
  ff = AIR_CAST(double *, calloc(sizeMax, sizeof(double)));
  zz = AIR_CAST(double *, calloc(sizeMax+1, sizeof(double)));
  vv = AIR_CAST(unsigned int *, calloc(sizeMax, sizeof(unsigned int)));
  airMopAdd(mop, dd, airFree, airMopAlways);
  airMopAdd(mop, ff, airFree, airMopAlways);
  airMopAdd(mop, zz, airFree, airMopAlways);
  airMopAdd(mop, vv, airFree, airMopAlways);
  if (!( dd && ff && zz && vv )) {
    /* MWC: This error condition was/is being ignored. */
    /* biffAddf(NRRD, "%s: couldn't allocate scanline buffers", me); */
  }

  /* set up array of buffers */
  npass[0] = ndist;
  for (di=1; di<ndist->dim; di++) {
    npass[di] = (di % 2) ? ntmpA : ntmpB;
  }
  npass[ndist->dim] = ndist;

  /* run the multiple passes */
  /* what makes the indexing here so simple is that by assuming that
     we're processing every axis, the input to any given pass can be
     logically considered a 2-D image (a list of scanlines), where the
     second axis is the merge of all input axes but the first.  With
     the rotational shuffle of axes through passes, the initial axis
     and the set of other axes swap places, so its like the 2-D image
     is being transposed.  NOTE: the Nrrds that were allocated as
     buffers are really being mis-used, in that the axis sizes and
     raster ordering of what we're storing there is *not* the same as
     told by axis[].size */
  lup = nrrdDLookup[ndist->type];
  ins = nrrdDInsert[ndist->type];
  for (di=0; di<ndist->dim; di++) {
    size_t lineIdx, lineNum, valIdx, valNum;

    valNum = ndist->axis[di].size;
    lineNum = nrrdElementNumber(ndist)/valNum;
    for (lineIdx=0; lineIdx<lineNum; lineIdx++) {
      /* read input scanline into ff */
      for (valIdx=0; valIdx<valNum; valIdx++) {
        ff[valIdx] = lup(npass[di]->data, valIdx + valNum*lineIdx);
      }
      /* do the transform */
      distanceL2Sqrd1D(dd, ff, zz, vv, valNum, spc[di]);
      /* write dd to output scanline */
      for (valIdx=0; valIdx<valNum; valIdx++) {
        ins(npass[di+1]->data, lineIdx + lineNum*valIdx, dd[valIdx]);
      }
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
** helper function for distance transforms, is called by things that want to do
** specific kinds of transforms.
*/
static int
_distanceBase(Nrrd *nout, const Nrrd *nin,
              int typeOut, const int *axisDo,
              double thresh, double bias, int insideHigher) {
  static const char me[]="_distanceBase";
  size_t ii, nn;
  double (*lup)(const void *, size_t), (*ins)(void *, size_t, double);
  double spcMean;

  if (!( nout && nin )) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    biffAddf(NRRD, "%s: need scalar type for distance transform (not %s)",
             me, airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }
  if (!( nrrdTypeDouble == typeOut || nrrdTypeFloat == typeOut )) {
    biffAddf(NRRD, "%s: sorry, can only transform to type %s or %s "
             "(not %s)", me,
             airEnumStr(nrrdType, nrrdTypeFloat),
             airEnumStr(nrrdType, nrrdTypeDouble),
             airEnumStr(nrrdType, typeOut));
    return 1;
  }
  if (axisDo) {
    biffAddf(NRRD, "%s: sorry, selective axis transform not implemented",
             me);
    return 1;
  }
  if (!AIR_EXISTS(thresh)) {
    biffAddf(NRRD, "%s: threshold (%g) doesn't exist", me, thresh);
    return 1;
  }

  if (nrrdConvert(nout, nin, typeOut)) {
    biffAddf(NRRD, "%s: couldn't allocate output", me);
    return 1;
  }
  lup = nrrdDLookup[nout->type];
  ins = nrrdDInsert[nout->type];

  nn = nrrdElementNumber(nout);
  for (ii=0; ii<nn; ii++) {
    double val, bb;
    val = lup(nout->data, ii);
    if (insideHigher) {
      bb = bias*(val - thresh);
      ins(nout->data, ii, val > thresh ? bb*bb : FLT_MAX);
    } else {
      bb = bias*(thresh - val);
      ins(nout->data, ii, val <= thresh ? bb*bb : FLT_MAX);
    }
  }

  if (distanceL2Sqrd(nout, &spcMean)) {
    biffAddf(NRRD, "%s: trouble doing transform", me);
    return 1;
  }

  for (ii=0; ii<nn; ii++) {
    double val;
    val = sqrt(lup(nout->data, ii));
    /* here's where the distance is tweaked downwards by half a sample width */
    ins(nout->data, ii, AIR_MAX(0, val - spcMean/2));
  }

  return 0;
}

/*
******** nrrdDistanceL2
**
** computes euclidean (L2) distance transform of input image, after
** thresholding at "thresh".
**
** NOTE: the output of this is slightly offset from what one might
** expect; decreased by half of the average (over all axes) sample
** spacing.  The reason for this is so that when the transform is
** applied to the inverted image and negated, to create a full
** signed distance map, the transition from interior to exterior
** distance values is smooth.  Without this trick, there is a
** small little plateau at the transition.
*/
int
nrrdDistanceL2(Nrrd *nout, const Nrrd *nin,
               int typeOut, const int *axisDo,
               double thresh, int insideHigher) {
  static const char me[]="nrrdDistanceL2";

  if (_distanceBase(nout, nin, typeOut, axisDo, thresh, 0, insideHigher)) {
    biffAddf(NRRD, "%s: trouble doing distance transform", me);
    return 1;
  }
  return 0;
}

int
nrrdDistanceL2Biased(Nrrd *nout, const Nrrd *nin,
                     int typeOut, const int *axisDo,
                     double thresh, double bias, int insideHigher) {
  static const char me[]="nrrdDistanceL2Biased";

  if (_distanceBase(nout, nin, typeOut, axisDo, thresh, bias, insideHigher)) {
    biffAddf(NRRD, "%s: trouble doing distance transform", me);
    return 1;
  }
  return 0;
}

int
nrrdDistanceL2Signed(Nrrd *nout, const Nrrd *nin,
                     int typeOut, const int *axisDo,
                     double thresh, int insideHigher) {
  static const char me[]="nrrdDistanceL2Signed";
  airArray *mop;
  Nrrd *ninv;

  if (!(nout && nin)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }

  mop = airMopNew();
  ninv = nrrdNew();
  airMopAdd(mop, ninv, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdDistanceL2(nout, nin, typeOut, axisDo, thresh, insideHigher)
      || nrrdDistanceL2(ninv, nin, typeOut, axisDo, thresh, !insideHigher)
      || nrrdArithUnaryOp(ninv, nrrdUnaryOpNegative, ninv)
      || nrrdArithBinaryOp(nout, nrrdBinaryOpAdd, nout, ninv)) {
    biffAddf(NRRD, "%s: trouble doing or combining transforms", me);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
