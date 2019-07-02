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
** This is a largely a re-write of the functionality in
** nrrdSpatialResample(), but with some improvements.  The big API
** change is that everything happens in a nrrdResampleContext, and no
** fields in this need to be directly set (except for rsmc->verbose).
**
** The big behavior/API change is that the range along the axis that
** is resampled is now defined in terms of index space, instead of
** axis-aligned scaled index space (what used to be called "world
** space", prior to general orientation).  This means that if you
** want the whole range resampled, you use [-0.5,size-0.5] for cell-
** centered, and [0,size-1] for node-centered.  One function helpful
** dealing with this is nrrdResampleRangeFullSet().
**
** Other improvements:
** -- ability to quickly resample a different nrrd with the same
**    sizes and kernels as with a previous (all useful state and
**    allocations of intermediate resampling results is preserved
**    in the nrrdResampleContext).  To resample a second nrrd,
**    you'd only need:
**    nrrdResampleInputSet(rsmc, nin2);
**    nrrdResampleExecute(rsmc, nout2);
** -- correct handling general orientation (space directions and
**    space origin).  This was impossible in the old resampler
**    because of how its logic was hard-wired to the axis-aligned
**    world space defined by the per-axis min and max.
** -- correct handling of "cheap" downsampling, with the use of
**    the new nrrdKernelCheap
** -- smaller memory footprint (smarter about freeing intermediate
**    resampling results)
*/

enum {
  flagUnknown,          /*  0 */
  flagDefaultCenter,    /*  1 */
  flagInput,            /*  2 */
  flagOverrideCenters,  /*  3 */
  flagInputDimension,   /*  4 */
  flagInputCenters,     /*  5 */
  flagInputSizes,       /*  6 */
  flagKernels,          /*  7 */
  flagSamples,          /*  8 */
  flagRanges,           /*  9 */
  flagBoundary,         /* 10 */
  flagLineAllocate,     /* 11 */
  flagLineFill,         /* 12 */
  flagVectorAllocate,   /* 13 */
  flagPermutation,      /* 14 */
  flagVectorFill,       /* 15 */
  flagClamp,            /* 16 */
  flagRound,            /* 17 */
  flagTypeOut,          /* 18 */
  flagPadValue,         /* 19 */
  flagRenormalize,      /* 20 */
  flagNonExistent,      /* 21 */
  flagLast
};
#define FLAG_MAX           21

void
nrrdResampleContextInit(NrrdResampleContext *rsmc) {
  unsigned int axIdx, axJdx, kpIdx, flagIdx;
  NrrdResampleAxis *axis;

  if (rsmc) {
    rsmc->nin = NULL;
    rsmc->boundary = nrrdDefaultResampleBoundary;
    rsmc->typeOut = nrrdDefaultResampleType;
    rsmc->renormalize = nrrdDefaultResampleRenormalize;
    rsmc->roundlast = nrrdDefaultResampleRound;
    rsmc->clamp = nrrdDefaultResampleClamp;
    rsmc->defaultCenter = nrrdDefaultCenter;
    rsmc->nonExistent = nrrdDefaultResampleNonExistent;
    rsmc->padValue = nrrdDefaultResamplePadValue;
    rsmc->dim = 0;
    rsmc->passNum = AIR_CAST(unsigned int, -1); /* 4294967295 */
    rsmc->topRax = AIR_CAST(unsigned int, -1);
    rsmc->botRax = AIR_CAST(unsigned int, -1);
    for (axIdx=0; axIdx<NRRD_DIM_MAX; axIdx++) {
      rsmc->permute[axIdx] = AIR_CAST(unsigned int, -1);
      rsmc->passAxis[axIdx] = AIR_CAST(unsigned int, -1);
    }
    for (axIdx=0; axIdx<NRRD_DIM_MAX+1; axIdx++) {
      axis = rsmc->axis + axIdx;
      axis->kernel = NULL;
      axis->kparm[0] = nrrdDefaultKernelParm0;
      for (kpIdx=1; kpIdx<NRRD_KERNEL_PARMS_NUM; kpIdx++) {
        axis->kparm[kpIdx] = AIR_NAN;
      }
      axis->min = axis->max = AIR_NAN;
      axis->samples = AIR_CAST(unsigned int, -1);
      axis->overrideCenter = nrrdCenterUnknown;
      axis->center = nrrdCenterUnknown;
      axis->sizeIn = AIR_CAST(unsigned int, -1);
      axis->axIdx = axIdx;                         /* never changes */
      axis->passIdx = AIR_CAST(unsigned int, -1);
      for (axJdx=0; axJdx<NRRD_DIM_MAX; axJdx++) {
        axis->sizePerm[axJdx] = AIR_CAST(size_t, -1);
        axis->axisPerm[axJdx] = AIR_CAST(unsigned int, -1);
      }
      axis->ratio = AIR_NAN;
      axis->nrsmp = NULL;    /* these are nrrdNew()'d as needed */
      axis->nline = nrrdNew();
      axis->nindex = nrrdNew();
      axis->nweight = nrrdNew();
    }
    /* initialize flags to all true */
    for (flagIdx=0; flagIdx<=FLAG_MAX; flagIdx++) {
      rsmc->flag[flagIdx] = AIR_TRUE;
    }
    rsmc->time = 0.0;
  }
  return;
}

NrrdResampleContext *
nrrdResampleContextNew() {
  NrrdResampleContext *rsmc;

  rsmc = AIR_CALLOC(1, NrrdResampleContext);
  if (rsmc) {
    rsmc->flag = AIR_CALLOC(1+FLAG_MAX, int);
    nrrdResampleContextInit(rsmc);
  }
  return rsmc;
}

NrrdResampleContext *
nrrdResampleContextNix(NrrdResampleContext *rsmc) {
  unsigned int axIdx;

  if (rsmc) {
    for (axIdx=0; axIdx<NRRD_DIM_MAX+1; axIdx++) {
      /* nrsmp should have been cleaned up by _nrrdResampleOutputUpdate() */
      nrrdNuke(rsmc->axis[axIdx].nline);
      nrrdNuke(rsmc->axis[axIdx].nindex);
      nrrdNuke(rsmc->axis[axIdx].nweight);
    }
    airFree(rsmc->flag);
    airFree(rsmc);
  }
  return NULL;
}

int
nrrdResampleDefaultCenterSet(NrrdResampleContext *rsmc,
                             int center) {
  static const char me[]="nrrdResampleDefaultCenterSet";

  if (!( rsmc )) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(nrrdCenterNode == center
        || nrrdCenterCell == center)) {
    biffAddf(NRRD, "%s: got invalid center (%d)", me, center);
    return 1;
  }

  if (center != rsmc->defaultCenter) {
    rsmc->defaultCenter = center;
    rsmc->flag[flagDefaultCenter] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResampleNonExistentSet(NrrdResampleContext *rsmc,
                           int nonExist) {
  static const char me[]="nrrdResampleNonExistentSet";

  if (!( rsmc )) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdResampleNonExistent, nonExist)) {
    biffAddf(NRRD, "%s: didn't get valid non-existent behavior (%d)",
             me, nonExist);
    return 1;
  }

  if (nonExist != rsmc->nonExistent) {
    rsmc->nonExistent = nonExist;
    rsmc->flag[flagNonExistent] = AIR_TRUE;
  }
  return 0;
}

#define NRRD_RESAMPLE_INPUT_SET_BODY \
  unsigned int axIdx, kpIdx; \
\
  if (!( rsmc && nin )) { \
    biffAddf(NRRD, "%s: got NULL pointer", me); \
    return 1; \
  } \
  if (nrrdCheck(nin)) { \
    biffAddf(NRRD, "%s: problems with given nrrd", me); \
    return 1; \
  } \
  if (nrrdTypeBlock == nin->type) { \
    biffAddf(NRRD, "%s: can't resample from type %s", me, \
             airEnumStr(nrrdType, nrrdTypeBlock)); \
    return 1; \
  } \
 \
  rsmc->nin = nin; \
  rsmc->flag[flagInput] = AIR_TRUE; \
 \
  /* per-axis information should be invalidated at this point, because \
     if we defer the invalidation to later ...Update() calls, it will \
     clobber the effects of intervening calls to the likes of  \
     ...KernelSet(), ...SampleSet(), and so on */ \
  if (rsmc->dim != nin->dim) { \
    for (axIdx=0; axIdx<NRRD_DIM_MAX; axIdx++) { \
      rsmc->axis[axIdx].center = nrrdCenterUnknown; \
      rsmc->axis[axIdx].sizeIn = 0; \
      rsmc->axis[axIdx].kernel = NULL; \
      rsmc->axis[axIdx].kparm[0] = nrrdDefaultKernelParm0; \
      for (kpIdx=1; kpIdx<NRRD_KERNEL_PARMS_NUM; kpIdx++) { \
        rsmc->axis[axIdx].kparm[kpIdx] = AIR_NAN; \
      } \
      rsmc->axis[axIdx].samples = 0; \
      rsmc->axis[axIdx].min = AIR_NAN; \
      rsmc->axis[axIdx].max = AIR_NAN; \
    } \
  } \
 \
  return 0


int
nrrdResampleNrrdSet(NrrdResampleContext *rsmc, const Nrrd *nin) {
  static const char me[]="nrrdResampleNrrdSet";

  NRRD_RESAMPLE_INPUT_SET_BODY;
}

int
nrrdResampleInputSet(NrrdResampleContext *rsmc, const Nrrd *nin) {
  static const char me[]="nrrdResampleInputSet";

  NRRD_RESAMPLE_INPUT_SET_BODY;
}

#define PER_AXIS_ERROR_CHECK                                            \
  if (!rsmc) {                                                          \
    biffAddf(NRRD, "%s: got NULL pointer", me);                         \
    return 1;                                                           \
  }                                                                     \
  if (!rsmc->nin) {                                                     \
    biffAddf(NRRD, "%s: haven't set input nrrd yet", me);               \
    return 1;                                                           \
  }                                                                     \
  if (!( axIdx < rsmc->nin->dim )) {                                    \
    biffAddf(NRRD, "%s: axis %u >= nin->dim %u",                        \
             me, axIdx, rsmc->nin->dim);                                \
    return 1;                                                           \
  }

int
nrrdResampleKernelSet(NrrdResampleContext *rsmc, unsigned int axIdx,
                      const NrrdKernel *kernel,
                      double kparm[NRRD_KERNEL_PARMS_NUM]) {
  static const char me[]="nrrdResampleKernelSet";
  unsigned int kpIdx;

  PER_AXIS_ERROR_CHECK;

  rsmc->axis[axIdx].kernel = kernel;
  if (kernel) {
    for (kpIdx=0; kpIdx<kernel->numParm; kpIdx++) {
      rsmc->axis[axIdx].kparm[kpIdx] = kparm[kpIdx];
    }
    if (rsmc->verbose) {
      char kstr[AIR_STRLEN_LARGE];
      NrrdKernelSpec ksp;
      nrrdKernelSpecSet(&ksp, rsmc->axis[axIdx].kernel,
                        rsmc->axis[axIdx].kparm);
      nrrdKernelSpecSprint(kstr, &ksp);
      fprintf(stderr, "%s: axis %u kernel %s\n", me, axIdx, kstr);
    }
  }
  rsmc->flag[flagKernels] = AIR_TRUE;

  return 0;
}

int
nrrdResampleSamplesSet(NrrdResampleContext *rsmc,
                       unsigned int axIdx,
                       size_t samples) {
  static const char me[]="nrrdResampleSamplesSet";

  PER_AXIS_ERROR_CHECK;

  if (rsmc->axis[axIdx].samples != samples) {
    if (rsmc->verbose) {
      fprintf(stderr, "%s: axis %u samples %u --> %u\n", me, axIdx,
              AIR_CAST(unsigned int, rsmc->axis[axIdx].samples),
              AIR_CAST(unsigned int, samples));
    }
    rsmc->axis[axIdx].samples = samples;
    rsmc->flag[flagSamples] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResampleRangeSet(NrrdResampleContext *rsmc,
                     unsigned int axIdx,
                     double min, double max) {
  static const char me[]="nrrdResampleRangeSet";

  PER_AXIS_ERROR_CHECK;
  if (!(AIR_EXISTS(min) && AIR_EXISTS(max) && min != max)) {
    biffAddf(NRRD, "%s: need min != max and both to exist", me);
    return 1;
  }
  if (!(rsmc->axis[axIdx].min == min
        && rsmc->axis[axIdx].max == max)) {
    rsmc->axis[axIdx].min = min;
    rsmc->axis[axIdx].max = max;
    rsmc->flag[flagRanges] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResampleOverrideCenterSet(NrrdResampleContext *rsmc,
                              unsigned int axIdx,
                              int center) {
  static const char me[]="nrrdResampleOverrideCenterSet";

  PER_AXIS_ERROR_CHECK;
  if (center) {
    /* we do allow passing nrrdCenterUnknown, to turn off override */
    if (airEnumValCheck(nrrdCenter, center)) {
      biffAddf(NRRD, "%s: didn't get valid centering (%d)", me, center);
      return 1;
    }
  }
  if (center != rsmc->axis[axIdx].overrideCenter) {
    rsmc->axis[axIdx].overrideCenter = center;
    rsmc->flag[flagOverrideCenters] = AIR_TRUE;
  }

  return 0;
}

void
_nrrdResampleMinMaxFull(double *minP, double *maxP,
                        int center, size_t size) {
  if (nrrdCenterCell == center) {
    *minP = -0.5;
    *maxP = size - 0.5;
  } else {
    *minP = 0.0;
    *maxP = size - 1.0;
  }
}

int
nrrdResampleRangeFullSet(NrrdResampleContext *rsmc,
                         unsigned int axIdx) {
  static const char me[]="nrrdResampleRangeFullSet";
  double min, max;
  int center;

  PER_AXIS_ERROR_CHECK;

  /* HEY trick is to figure out the axis's centering, and to
     make sure its the same code as used elsewhere */
  center = (rsmc->axis[axIdx].overrideCenter
            ? rsmc->axis[axIdx].overrideCenter
            : (rsmc->nin->axis[axIdx].center
               ? rsmc->nin->axis[axIdx].center
               : rsmc->defaultCenter));
  _nrrdResampleMinMaxFull(&min, &max, center, rsmc->nin->axis[axIdx].size);
  if (!(rsmc->axis[axIdx].min == min
        && rsmc->axis[axIdx].max == max)) {
    rsmc->axis[axIdx].min = min;
    rsmc->axis[axIdx].max = max;
    rsmc->flag[flagRanges] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResampleBoundarySet(NrrdResampleContext *rsmc,
                        int boundary) {
  static const char me[]="nrrdResampleBoundarySet";

  if (!rsmc) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdBoundary, boundary)) {
    biffAddf(NRRD, "%s: invalid boundary %d", me, boundary);
    return 1;
  }

  if (rsmc->boundary != boundary) {
    rsmc->boundary = boundary;
    rsmc->flag[flagBoundary] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResamplePadValueSet(NrrdResampleContext *rsmc,
                        double padValue) {
  static const char me[]="nrrdResamplePadValueSet";

  if (!rsmc) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }

  if (rsmc->padValue != padValue) {
    rsmc->padValue = padValue;
    rsmc->flag[flagPadValue] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResampleRenormalizeSet(NrrdResampleContext *rsmc,
                           int renormalize) {
  static const char me[]="nrrdResampleRenormalizeSet";

  if (!rsmc) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }

  if (rsmc->renormalize != renormalize) {
    rsmc->renormalize = renormalize;
    rsmc->flag[flagRenormalize] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResampleTypeOutSet(NrrdResampleContext *rsmc,
                       int type) {
  static const char me[]="nrrdResampleTypeOutSet";

  if (!rsmc) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdTypeDefault != type && airEnumValCheck(nrrdType, type)) {
    biffAddf(NRRD, "%s: invalid type %d", me, type);
    return 1;
  }
  if (nrrdTypeBlock == type) {
    biffAddf(NRRD, "%s: can't output %s type", me,
             airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }

  if (rsmc->typeOut != type) {
    rsmc->typeOut = type;
    rsmc->flag[flagTypeOut] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResampleRoundSet(NrrdResampleContext *rsmc,
                     int roundlast) {
  static const char me[]="nrrdResampleRoundSet";

  if (!rsmc) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }

  if (rsmc->roundlast != roundlast) {
    rsmc->roundlast = roundlast;
    rsmc->flag[flagRound] = AIR_TRUE;
  }

  return 0;
}

int
nrrdResampleClampSet(NrrdResampleContext *rsmc,
                     int clamp) {
  static const char me[]="nrrdResampleClampSet";

  if (!rsmc) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }

  if (rsmc->clamp != clamp) {
    rsmc->clamp = clamp;
    rsmc->flag[flagClamp] = AIR_TRUE;
  }

  return 0;
}

int
_nrrdResampleInputDimensionUpdate(NrrdResampleContext *rsmc) {

  if (rsmc->flag[flagInput]) {
    if (rsmc->dim != rsmc->nin->dim) {
      rsmc->dim = rsmc->nin->dim;
      rsmc->flag[flagInputDimension] = AIR_TRUE;
    }
  }
  return 0;
}

int
_nrrdResampleInputCentersUpdate(NrrdResampleContext *rsmc) {
  unsigned int axIdx;
  int center;

  if (rsmc->flag[flagOverrideCenters]
      || rsmc->flag[flagDefaultCenter]
      || rsmc->flag[flagInputDimension]
      || rsmc->flag[flagInput]) {
    for (axIdx=0; axIdx<NRRD_DIM_MAX; axIdx++) {
      center = (rsmc->axis[axIdx].overrideCenter
                ? rsmc->axis[axIdx].overrideCenter
                : (rsmc->nin->axis[axIdx].center
                   ? rsmc->nin->axis[axIdx].center
                   : rsmc->defaultCenter));
      if (rsmc->axis[axIdx].center != center) {
        rsmc->axis[axIdx].center = center;
        rsmc->flag[flagInputCenters] = AIR_TRUE;
      }
    }
    rsmc->flag[flagOverrideCenters] = AIR_FALSE;
    rsmc->flag[flagDefaultCenter] = AIR_FALSE;
  }

  return 0;
}

int
_nrrdResampleInputSizesUpdate(NrrdResampleContext *rsmc) {
  unsigned int axIdx;

  if (rsmc->flag[flagInputDimension]
      || rsmc->flag[flagInput]) {
    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      if (rsmc->axis[axIdx].sizeIn != rsmc->nin->axis[axIdx].size) {
        rsmc->axis[axIdx].sizeIn = rsmc->nin->axis[axIdx].size;
        rsmc->flag[flagInputSizes] = AIR_TRUE;
      }
    }
    rsmc->flag[flagInputDimension] = AIR_FALSE;
  }

  return 0;
}

int
_nrrdResampleLineAllocateUpdate(NrrdResampleContext *rsmc) {
  static const char me[]="_nrrdResampleLineAllocateUpdate";
  unsigned int axIdx;
  NrrdResampleAxis *axis;

  if (rsmc->flag[flagInputSizes]
      || rsmc->flag[flagKernels]) {
    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      axis = rsmc->axis + axIdx;
      if (!axis->kernel) {
        nrrdEmpty(axis->nline);
      } else {
        if (nrrdMaybeAlloc_va(axis->nline, nrrdResample_nt, 1,
                              AIR_CAST(size_t, 1 + axis->sizeIn))) {
          biffAddf(NRRD, "%s: couldn't allocate scanline buffer", me);
          return 1;
        }
      }
    }
    rsmc->flag[flagLineAllocate] = AIR_TRUE;
  }
  return 0;
}

int
_nrrdResampleVectorAllocateUpdate(NrrdResampleContext *rsmc) {
  static const char me[]="_nrrdResampleVectorAllocateUpdate";
  unsigned int axIdx, kpIdx, dotLen, minSamples;
  nrrdResample_t spacingOut, support;
  NrrdResampleAxis *axis;

  if (rsmc->flag[flagKernels]
      || rsmc->flag[flagSamples]
      || rsmc->flag[flagRanges]) {
    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      axis = rsmc->axis + axIdx;
      if (!axis->kernel) {
        /* no resampling on this axis */
        continue;
      }
      /* check user-set parameters */
      if (!( AIR_EXISTS(axis->min) && AIR_EXISTS(axis->max) )) {
        biffAddf(NRRD, "%s: don't have min, max set on axis %u", me, axIdx);
        return 1;
      }
      for (kpIdx=0; kpIdx<axis->kernel->numParm; kpIdx++) {
        if (!AIR_EXISTS(axis->kparm[kpIdx])) {
          biffAddf(NRRD, "%s: didn't set kernel parm %u on axis %u",
                   me, kpIdx, axIdx);
          return 1;
        }
      }
      minSamples = (nrrdCenterCell == axis->center ? 1 : 2);
      if (!( axis->samples >= minSamples )) {
        biffAddf(NRRD, "%s: need at least %u output samples (not %u) for "
                 "%s-centered sampling along axis %u", me, minSamples,
                 AIR_CAST(unsigned int, axis->samples),
                 airEnumStr(nrrdCenter, axis->center), axIdx);
        return 1;
      }
      /* compute support (spacingIn == 1.0 by definition) */
      spacingOut = AIR_CAST(nrrdResample_t, ((axis->max - axis->min)
                                             / (nrrdCenterCell == axis->center
                                                ? axis->samples
                                                : axis->samples - 1)));
      axis->ratio = 1.0/spacingOut;
      support = AIR_CAST(nrrdResample_t, axis->kernel->support(axis->kparm));
      if (axis->ratio > 1) {
        /* if upsampling, we need only as many samples as needed for
           interpolation with the given kernel */
        dotLen = (int)(2*ceil(support));
      } else {
        /* if downsampling, we need to use all the samples covered by
           the stretched out version of the kernel */
        dotLen = (int)(2*ceil(support/axis->ratio));
      }
      /* some kernels can report zero support when they're basically
         delta functions */
      dotLen = AIR_MAX(2, dotLen);
      if (nrrdMaybeAlloc_va(axis->nweight, nrrdResample_nt, 2,
                            AIR_CAST(size_t, dotLen),
                            AIR_CAST(size_t, axis->samples))
          || nrrdMaybeAlloc_va(axis->nindex, nrrdTypeInt, 2,
                               AIR_CAST(size_t, dotLen),
                               AIR_CAST(size_t, axis->samples))) {
        biffAddf(NRRD, "%s: trouble allocating index and weighting vectors",
                 me);
        return 1;
      }
    }
    rsmc->flag[flagRanges] = AIR_FALSE;
    rsmc->flag[flagVectorAllocate] = AIR_TRUE;
  }

  return 0;
}

int
_nrrdResampleLineFillUpdate(NrrdResampleContext *rsmc) {
  unsigned int axIdx;
  NrrdResampleAxis *axis;
  nrrdResample_t *line;

  if (rsmc->flag[flagPadValue]
      || rsmc->flag[flagLineAllocate]) {

    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      axis = rsmc->axis + axIdx;
      if (axis->kernel) {
        line = (nrrdResample_t*)(axis->nline->data);
        line[axis->sizeIn] = AIR_CAST(nrrdResample_t, rsmc->padValue);
      }
    }

    rsmc->flag[flagPadValue] = AIR_FALSE;
    rsmc->flag[flagLineAllocate] = AIR_FALSE;
    rsmc->flag[flagLineFill] = AIR_TRUE;
  }
  return 0;
}

int
_nrrdResampleVectorFillUpdate(NrrdResampleContext *rsmc) {
  static const char me[]="_nrrdResampleVectorFillUpdate";
  unsigned int axIdx, dotIdx, dotLen, halfLen, smpIdx, kpIdx;
  int *indexData, tmp, base, rawIdx;
  nrrdResample_t *weightData, idx, integral;
  NrrdResampleAxis *axis;
  double kparm[NRRD_KERNEL_PARMS_NUM];

  if (rsmc->flag[flagRenormalize]
      || rsmc->flag[flagBoundary]
      || rsmc->flag[flagInputCenters]
      || rsmc->flag[flagInputSizes]
      || rsmc->flag[flagVectorAllocate]) {
    if (rsmc->verbose) {
      for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
        if (rsmc->axis[axIdx].kernel) {
          fprintf(stderr, "%s: axis %u: %s-centering\n", me, axIdx,
                  airEnumStr(nrrdCenter, rsmc->axis[axIdx].center));
        }
      }
    }

    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      axis = rsmc->axis + axIdx;
      if (!axis->kernel) {
        /* no resampling on this axis */
        continue;
      }

      /* calculate sample locations and do first pass on indices */
      indexData = (int *)axis->nindex->data;
      weightData = (nrrdResample_t *)axis->nweight->data;
      dotLen = AIR_CAST(unsigned int, axis->nweight->axis[0].size);
      halfLen = dotLen/2;
      for (smpIdx=0; smpIdx<axis->samples; smpIdx++) {
        idx = AIR_CAST(nrrdResample_t,
                       (nrrdCenterCell == axis->center
                        ? AIR_AFFINE(-0.5, smpIdx, axis->samples-0.5,
                                     axis->min, axis->max)
                        : AIR_AFFINE(0.0, smpIdx, axis->samples-1.0,
                                     axis->min, axis->max)));
        base = (int)floor(idx) - halfLen + 1;
        for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
          tmp = indexData[dotIdx + dotLen*smpIdx] = base + dotIdx;
          weightData[dotIdx + dotLen*smpIdx] = idx - tmp;
        }
        if (rsmc->verbose) {
          if (!smpIdx) {
            fprintf(stderr, "%s: smpIdx=%u -> idx=%g -> base=%d\n", me,
                    smpIdx, idx, base);
            fprintf(stderr, "%s: sample locations:\n", me);
          }
          fprintf(stderr, "%s: %d (sample locations)\n        ", me, smpIdx);
          for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
            fprintf(stderr, "%d/%g ",
                    indexData[dotIdx + dotLen*smpIdx],
                    weightData[dotIdx + dotLen*smpIdx]);
          }
          fprintf(stderr, "\n");
        }
      }

      /* figure out what to do with the out-of-range indices */
      for (dotIdx=0; dotIdx<dotLen*axis->samples; dotIdx++) {
        rawIdx = indexData[dotIdx];
        if (!AIR_IN_CL(0, rawIdx, AIR_CAST(int, axis->sizeIn)-1)) {
          switch(rsmc->boundary) {
          case nrrdBoundaryPad:
          case nrrdBoundaryWeight:  /* this will be further handled later */
            rawIdx = AIR_CAST(int, axis->sizeIn);
            break;
          case nrrdBoundaryBleed:
            rawIdx = AIR_CLAMP(0, rawIdx, AIR_CAST(int, axis->sizeIn)-1);
            break;
          case nrrdBoundaryWrap:
            rawIdx = AIR_MOD(rawIdx, AIR_CAST(int, axis->sizeIn));
            break;
          case nrrdBoundaryMirror:
            rawIdx = _nrrdMirror_32(AIR_CAST(unsigned int, axis->sizeIn), rawIdx);
            break;
          default:
            biffAddf(NRRD, "%s: boundary behavior %d unknown/unimplemented",
                     me, rsmc->boundary);
            return 0;
          }
          indexData[dotIdx] = rawIdx;
        }
      }

      /* run the sample locations through the chosen kernel.  We play a
         sneaky trick on the kernel parameter 0 in case of downsampling
         to create the blurring of the old index space */
      kparm[0] = (axis->ratio < 1
                  ? axis->kparm[0] / axis->ratio
                  : axis->kparm[0]);
      for (kpIdx=1; kpIdx<NRRD_KERNEL_PARMS_NUM; kpIdx++) {
        kparm[kpIdx] = axis->kparm[kpIdx];
      }
      axis->kernel->EVALN(weightData, weightData, dotLen*axis->samples, kparm);

      /* special handling of "cheap" kernel */
      if (nrrdKernelCheap == axis->kernel) {
        for (smpIdx=0; smpIdx<axis->samples; smpIdx++) {
          nrrdResample_t dist, minDist;
          int minIdx, minSet;
          minIdx = indexData[0 + dotLen*smpIdx];
          minDist = weightData[0 + dotLen*smpIdx];
          /* find sample closest to origin */
          for (dotIdx=1; dotIdx<dotLen; dotIdx++) {
            dist = weightData[dotIdx + dotLen*smpIdx];
            if (dist < minDist) {
              minDist = dist;
              minIdx = indexData[dotIdx + dotLen*smpIdx];
            }
          }
          /* set kernel weights to select sample closest to origin */
          minSet = AIR_FALSE;
          for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
            if (minIdx == indexData[dotIdx + dotLen*smpIdx] && !minSet) {
              weightData[dotIdx + dotLen*smpIdx] = 1.0;
              minSet = AIR_TRUE;
            } else {
              weightData[dotIdx + dotLen*smpIdx] = 0.0;
            }
          }
        }
      }

      if (rsmc->verbose) {
        fprintf(stderr, "%s: axis %u sample weights:\n", me, axIdx);
        for (smpIdx=0; smpIdx<axis->samples; smpIdx++) {
          fprintf(stderr, "%s: %d (sample weights)\n        ", me, smpIdx);
          for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
            fprintf(stderr, "%d/%g ", indexData[dotIdx + dotLen*smpIdx],
                    weightData[dotIdx + dotLen*smpIdx]);
          }
          fprintf(stderr, "\n");
        }
      }

      /* final fixes on weighting values */
      integral = AIR_CAST(nrrdResample_t, axis->kernel->integral(axis->kparm));
      if (nrrdBoundaryWeight == rsmc->boundary) {
        if (integral) {
          /* above, we set to axis->sizeIn all the indices that were out of
             range.  We now use that to determine the sum of the weights
             for the indices that were in-range */
          for (smpIdx=0; smpIdx<axis->samples; smpIdx++) {
            nrrdResample_t wght = 0;
            for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
              if (AIR_CAST(int, axis->sizeIn)
                  != indexData[dotIdx + dotLen*smpIdx]) {
                wght += weightData[dotIdx + dotLen*smpIdx];
              }
            }
            for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
              if (AIR_CAST(int, axis->sizeIn)
                  != indexData[dotIdx + dotLen*smpIdx]) {
                weightData[dotIdx + dotLen*smpIdx] *= integral/wght;
              } else {
                weightData[dotIdx + dotLen*smpIdx] = 0;
              }
            }
          }
        }
      } else {
        /* try to remove ripple/grating on downsampling, and errors in
           weighting on upsampling when using kernels that are not
           first-order accurate */
        if (rsmc->renormalize && integral) {
          for (smpIdx=0; smpIdx<axis->samples; smpIdx++) {
            nrrdResample_t wght = 0;
            for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
              wght += weightData[dotIdx + dotLen*smpIdx];
            }
            if (wght) {
              for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
                /* this used to normalize the weights so that they summed
                   to integral ("*= integral/wght"), which meant that if
                   you use a very truncated Gaussian, then your over-all
                   image brightness goes down.  This seems very contrary
                   to the whole point of renormalization. */
                weightData[dotIdx + dotLen*smpIdx] *=
                  AIR_CAST(nrrdResample_t, 1.0/wght);
              }
            }
          }
        }
      }

      if (rsmc->verbose) {
        fprintf(stderr, "%s: axis %u post-correction sample weights:\n",
                me, axIdx);
        for (smpIdx=0; smpIdx<axis->samples; smpIdx++) {
          fprintf(stderr, "%s: %d (sample weights)\n        ", me, smpIdx);
          for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
            fprintf(stderr, "%d/%g ", indexData[dotIdx + dotLen*smpIdx],
                    weightData[dotIdx + dotLen*smpIdx]);
          }
          fprintf(stderr, "\n");
        }
      }
    }
    rsmc->flag[flagRenormalize] = AIR_FALSE;
    rsmc->flag[flagBoundary] = AIR_FALSE;
    rsmc->flag[flagInputCenters] = AIR_FALSE;
    rsmc->flag[flagVectorAllocate] = AIR_FALSE;
    rsmc->flag[flagVectorFill] = AIR_TRUE;
  }

  return 0;
}

int
_nrrdResamplePermutationUpdate(NrrdResampleContext *rsmc) {
  static const char me[]="_nrrdResamplePermutationUpdate";
  unsigned int axIdx, passIdx, currTop, lastTop, fromTop, toTop;
  int bi;

  if (rsmc->flag[flagInputSizes]
      || rsmc->flag[flagKernels]
      || rsmc->flag[flagSamples]) {

    rsmc->topRax = rsmc->botRax = AIR_CAST(unsigned int, -1);
    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      if (rsmc->axis[axIdx].kernel) {
        if (AIR_CAST(unsigned int, -1) == rsmc->topRax) {
          rsmc->topRax = axIdx;
        }
        rsmc->botRax = axIdx;
      }
    }
    if (rsmc->verbose) {
      fprintf(stderr, "%s: topRax = %u (%d); botRax = %u (%d)\n", me,
              rsmc->topRax, AIR_CAST(int, rsmc->topRax),
              rsmc->botRax, AIR_CAST(int, rsmc->botRax));
    }

    /* figure out total number of passes needed, and construct the
       permute[] array.  permute[i] = j means that the axis in position
       i of the old array will be in position j of the new one
       (permute[] answers "where do I put this", not "what got put here").
    */
    rsmc->passNum = 0;
    bi = 0;
    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      if (rsmc->axis[axIdx].kernel) {
        do {
          bi = AIR_MOD(bi+1, AIR_CAST(int, rsmc->dim));
        } while (!rsmc->axis[bi].kernel);
        rsmc->permute[bi] = axIdx;
        rsmc->passNum += 1;
      } else {
        rsmc->permute[axIdx] = axIdx;
        bi += bi == AIR_CAST(int, axIdx);
      }
    }
    rsmc->permute[rsmc->dim] = rsmc->dim;  /* HEY: what is this for? */

    if (rsmc->passNum) {
      toTop = AIR_CAST(unsigned int, -1);
      for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
        /* this will always "break" somewhere */
        if (rsmc->topRax == rsmc->permute[axIdx]) {
          toTop = axIdx;
          break;
        }
      }
      fromTop = rsmc->permute[rsmc->topRax];

      if (rsmc->verbose) {
        fprintf(stderr, "%s: passNum = %u; permute =\n     ",
                me, rsmc->passNum);
        for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
          fprintf(stderr, "%u ", rsmc->permute[axIdx]);
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "%s: toTop = %u; fromTop = %u\n", me, toTop, fromTop);
      }

      /* create array of how the axes will be arranged in each pass ("ax"),
         and create array of how big each axes is in each pass ("sz").
         The input to pass i will have axis layout described in ax[i] and
         axis sizes described in sz[i] */
      passIdx = 0;
      currTop = rsmc->topRax;
      rsmc->passAxis[passIdx] = currTop;
      rsmc->axis[currTop].passIdx = passIdx;
      for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
        rsmc->axis[currTop].axisPerm[axIdx] = axIdx;
        rsmc->axis[currTop].sizePerm[axIdx] = rsmc->axis[axIdx].sizeIn;
      }
      for (passIdx=1; passIdx<rsmc->passNum+1; passIdx++) {
        lastTop = currTop;
        currTop = (passIdx<rsmc->passNum
                   ? rsmc->axis[currTop].axisPerm[toTop]
                   : NRRD_DIM_MAX);
        rsmc->passAxis[passIdx] = currTop;
        rsmc->axis[currTop].passIdx = passIdx;
        for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
          rsmc->axis[currTop].axisPerm[rsmc->permute[axIdx]]
            = rsmc->axis[lastTop].axisPerm[axIdx];
          rsmc->axis[currTop].sizePerm[rsmc->permute[axIdx]]
            = rsmc->axis[lastTop].sizePerm[axIdx];
          /* modify the one size corresponding to the resampled axis */
          rsmc->axis[currTop].sizePerm[fromTop] = rsmc->axis[lastTop].samples;
        }
      }
      if (rsmc->verbose) {
        NrrdResampleAxis *axis;
        fprintf(stderr, "%s: axis and size permutations:\n", me);
        for (passIdx=0; passIdx<rsmc->passNum+1; passIdx++) {
          axis = rsmc->axis + rsmc->passAxis[passIdx];
          fprintf(stderr, "----- pass[%u=?=%u] @ %u %s:\n", passIdx,
                  axis->passIdx, rsmc->passAxis[passIdx],
                  (passIdx<rsmc->passNum ? "" : "(output of final pass)"));
          if (!passIdx) {
            fprintf(stderr, "resampling: ");
            for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
              fprintf(stderr, "%s ", rsmc->axis[axIdx].kernel ? " XX" : "   ");
            }
            fprintf(stderr, "\n");
          }
          fprintf(stderr, "      axes: ");
          for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
            fprintf(stderr, "%3u ", axis->axisPerm[axIdx]);
          }
          fprintf(stderr, "\n");
          fprintf(stderr, "     sizes: ");
          for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
            fprintf(stderr, "%3u ",
                    AIR_CAST(unsigned int, axis->sizePerm[axIdx]));
          }
          fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
      }
    }

    rsmc->flag[flagInputSizes] = AIR_FALSE;
    rsmc->flag[flagKernels] = AIR_FALSE;
    rsmc->flag[flagSamples] = AIR_FALSE;
    rsmc->flag[flagPermutation] = AIR_TRUE;
  }

  return 0;
}

/* Copy input to output, but with the optional clamping and rounding */
int
_nrrdResampleTrivial(NrrdResampleContext *rsmc, Nrrd *nout,
                     int typeOut, int doRound,
                     nrrdResample_t (*lup)(const void *, size_t),
                     nrrdResample_t (*clamp)(nrrdResample_t),
                     nrrdResample_t (*ins)(void *, size_t, nrrdResample_t)) {
  static const char me[]="_nrrdResampleTrivial";
  size_t size[NRRD_DIM_MAX], valNum, valIdx;
  nrrdResample_t val;
  const void *dataIn;
  void *dataOut;

  nrrdAxisInfoGet_nva(rsmc->nin, nrrdAxisInfoSize, size);
  if (nrrdMaybeAlloc_nva(nout, typeOut, rsmc->nin->dim, size)) {
    biffAddf(NRRD, "%s: couldn't allocate output", me);
    return 1;
  }
  valNum = nrrdElementNumber(rsmc->nin);
  dataIn = rsmc->nin->data;
  dataOut = nout->data;
  for (valIdx=0; valIdx<valNum; valIdx++) {
    val = lup(dataIn, valIdx);
    if (doRound) {
      val = AIR_CAST(nrrdResample_t, AIR_ROUNDUP(val));
    }
    if (rsmc->clamp) {
      val = clamp(val);
    }
    ins(dataOut, valIdx, val);
  }

  return 0;
}

int
_nrrdResampleCore(NrrdResampleContext *rsmc, Nrrd *nout,
                  int typeOut, int doRound,
                  nrrdResample_t (*lup)(const void *, size_t),
                  nrrdResample_t (*clamp)(nrrdResample_t),
                  nrrdResample_t (*ins)(void *, size_t, nrrdResample_t)) {
  static const char me[]="_nrrdResampleCore";
  unsigned int axIdx, passIdx;
  size_t strideIn, strideOut, lineNum, lineIdx,
    coordIn[NRRD_DIM_MAX], coordOut[NRRD_DIM_MAX];
  nrrdResample_t *line, *weight, *rsmpIn, *rsmpOut;
  int *indx;
  const void *dataIn;
  void *dataOut;
  NrrdResampleAxis *axisIn, *axisOut;
  airArray *mop;

  /* NOTE: there was an odd memory leak here with normal operation (no
     errors), because the final airMopOkay() was missing, but quick
     attempts at resolving it pre-Teem-1.9 release were not successful
     (surprisingly, commenting out the airMopSub's led to a segfault).
     So, the airMopAdd which is supposed to manage the per-axis
     resampling result is commented out, and there are no leaks and
     no segfaults with normal operation, which is good enough for now */

  /* compute strideIn; this is constant across passes because all
     passes resample topRax, and axes with lower indices have
     constant length. */
  strideIn = 1;
  for (axIdx=0; axIdx<rsmc->topRax; axIdx++) {
    strideIn *= rsmc->axis[axIdx].sizeIn;
  }

  mop = airMopNew();
  for (passIdx=0; passIdx<rsmc->passNum; passIdx++) {
    if (rsmc->verbose) {
      fprintf(stderr, "%s: -------------- pass %u/%u \n",
              me, passIdx, rsmc->passNum);
    }

    /* calculate pass-specific size, stride, and number info */
    axisIn = rsmc->axis + rsmc->passAxis[passIdx];
    axisOut = rsmc->axis + rsmc->passAxis[passIdx+1];
    lineNum = strideOut = 1;
    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      if (axIdx < rsmc->botRax) {
        strideOut *= axisOut->sizePerm[axIdx];
      }
      if (axIdx != rsmc->topRax) {
        lineNum *= axisIn->sizePerm[axIdx];
      }
    }
    if (rsmc->verbose) {
      char stmp1[AIR_STRLEN_SMALL], stmp2[AIR_STRLEN_SMALL];
      fprintf(stderr, "%s(%u): lineNum = %s\n", me, passIdx,
              airSprintSize_t(stmp1, lineNum));
      fprintf(stderr, "%s(%u): strideIn = %s, stridOut = %s\n", me, passIdx,
              airSprintSize_t(stmp1, strideIn),
              airSprintSize_t(stmp2, strideOut));
    }

    /* allocate output for this pass */
    if (passIdx < rsmc->passNum-1) {
      axisOut->nrsmp = nrrdNew();
      /* see NOTE above!
         airMopAdd(mop, axisOut->nrsmp, (airMopper)nrrdNuke, airMopAlways); */
      if (nrrdMaybeAlloc_nva(axisOut->nrsmp, nrrdResample_nt, rsmc->dim,
                             axisOut->sizePerm)) {
        biffAddf(NRRD, "%s: trouble allocating output of pass %u", me,
                 passIdx);
        airMopError(mop); return 1;
      }
      if (rsmc->verbose) {
        fprintf(stderr, "%s: allocated pass %u/%u output nrrd @ %p/%p "
                "(on axis %u)\n", me, passIdx, axisIn->passIdx,
                AIR_CAST(void*, axisOut->nrsmp),
                AIR_CAST(void*, axisOut->nrsmp->data), axisOut->axIdx);
      }
    } else {
      if (nrrdMaybeAlloc_nva(nout, typeOut, rsmc->dim, axisOut->sizePerm)) {
        biffAddf(NRRD, "%s: trouble allocating final output", me);
        airMopError(mop); return 1;
      }
      if (rsmc->verbose) {
        fprintf(stderr, "%s: allocated final pass %u output nrrd @ %p/%p\n",
                me, passIdx,
                AIR_CAST(void*, nout),
                AIR_CAST(void*, nout->data));
      }
    }

    /* set up data pointers */
    if (0 == passIdx) {
      rsmpIn = NULL;
      dataIn = rsmc->nin->data;
    } else {
      rsmpIn = (nrrdResample_t *)(axisIn->nrsmp->data);
      dataIn = NULL;
    }
    if (passIdx < rsmc->passNum-1) {
      rsmpOut = (nrrdResample_t *)(axisOut->nrsmp->data);
      dataOut = NULL;
    } else {
      rsmpOut = NULL;
      dataOut = nout->data;
    }

    line = (nrrdResample_t *)(axisIn->nline->data);
    indx = (int *)(axisIn->nindex->data);
    weight = (nrrdResample_t *)(axisIn->nweight->data);

    /* the skinny */
    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      coordIn[axIdx] = 0;
      coordOut[axIdx] = 0;
    }
    for (lineIdx=0; lineIdx<lineNum; lineIdx++) {
      size_t smpIdx, dotIdx, dotLen, indexIn, indexOut;

      /* calculate the (linear) indices of the beginnings of
         the input and output scanlines */
      NRRD_INDEX_GEN(indexIn, coordIn, axisIn->sizePerm, rsmc->dim);
      NRRD_INDEX_GEN(indexOut, coordOut, axisOut->sizePerm, rsmc->dim);

      /* read input scanline into scanline buffer */
      if (0 == passIdx) {
        for (smpIdx=0; smpIdx<axisIn->sizeIn; smpIdx++) {
          line[smpIdx] = lup(dataIn, smpIdx*strideIn + indexIn);
        }
      } else {
        for (smpIdx=0; smpIdx<axisIn->sizeIn; smpIdx++) {
          line[smpIdx] = rsmpIn[smpIdx*strideIn + indexIn];
        }
      }

      /* do the bloody convolution and save the output value */
      dotLen = axisIn->nweight->axis[0].size;
      for (smpIdx=0; smpIdx<axisIn->samples; smpIdx++) {
        double val;
        val = 0.0;
        if (nrrdResampleNonExistentNoop != rsmc->nonExistent) {
          double wsum;
          wsum = 0.0;
          for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
            double tmpV, tmpW;
            tmpV = line[indx[dotIdx + dotLen*smpIdx]];
            if (AIR_EXISTS(tmpV)) {
              tmpW = weight[dotIdx + dotLen*smpIdx];
              val += tmpV*tmpW;
              wsum += tmpW;
            }
          }
          if (wsum) {
            if (nrrdResampleNonExistentRenormalize == rsmc->nonExistent) {
              val /= wsum;
            }
            /* else nrrdResampleNonExistentWeight: leave as is */
          } else {
            val = AIR_NAN;
          }
        } else {
          /* nrrdResampleNonExistentNoop: do convolution sum
             w/out worries about value existance */
          for (dotIdx=0; dotIdx<dotLen; dotIdx++) {
            val += (line[indx[dotIdx + dotLen*smpIdx]]
                    * weight[dotIdx + dotLen*smpIdx]);
          }
        }
        if (passIdx < rsmc->passNum-1) {
          rsmpOut[smpIdx*strideOut + indexOut] = val;
        } else {
          if (doRound) {
            val = AIR_CAST(nrrdResample_t, AIR_ROUNDUP(val));
          }
          if (rsmc->clamp) {
            val = clamp(val);
          }
          ins(dataOut, smpIdx*strideOut + indexOut, val);
        }
      }

      /* as long as there's another line to be processed, increment the
         coordinates for the scanline starts.  We don't use the usual
         NRRD_COORD macros because we're subject to the unusual constraint
         that coordIn[topRax] and coordOut[permute[topRax]] must stay == 0 */
      if (lineIdx < lineNum-1) {
        axIdx = rsmc->topRax ? 0 : 1;
        coordIn[axIdx]++;
        coordOut[rsmc->permute[axIdx]]++;
        while (coordIn[axIdx] == axisIn->sizePerm[axIdx]) {
          coordIn[axIdx] = coordOut[rsmc->permute[axIdx]] = 0;
          axIdx++;
          axIdx += axIdx == rsmc->topRax;
          coordIn[axIdx]++;
          coordOut[rsmc->permute[axIdx]]++;
        }
      }
    }

    /* (maybe) free input to this pass, now that we're done with it */
    if (axisIn->nrsmp) {
      if (rsmc->verbose) {
        fprintf(stderr, "%s: nrrdNuke(%p) pass %u input (on axis %u)\n",
                me, AIR_CAST(void*, axisIn->nrsmp), axisIn->passIdx,
                axisIn->axIdx);
      }
      axisIn->nrsmp = nrrdNuke(axisIn->nrsmp);
      /* airMopSub(mop, axisIn->nrsmp, (airMopper)nrrdNuke); */
    }
  } /* for passIdx */

  airMopOkay(mop);
  return 0;
}

int
_nrrdResampleOutputUpdate(NrrdResampleContext *rsmc, Nrrd *nout,
                          const char *func) {
  static const char me[]="_nrrdResampleOutputUpdate";
#if NRRD_RESAMPLE_FLOAT
  float (*lup)(const void *, size_t),
    (*clamp)(float), (*ins)(void *, size_t, float);
#else
  double (*lup)(const void *, size_t),
    (*clamp)(double), (*ins)(void *, size_t, double);
#endif
  unsigned int axIdx;
  int typeOut, doRound;

  if (rsmc->flag[flagClamp]
      || rsmc->flag[flagNonExistent]
      || rsmc->flag[flagRound]
      || rsmc->flag[flagTypeOut]
      || rsmc->flag[flagLineFill]
      || rsmc->flag[flagVectorFill]
      || rsmc->flag[flagPermutation]
      || rsmc->flag[flagInput]) {

    typeOut = (nrrdTypeDefault == rsmc->typeOut
               ? rsmc->nin->type
               : rsmc->typeOut);
    doRound = rsmc->roundlast && nrrdTypeIsIntegral[typeOut];
    if (doRound && (nrrdTypeInt == typeOut
                    || nrrdTypeUInt == typeOut
                    || nrrdTypeLLong == typeOut
                    || nrrdTypeULLong == typeOut)) {
      fprintf(stderr, "%s: WARNING: possible erroneous output with "
              "rounding of %s output type due to int-based implementation "
              "of rounding\n", me, airEnumStr(nrrdType, typeOut));
    }

#if NRRD_RESAMPLE_FLOAT
    lup = nrrdFLookup[rsmc->nin->type];
    clamp = nrrdFClamp[typeOut];
    ins = nrrdFInsert[typeOut];
#else
    lup = nrrdDLookup[rsmc->nin->type];
    clamp = nrrdDClamp[typeOut];
    ins = nrrdDInsert[typeOut];
#endif

    if (0 == rsmc->passNum) {
      if (_nrrdResampleTrivial(rsmc, nout, typeOut, doRound,
                               lup, clamp, ins)) {
        biffAddf(NRRD, "%s: trouble", me);
        return 1;
      }
    } else {
      if (_nrrdResampleCore(rsmc, nout, typeOut, doRound,
                            lup, clamp, ins)) {
        biffAddf(NRRD, "%s: trouble", me);
        return 1;
      }
    }

    /* HEY: need to create textual representation of resampling parameters */
    if (nrrdContentSet_va(nout, func, rsmc->nin, "")) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }

    /* start work of updating space origin */
    nrrdSpaceVecCopy(nout->spaceOrigin, rsmc->nin->spaceOrigin);
    for (axIdx=0; axIdx<rsmc->dim; axIdx++) {
      if (rsmc->axis[axIdx].kernel) {
        /* this axis was resampled */
        double minIdxFull, maxIdxFull,
          zeroPos;  /* actually its in continuous index space ... */
        _nrrdAxisInfoCopy(nout->axis + axIdx, rsmc->nin->axis + axIdx,
                          (NRRD_AXIS_INFO_SIZE_BIT
                           | NRRD_AXIS_INFO_SPACING_BIT
                           | NRRD_AXIS_INFO_THICKNESS_BIT
                           | NRRD_AXIS_INFO_MIN_BIT
                           | NRRD_AXIS_INFO_MAX_BIT
                           | NRRD_AXIS_INFO_SPACEDIRECTION_BIT
                           | NRRD_AXIS_INFO_CENTER_BIT
                           | NRRD_AXIS_INFO_KIND_BIT));
        /* now set all the per-axis fields we just abstained from copying */
        /* size was already set */
        nout->axis[axIdx].spacing = (rsmc->nin->axis[axIdx].spacing
                                     / rsmc->axis[axIdx].ratio);
        /* for now, we don't attempt to modify thickness */
        nout->axis[axIdx].thickness = AIR_NAN;
        /* We had to assume a specific centering when doing resampling */
        nout->axis[axIdx].center = rsmc->axis[axIdx].center;
        _nrrdResampleMinMaxFull(&minIdxFull, &maxIdxFull,
                                rsmc->axis[axIdx].center,
                                rsmc->nin->axis[axIdx].size);
        nout->axis[axIdx].min = AIR_AFFINE(minIdxFull,
                                           rsmc->axis[axIdx].min,
                                           maxIdxFull,
                                           rsmc->nin->axis[axIdx].min,
                                           rsmc->nin->axis[axIdx].max);
        nout->axis[axIdx].max = AIR_AFFINE(minIdxFull,
                                           rsmc->axis[axIdx].max,
                                           maxIdxFull,
                                           rsmc->nin->axis[axIdx].min,
                                           rsmc->nin->axis[axIdx].max);
        nrrdSpaceVecScale(nout->axis[axIdx].spaceDirection,
                          1.0/rsmc->axis[axIdx].ratio,
                          rsmc->nin->axis[axIdx].spaceDirection);
        nout->axis[axIdx].kind = _nrrdKindAltered(rsmc->nin->axis[axIdx].kind,
                                                  AIR_TRUE);
        /* space origin may have translated along this axis;
           only do this if the axis was already spatial */
        if (AIR_EXISTS(rsmc->nin->axis[axIdx].spaceDirection[0])) {
          zeroPos = NRRD_POS(nout->axis[axIdx].center,
                             rsmc->axis[axIdx].min,
                             rsmc->axis[axIdx].max,
                             rsmc->axis[axIdx].samples,
                             0);
          nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                                1.0, nout->spaceOrigin,
                                zeroPos,
                                rsmc->nin->axis[axIdx].spaceDirection);
        }
      } else {
        /* no resampling; this axis totally unchanged */
        _nrrdAxisInfoCopy(nout->axis + axIdx, rsmc->nin->axis + axIdx,
                          NRRD_AXIS_INFO_NONE);
        /* also: the space origin has not translated along this axis */
      }
    }
    if (nrrdBasicInfoCopy(nout, rsmc->nin,
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


    rsmc->flag[flagClamp] = AIR_FALSE;
    rsmc->flag[flagNonExistent] = AIR_FALSE;
    rsmc->flag[flagRound] = AIR_FALSE;
    rsmc->flag[flagTypeOut] = AIR_FALSE;
    rsmc->flag[flagLineFill] = AIR_FALSE;
    rsmc->flag[flagVectorFill] = AIR_FALSE;
    rsmc->flag[flagPermutation] = AIR_FALSE;
    rsmc->flag[flagInput] = AIR_FALSE;
  }

  return 0;
}

int
nrrdResampleExecute(NrrdResampleContext *rsmc, Nrrd *nout) {
  static const char me[]="nrrdResampleExecute", func[]="resample";
  double time0;

  if (!(rsmc && nout)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }

  /* any other error checking?  Do we need a _nrrdResampleContextCheck() ? */
  if (nrrdBoundaryPad == rsmc->boundary && !AIR_EXISTS(rsmc->padValue)) {
    biffAddf(NRRD, "%s: asked for boundary padding, "
             "but no pad value set", me);
    return 1;
  }

  time0 = airTime();
  if (_nrrdResampleInputDimensionUpdate(rsmc)
      || _nrrdResampleInputCentersUpdate(rsmc)
      || _nrrdResampleInputSizesUpdate(rsmc)
      || _nrrdResampleLineAllocateUpdate(rsmc)
      || _nrrdResampleVectorAllocateUpdate(rsmc)
      || _nrrdResampleLineFillUpdate(rsmc)
      || _nrrdResampleVectorFillUpdate(rsmc)
      || _nrrdResamplePermutationUpdate(rsmc)
      || _nrrdResampleOutputUpdate(rsmc, nout, func)) {
    biffAddf(NRRD, "%s: trouble", me);
    return 1;
  }
  rsmc->time = airTime() - time0;

  return 0;
}
