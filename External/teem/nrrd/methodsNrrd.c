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
Wed Sep 14 05:55:40 EDT 2005: these are no longer used
void
nrrdPeripheralInit(Nrrd *nrrd) {

  nrrdBasicInfoInit(nrrd,
                    NRRD_BASIC_INFO_DATA_BIT
                    | NRRD_BASIC_INFO_TYPE_BIT
                    | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                    | NRRD_BASIC_INFO_DIMENSION_BIT
                    | NRRD_BASIC_INFO_CONTENT_BIT
                    | NRRD_BASIC_INFO_COMMENTS_BIT
                    | NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT);
  return;
}

int
nrrdPeripheralCopy(Nrrd *nout, const Nrrd *nin) {

  nrrdBasicInfoCopy(nout, nin,
                    NRRD_BASIC_INFO_DATA_BIT
                    | NRRD_BASIC_INFO_TYPE_BIT
                    | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                    | NRRD_BASIC_INFO_DIMENSION_BIT
                    | NRRD_BASIC_INFO_CONTENT_BIT
                    | NRRD_BASIC_INFO_COMMENTS_BIT
                    | NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT);
  return 0;
}
*/

/* ---- BEGIN non-NrrdIO */
const int
nrrdPresent = 42;
/* ---- END non-NrrdIO */

/* ------------------------------------------------------------ */

void
nrrdIoStateInit(NrrdIoState *nio) {

  if (nio) {
    nio->path = (char *)airFree(nio->path);
    nio->base = (char *)airFree(nio->base);
    nio->line = (char *)airFree(nio->line);
    nio->dataFNFormat = (char *)airFree(nio->dataFNFormat);
    /* the way IO to/from strings works, I don't think this should be freed */
    nio->headerStringRead = NULL;
    nio->headerStringWrite = NULL;
    airArrayLenSet(nio->dataFNArr, 0);
    /* closing this is always someone else's responsibility */
    nio->headerFile = NULL;
    nio->dataFile = NULL;
    nio->dataFileDim = 0;
    nio->dataFNMin = 0;
    nio->dataFNMax = 0;
    nio->dataFNStep = 0;
    nio->dataFNIndex = 0;
    nio->lineLen = 0;
    nio->pos = 0;
    nio->endian = airEndianUnknown;
    nio->lineSkip = 0;
    nio->headerStrlen = 0;
    nio->headerStrpos = 0;
    nio->byteSkip = 0;
    memset(nio->seen, 0, (NRRD_FIELD_MAX+1)*sizeof(int));
    nio->detachedHeader = AIR_FALSE;
    nio->bareText = nrrdDefaultWriteBareText;
    nio->charsPerLine = nrrdDefaultWriteCharsPerLine;
    nio->valsPerLine = nrrdDefaultWriteValsPerLine;
    nio->skipData = AIR_FALSE;
    nio->skipFormatURL = AIR_FALSE;
    nio->keepNrrdDataFileOpen = AIR_FALSE;
    nio->zlibLevel = -1;
    nio->zlibStrategy = nrrdZlibStrategyDefault;
    nio->bzip2BlockSize = -1;
    nio->learningHeaderStrlen = AIR_FALSE;
    nio->oldData = NULL;
    nio->oldDataSize = 0;
    nio->format = nrrdFormatUnknown;
    nio->encoding = nrrdEncodingUnknown;
  }
  return;
}

NrrdIoState *
nrrdIoStateNew(void) {
  NrrdIoState *nio;

  nio = (NrrdIoState *)calloc(1, sizeof(NrrdIoState));
  if (nio) {
    airPtrPtrUnion appu;

    nio->path = NULL;
    nio->base = NULL;
    nio->line = NULL;
    nio->dataFNFormat = NULL;
    nio->dataFN = NULL;
    nio->headerStringRead = NULL;
    nio->headerStringWrite = NULL;
    appu.cp = &(nio->dataFN);
    nio->dataFNArr = airArrayNew(appu.v, NULL,
                                 sizeof(char *), NRRD_FILENAME_INCR);
    airArrayPointerCB(nio->dataFNArr, airNull, airFree);
    nio->format = nrrdFormatUnknown;
    nio->encoding = nrrdEncodingUnknown;
    nrrdIoStateInit(nio);
  }
  return nio;
}

NrrdIoState *
nrrdIoStateNix(NrrdIoState *nio) {

  nio->path = (char *)airFree(nio->path);
  nio->base = (char *)airFree(nio->base);
  nio->line = (char *)airFree(nio->line);
  nio->dataFNFormat = (char *)airFree(nio->dataFNFormat);
  nio->dataFNArr = airArrayNuke(nio->dataFNArr);
  /* the NrrdIoState never owned nio->oldData; we don't free it */
  airFree(nio);  /* no NULL assignment, else compile warnings */
  return NULL;
}

/* ---- BEGIN non-NrrdIO */

/* ------------------------------------------------------------ */

void
_nrrdResampleInfoInit(NrrdResampleInfo *info) {
  int i, d;

  for (d=0; d<NRRD_DIM_MAX; d++) {
    info->kernel[d] = NULL;
    info->samples[d] = 0;
    info->parm[d][0] = nrrdDefaultKernelParm0;
    for (i=1; i<NRRD_KERNEL_PARMS_NUM; i++)
      info->parm[d][i] = AIR_NAN;
    info->min[d] = info->max[d] = AIR_NAN;
  }
  info->boundary = nrrdDefaultResampleBoundary;
  info->type = nrrdDefaultResampleType;
  info->renormalize = nrrdDefaultResampleRenormalize;
  info->round = nrrdDefaultResampleRound;
  info->clamp = nrrdDefaultResampleClamp;
  info->cheap = nrrdDefaultResampleCheap;
  info->padValue = nrrdDefaultResamplePadValue;
}

NrrdResampleInfo *
nrrdResampleInfoNew(void) {
  NrrdResampleInfo *info;

  info = (NrrdResampleInfo*)(calloc(1, sizeof(NrrdResampleInfo)));
  if (info) {
    /* explicitly sets pointers to NULL */
    _nrrdResampleInfoInit(info);
  }
  return info;
}

NrrdResampleInfo *
nrrdResampleInfoNix(NrrdResampleInfo *info) {

  info = (NrrdResampleInfo *)airFree(info);
  return NULL;
}

/* ------------------------------------------------------------ */

NrrdKernelSpec *
nrrdKernelSpecNew(void) {
  NrrdKernelSpec *ksp;
  int i;

  ksp = (NrrdKernelSpec *)calloc(1, sizeof(NrrdKernelSpec));
  if (ksp) {
    ksp->kernel = NULL;
    for (i=0; i<NRRD_KERNEL_PARMS_NUM; i++) {
      ksp->parm[i] = airNaN();  /* valgrind complained about AIR_NAN at -O2 */
    }
  }
  return ksp;
}

NrrdKernelSpec *
nrrdKernelSpecCopy(const NrrdKernelSpec *oldKsp) {
  NrrdKernelSpec *ksp=NULL;

  if (oldKsp) {
    ksp = (NrrdKernelSpec *)calloc(1, sizeof(NrrdKernelSpec));
    if (ksp) {
      memcpy(ksp, oldKsp, sizeof(NrrdKernelSpec));
    }
  }
  return ksp;
}

NrrdKernelSpec *
nrrdKernelSpecNix(NrrdKernelSpec *ksp) {

  ksp = (NrrdKernelSpec *)airFree(ksp);
  return NULL;
}

void
nrrdKernelSpecSet(NrrdKernelSpec *ksp, const NrrdKernel *k,
                  const double kparm[NRRD_KERNEL_PARMS_NUM]) {
  unsigned int p;

  if (ksp && k && kparm) {
    ksp->kernel = k;
    for (p=0; p<(k->numParm); p++) {
      ksp->parm[p] = kparm[p];
    }
  }
}

void
nrrdKernelParmSet(const NrrdKernel **kP, double kparm[NRRD_KERNEL_PARMS_NUM],
                  NrrdKernelSpec *ksp) {
  int p;

  if (kP && kparm && ksp) {
    *kP = ksp->kernel;
    for (p=0; p<NRRD_KERNEL_PARMS_NUM; p++) {
      kparm[p] = ksp->parm[p];
    }
  }
}

/* ---- END non-NrrdIO */

/* ------------------------------------------------------------ */

/* see axis.c for axis-specific "methods" */

/* ------------------------------------------------------------ */

/*
******** nrrdBasicInfoInit
**
** resets "basic" (per-array) information
** formerly nrrdPeripheralInit
**
** the bitflag communicates which fields should *not* be initialized
*/
void
nrrdBasicInfoInit(Nrrd *nrrd, int bitflag) {
  int dd, ee;

  if (!nrrd) {
    return;
  }

  if (!(NRRD_BASIC_INFO_DATA_BIT & bitflag)) {
    nrrd->data = airFree(nrrd->data);
  }
  if (!(NRRD_BASIC_INFO_TYPE_BIT & bitflag)) {
    nrrd->type = nrrdTypeUnknown;
  }
  if (!(NRRD_BASIC_INFO_BLOCKSIZE_BIT & bitflag)) {
    nrrd->blockSize = 0;
  }
  if (!(NRRD_BASIC_INFO_DIMENSION_BIT & bitflag)) {
    nrrd->dim = 0;
  }
  if (!(NRRD_BASIC_INFO_CONTENT_BIT & bitflag)) {
    nrrd->content = (char *)airFree(nrrd->content);
  }
  if (!(NRRD_BASIC_INFO_SAMPLEUNITS_BIT & bitflag)) {
    nrrd->sampleUnits = (char *)airFree(nrrd->sampleUnits);
  }
  if (!(NRRD_BASIC_INFO_SPACE_BIT & bitflag)) {
    nrrd->space = nrrdSpaceUnknown;
    nrrd->spaceDim = 0;
  }
  if (!(NRRD_BASIC_INFO_SPACEDIMENSION_BIT & bitflag)) {
    nrrd->space = nrrdSpaceUnknown;
    nrrd->spaceDim = 0;
  }
  if (!(NRRD_BASIC_INFO_SPACEUNITS_BIT & bitflag)) {
    for (dd=0; dd<NRRD_SPACE_DIM_MAX; dd++) {
      nrrd->spaceUnits[dd] = (char *)airFree(nrrd->spaceUnits[dd]);
    }
  }
  if (!(NRRD_BASIC_INFO_SPACEORIGIN_BIT & bitflag)) {
    for (dd=0; dd<NRRD_SPACE_DIM_MAX; dd++) {
      nrrd->spaceOrigin[dd] = AIR_NAN;
    }
  }
  if (!(NRRD_BASIC_INFO_MEASUREMENTFRAME_BIT & bitflag)) {
    for (dd=0; dd<NRRD_SPACE_DIM_MAX; dd++) {
      for (ee=0; ee<NRRD_SPACE_DIM_MAX; ee++) {
        nrrd->measurementFrame[dd][ee] = AIR_NAN;
      }
    }
  }
  if (!(NRRD_BASIC_INFO_OLDMIN_BIT & bitflag)) {
    nrrd->oldMin = AIR_NAN;
  }
  if (!(NRRD_BASIC_INFO_OLDMAX_BIT & bitflag)) {
    nrrd->oldMax = AIR_NAN;
  }
  if (!(NRRD_BASIC_INFO_COMMENTS_BIT & bitflag)) {
    nrrdCommentClear(nrrd);
  }
  if (!(NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT & bitflag)) {
    nrrdKeyValueClear(nrrd);
  }
  return;
}

/*
******** nrrdBasicInfoCopy
**
** copies "basic" (per-array) information
** formerly known as nrrdPeripheralCopy, which was not used consistently
**
** the bitflag communicates which fields should *not* be copied
*/
int
nrrdBasicInfoCopy(Nrrd *dest, const Nrrd *src, int bitflag) {
  static const char me[]="nrrdBasicInfoCopy";
  unsigned int dd, ee;

  if (!( dest && src ))
    return 0;
  if (dest == src) {
    /* nothing to do */
    return 0;
  }

  if (!(NRRD_BASIC_INFO_DATA_BIT & bitflag)) {
    dest->data = src->data;
  }
  if (!(NRRD_BASIC_INFO_TYPE_BIT & bitflag)) {
    dest->type = src->type;
  }
  if (!(NRRD_BASIC_INFO_BLOCKSIZE_BIT & bitflag)) {
    dest->blockSize = src->blockSize;
  }
  if (!(NRRD_BASIC_INFO_DIMENSION_BIT & bitflag)) {
    dest->dim = src->dim;
  }
  if (!(NRRD_BASIC_INFO_CONTENT_BIT & bitflag)) {
    dest->content = (char *)airFree(dest->content);
    dest->content = airStrdup(src->content);
    if (src->content && !dest->content) {
      biffAddf(NRRD, "%s: couldn't copy content", me);
      return 1;
    }
  }
  if (!(NRRD_BASIC_INFO_SAMPLEUNITS_BIT & bitflag)) {
    dest->sampleUnits = (char *)airFree(dest->sampleUnits);
    dest->sampleUnits = airStrdup(src->sampleUnits);
    if (src->sampleUnits && !dest->sampleUnits) {
      biffAddf(NRRD, "%s: couldn't copy sampleUnits", me);
      return 1;
    }
  }
  if (!(NRRD_BASIC_INFO_SPACE_BIT & bitflag)) {
    dest->space = src->space;
  }
  if (!(NRRD_BASIC_INFO_SPACEDIMENSION_BIT & bitflag)) {
    dest->spaceDim = src->spaceDim;
  }
  if (!(NRRD_BASIC_INFO_SPACEUNITS_BIT & bitflag)) {
    for (dd=0; dd<src->spaceDim; dd++) {
      dest->spaceUnits[dd] = (char *)airFree(dest->spaceUnits[dd]);
      dest->spaceUnits[dd] = airStrdup(src->spaceUnits[dd]);
      if (src->spaceUnits[dd] && !dest->spaceUnits[dd]) {
        biffAddf(NRRD, "%s: couldn't copy spaceUnits[%d]", me, dd);
        return 1;
      }
    }
    for (dd=src->spaceDim; dd<NRRD_SPACE_DIM_MAX; dd++) {
      dest->spaceUnits[dd] = (char *)airFree(dest->spaceUnits[dd]);
    }
  }
  if (!(NRRD_BASIC_INFO_SPACEORIGIN_BIT & bitflag)) {
    for (dd=0; dd<NRRD_SPACE_DIM_MAX; dd++) {
      if (dd <= src->spaceDim-1) {
        dest->spaceOrigin[dd] = src->spaceOrigin[dd];
      } else {
        dest->spaceOrigin[dd] = AIR_NAN;
      }
    }
  }
  if (!(NRRD_BASIC_INFO_MEASUREMENTFRAME_BIT & bitflag)) {
    for (dd=0; dd<NRRD_SPACE_DIM_MAX; dd++) {
      for (ee=0; ee<NRRD_SPACE_DIM_MAX; ee++) {
        if (dd <= src->spaceDim-1 && ee <= src->spaceDim-1) {
          dest->measurementFrame[dd][ee] = src->measurementFrame[dd][ee];
        } else {
          dest->measurementFrame[dd][ee] = AIR_NAN;
        }
      }
    }
    for (dd=src->spaceDim; dd<NRRD_SPACE_DIM_MAX; dd++) {
      dest->spaceOrigin[dd] = AIR_NAN;
    }
  }
  if (!(NRRD_BASIC_INFO_OLDMIN_BIT & bitflag)) {
    dest->oldMin = src->oldMin;
  }
  if (!(NRRD_BASIC_INFO_OLDMAX_BIT & bitflag)) {
    dest->oldMax = src->oldMax;
  }
  if (!(NRRD_BASIC_INFO_COMMENTS_BIT & bitflag)) {
    if (nrrdCommentCopy(dest, src)) {
      biffAddf(NRRD, "%s: trouble copying comments", me);
      return 1;
    }
  }
  if (!(NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT & bitflag)) {
    if (nrrdKeyValueCopy(dest, src)) {
      biffAddf(NRRD, "%s: trouble copying key/value pairs", me);
      return 1;
    }
  }
  return 0;
}

/*
******* nrrdInit
**
** initializes a nrrd to default state.  All nrrd functions in the
** business of initializing a nrrd struct use this function.  Mostly
** just sets values to 0, NaN, "", NULL, or Unknown
*/
void
nrrdInit(Nrrd *nrrd) {
  int ii;

  if (nrrd) {
    nrrdBasicInfoInit(nrrd, NRRD_BASIC_INFO_NONE);
    for (ii=0; ii<NRRD_DIM_MAX; ii++) {
      _nrrdAxisInfoInit(nrrd->axis + ii);
    }
  }
  return;
}

/*
******** nrrdNew()
**
** creates and initializes a Nrrd
**
** this does NOT use biff
*/
Nrrd *
nrrdNew(void) {
  int ii;
  Nrrd *nrrd;
  airPtrPtrUnion appu;

  nrrd = (Nrrd*)(calloc(1, sizeof(Nrrd)));
  if (!nrrd) {
    return NULL;
  }

  /* explicitly set pointers to NULL, since calloc isn't officially
     guaranteed to do that.  */
  nrrd->data = NULL;
  for (ii=0; ii<NRRD_DIM_MAX; ii++) {
    _nrrdAxisInfoNewInit(nrrd->axis + ii);
  }
  for (ii=0; ii<NRRD_SPACE_DIM_MAX; ii++) {
    nrrd->spaceUnits[ii] = NULL;
  }
  nrrd->content = NULL;
  nrrd->sampleUnits = NULL;

  /* create comment airArray (even though it starts empty) */
  nrrd->cmt = NULL;
  appu.cp = &(nrrd->cmt);
  nrrd->cmtArr = airArrayNew(appu.v, NULL, sizeof(char *), NRRD_COMMENT_INCR);
  if (!nrrd->cmtArr) {
    return NULL;
  }
  airArrayPointerCB(nrrd->cmtArr, airNull, airFree);

  /* create key/value airArray (even thought it starts empty) */
  nrrd->kvp = NULL;
  appu.cp = &(nrrd->kvp);
  nrrd->kvpArr = airArrayNew(appu.v, NULL,
                             2*sizeof(char *), NRRD_KEYVALUE_INCR);
  if (!nrrd->kvpArr) {
    return NULL;
  }
  /* key/value airArray uses no callbacks for now */

  /* finish initializations */
  nrrdInit(nrrd);

  return nrrd;
}

/*
******** nrrdNix()
**
** does nothing with the array data inside, just does whatever is needed
** to free the nrrd itself
**
** returns NULL
**
** this does NOT use biff
*/
Nrrd *
nrrdNix(Nrrd *nrrd) {
  int ii;

  if (nrrd) {
    for (ii=0; ii<NRRD_DIM_MAX; ii++) {
      _nrrdAxisInfoInit(&(nrrd->axis[ii]));
    }
    for (ii=0; ii<NRRD_SPACE_DIM_MAX; ii++) {
      nrrd->spaceUnits[ii] = (char *)airFree(nrrd->spaceUnits[ii]);
    }
    nrrd->content = (char *)airFree(nrrd->content);
    nrrd->sampleUnits = (char *)airFree(nrrd->sampleUnits);
    nrrdCommentClear(nrrd);
    nrrd->cmtArr = airArrayNix(nrrd->cmtArr);
    nrrdKeyValueClear(nrrd);
    nrrd->kvpArr = airArrayNix(nrrd->kvpArr);
    airFree(nrrd);
  }
  return NULL;
}

/*
******** nrrdEmpty()
**
** frees data inside nrrd AND resets all its state, so its the
** same as what comes from nrrdNew().  This includes free()ing
** any comments.
*/
Nrrd *
nrrdEmpty(Nrrd *nrrd) {

  if (nrrd) {
    nrrd->data = airFree(nrrd->data);
    nrrdInit(nrrd);
  }
  return nrrd;
}

/*
******** nrrdNuke()
**
** blows away the nrrd and everything inside
**
** always returns NULL
*/
Nrrd *
nrrdNuke(Nrrd *nrrd) {

  if (nrrd) {
    nrrdEmpty(nrrd);
    nrrdNix(nrrd);
  }
  return NULL;
}

/* ------------------------------------------------------------ */

int
_nrrdSizeCheck(const size_t *size, unsigned int dim, int useBiff) {
  static const char me[]="_nrrdSizeCheck";
  size_t num, pre;
  unsigned int ai;

  pre = num = 1;
  for (ai=0; ai<dim; ai++) {
    if (!size[ai]) {
      biffMaybeAddf(useBiff, NRRD, "%s: axis %u size is zero!", me, ai);
      return 1;
    }
    num *= size[ai];
    if (num/size[ai] != pre) {
      biffMaybeAddf(useBiff, NRRD,
                    "%s: total # of elements too large to be represented in "
                    "type size_t, so too large for current architecture", me);
      return 1;
    }
    pre *= size[ai];
  }
  return 0;
}

/*
******** nrrdWrap_nva()
**
** wraps a given Nrrd around a given array
**
** we don't touch any of the peripheral information (content, comments,
** blocksize, min/max) because it is entirely reasonable to be setting
** this before or after this call.  "type" could be passed as
** nrrdTypeBlock, in which case it is the user's responsibility to
** set nrrd->blockSize at some other time.
*/
int
nrrdWrap_nva(Nrrd *nrrd, void *data, int type,
             unsigned int dim, const size_t *size) {
  static const char me[]="nrrdWrap_nva";

  if (!(nrrd && size)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  nrrd->data = data;
  nrrd->type = type;
  nrrd->dim = dim;
  if (_nrrdSizeCheck(size, dim, AIR_TRUE)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoSize, size);
  return 0;
}

/*
******** nrrdWrap_va()
**
** Minimal var args wrapper around nrrdWrap_nva, with the advantage of
** taking all the axes sizes as the var args.
**
** This is THE BEST WAY to wrap a nrrd around existing raster data,
** assuming that the dimension is known at compile time.
**
** If successful, returns 0, otherwise, 1.
** This does use biff.
*/
int
nrrdWrap_va(Nrrd *nrrd, void *data, int type, unsigned int dim, ...) {
  static const char me[]="nrrdWrap_va";
  va_list ap;
  size_t size[NRRD_DIM_MAX];
  unsigned int ai;

  if (!(nrrd && data)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  va_start(ap, dim);
  for (ai=0; ai<dim; ai++) {
    size[ai] = va_arg(ap, size_t);
  }
  va_end(ap);

  return nrrdWrap_nva(nrrd, data, type, dim, size);
}

/*
void
_nrrdTraverse(Nrrd *nrrd) {
  char *test, tval;
  size_t I, N;
  int S;

  N = nrrdElementNumber(nrrd);
  S = nrrdElementSize(nrrd);
  tval = 0;
  test = nrrd->data;
  for (I=0; I<N*S; I++) {
    tval += test[I];
  }
}
*/

int
_nrrdCopy(Nrrd *nout, const Nrrd *nin, int bitflag) {
  static const char me[]="_nrrdCopy";
  size_t size[NRRD_DIM_MAX];

  if (!(nin && nout)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == nin) {
    /* its not the case that we have nothing to do- the semantics of
       copying cannot be achieved if the input and output nrrd are
       the same; this is an error */
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  if (!nrrdElementSize(nin)) {
    biffAddf(NRRD, "%s: input nrrd reports zero element size!", me);
    return 1;
  }
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  if (nin->data) {
    if (nrrdMaybeAlloc_nva(nout, nin->type, nin->dim, size)) {
      biffAddf(NRRD, "%s: couldn't allocate data", me);
      return 1;
    }
    memcpy(nout->data, nin->data,
           nrrdElementNumber(nin)*nrrdElementSize(nin));
  } else {
    /* someone is trying to copy structs without data, fine fine fine */
    if (nrrdWrap_nva(nout, NULL, nin->type, nin->dim, size)) {
      biffAddf(NRRD, "%s: couldn't allocate data", me);
      return 1;
    }
  }
  nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_SIZE_BIT);
  /* if nin->data non-NULL (second branch above), this will
     harmlessly unset and set type and dim */
  nrrdBasicInfoInit(nout, NRRD_BASIC_INFO_DATA_BIT | bitflag);
  if (nrrdBasicInfoCopy(nout, nin, NRRD_BASIC_INFO_DATA_BIT | bitflag)) {
    biffAddf(NRRD, "%s: trouble copying basic info", me);
    return 1;
  }

  return 0;
}

/*
******** nrrdCopy
**
** copy method for nrrds.  nout will end up as an "exact" copy of nin.
** New space for data is allocated here, and output nrrd points to it.
** Comments from old are added to comments for new, so these are also
** newly allocated.  nout->ptr is not set, nin->ptr is not read.
*/
int
nrrdCopy(Nrrd *nout, const Nrrd *nin) {
  static const char me[]="nrrdCopy";

  if (_nrrdCopy(nout, nin, NRRD_BASIC_INFO_NONE)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  return 0;
}

/*
******** nrrdAlloc_nva()
**
** allocates data array and sets information.  If this is a block type
** nrrd, it is necessary to set nrrd->blockSize PRIOR to calling
** this function.
**
** This function will always allocate more memory (via calloc), but
** it will free() nrrd->data if it is non-NULL when passed in.
**
** This function takes the same "don't mess with peripheral information"
** attitude as nrrdWrap().
**
** Note to Gordon: don't get clever and change ANY axis-specific
** information here.  It may be very convenient to set that before
** nrrdAlloc or nrrdMaybeAlloc
**
** Note: This function DOES use biff
*/
int
nrrdAlloc_nva(Nrrd *nrrd, int type, unsigned int dim, const size_t *size) {
  static const char me[]="nrrdAlloc_nva";
  size_t num, esize;
  char stmp[2][AIR_STRLEN_SMALL];

  if (!(nrrd && size)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdType, type)) {
    biffAddf(NRRD, "%s: type (%d) is invalid", me, type);
    return 1;
  }
  if (nrrdTypeBlock == type) {
    if (!(0 < nrrd->blockSize)) {
      biffAddf(NRRD, "%s: given nrrd->blockSize %s invalid", me,
               airSprintSize_t(stmp[0], nrrd->blockSize));
      return 1;
    }
  }
  if (!AIR_IN_CL(1, dim, NRRD_DIM_MAX)) {
    biffAddf(NRRD, "%s: dim (%d) not in valid range [1,%d]",
             me, dim, NRRD_DIM_MAX);
    return 1;
  }

  nrrd->data = airFree(nrrd->data);
  if (nrrdWrap_nva(nrrd, NULL, type, dim, size)) {
    biffAddf(NRRD, "%s:", me);
    return 1 ;
  }
  num = nrrdElementNumber(nrrd);
  esize = nrrdElementSize(nrrd);
  nrrd->data = calloc(num, esize);
  if (!(nrrd->data)) {
    biffAddf(NRRD, "%s: calloc(%s,%s) failed", me,
             airSprintSize_t(stmp[0], num),
             airSprintSize_t(stmp[1], esize));
    return 1 ;
  }

  return 0;
}

/*
******** nrrdAlloc_va()
**
** Handy wrapper around nrrdAlloc_nva, which takes, as its vararg list,
** all the axes sizes.
*/
int
nrrdAlloc_va(Nrrd *nrrd, int type, unsigned int dim, ...) {
  static const char me[]="nrrdAlloc_va";
  size_t size[NRRD_DIM_MAX];
  unsigned int ai;
  va_list ap;

  if (!nrrd) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  va_start(ap, dim);
  for (ai=0; ai<dim; ai++) {
    size[ai] = va_arg(ap, size_t);
  }
  va_end(ap);
  if (nrrdAlloc_nva(nrrd, type, dim, size)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  return 0;
}


/*
** _nrrdMaybeAllocMaybeZero_nva
**
** New implementation of nrrdMaybeAlloc_nva, but now with ability
** to control whether or not to zero out when re-allocation wasn't needed
**
** HEY: should consider making this a public function, but GLK couldn't
** think of a name that wasn't silly
*/
int
_nrrdMaybeAllocMaybeZero_nva(Nrrd *nrrd, int type,
                             unsigned int dim, const size_t *size,
                             int zeroWhenNoAlloc) {
  static const char me[]="nrrdMaybeAllocMaybeZero_nva";
  size_t sizeWant, sizeHave, numWant, elementSizeWant;
  int need;
  unsigned int ai;

  if (!nrrd) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdType, type)) {
    biffAddf(NRRD, "%s: type (%d) is invalid", me, type);
    return 1;
  }
  if (nrrdTypeBlock == type) {
    if (nrrdTypeBlock == nrrd->type) {
      biffAddf(NRRD, "%s: can't change from one block nrrd to another", me);
      return 1;
    }
    if (!(0 < nrrd->blockSize)) {
      char stmp[AIR_STRLEN_SMALL];
      biffAddf(NRRD, "%s: given nrrd->blockSize %s invalid", me,
               airSprintSize_t(stmp, nrrd->blockSize));
      return 1;
    }
    elementSizeWant = nrrd->blockSize;
  } else {
    elementSizeWant = nrrdTypeSize[type];
  }
  if (_nrrdSizeCheck(size, dim, AIR_TRUE)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }

  if (!(nrrd->data)) {
    need = 1;
  } else {
    numWant = 1;
    for (ai=0; ai<dim; ai++) {
      numWant *= size[ai];
    }
    if (!nrrdElementSize(nrrd)) {
      biffAddf(NRRD, "%s: nrrd reports zero element size!", me);
      return 1;
    }
    sizeHave = nrrdElementNumber(nrrd) * nrrdElementSize(nrrd);
    /* fprintf(stderr, "##%s: sizeHave = %d * %d = %d\n", me,
            (int)(nrrdElementNumber(nrrd)),
            (int)(nrrdElementSize(nrrd)), (int)sizeHave); */
    sizeWant = numWant * elementSizeWant;
    /* fprintf(stderr, "##%s: sizeWant = %d * %d = %d\n", me,
            (int)(numWant),
            (int)(elementSizeWant), (int)sizeWant); */
    need = sizeHave != sizeWant;
    /* fprintf(stderr, "##%s: need = %d\n", me, need); */
  }
  if (need) {
    if (nrrdAlloc_nva(nrrd, type, dim, size)) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
  } else {
    /* size is already exactly what we want */
    if (nrrdWrap_nva(nrrd, nrrd->data, type, dim, size)) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
    /* but we may have to initialize memory */
    if (zeroWhenNoAlloc) {
      memset(nrrd->data, 0, nrrdElementNumber(nrrd)*nrrdElementSize(nrrd));
    }
  }

  return 0;
}

/*
******** nrrdMaybeAlloc_nva
**
** NOTE: this is now just a wrapper around _nrrdMaybeAllocMaybeZero_nva;
** below info referred to original implementation.
**
** calls nrrdAlloc_nva if the requested space is different than
** what is currently held
**
** also subscribes to the "don't mess with peripheral information" philosophy
*/
int
nrrdMaybeAlloc_nva(Nrrd *nrrd, int type,
                   unsigned int dim, const size_t *size) {
  static const char me[]="nrrdMaybeAlloc_nva";
  int ret;
  ret = _nrrdMaybeAllocMaybeZero_nva(nrrd, type, dim, size,
                                     AIR_TRUE);
  if (ret) {
    biffAddf(NRRD, "%s: trouble", me);
  }
  return ret;
}

/*
******** nrrdMaybeAlloc_va()
**
** Handy wrapper around nrrdAlloc, which takes, as its vararg list
** all the axes sizes, thereby calculating the total number.
*/
int
nrrdMaybeAlloc_va(Nrrd *nrrd, int type, unsigned int dim, ...) {
  static const char me[]="nrrdMaybeAlloc_va";
  size_t size[NRRD_DIM_MAX];
  unsigned int ai;
  va_list ap;

  if (!nrrd) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  va_start(ap, dim);
  for (ai=0; ai<dim; ai++) {
    size[ai] = va_arg(ap, size_t);
  }
  va_end(ap);
  if (nrrdMaybeAlloc_nva(nrrd, type, dim, size)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  return 0;
}

/* ---- BEGIN non-NrrdIO */

/*
** nrrdCompare
**
** walks through all fields of the two nrrds to see if they contain
** the same information.  So, this doesn't compare any pointer values,
** only the contents of things.
**
** Unlike strcmp(), the return value from this can't be the indication
** of the difference, because we'd have to return a value even in the
** case of an error, which would be be strange.  GLK briefly tried
** created a new #define _NRRD_ERROR_RETURN for this purpose (with
** leet-speak value 32202), but its just a bad idea.
**
** NOTE: the structure of this code is very similar to that of
** nrrdAxisInfoCompare, and any improvements here should be reflected there
*/
int
nrrdCompare(const Nrrd *ninA, const Nrrd *ninB,
            int onlyData, double epsilon,
            int *differ, char explain[AIR_STRLEN_LARGE]) {
  static const char me[]="nrrdCompare";
  size_t numA, numB;
  unsigned int axi, saxi;

  if (!(ninA && ninB && differ)) {
    biffAddf(NRRD, "%s: got NULL pointer (%p, %p, or %p)", me,
             AIR_CVOIDP(ninA), AIR_CVOIDP(ninB), AIR_VOIDP(differ));
    return 1;
  }

  if (explain) {
    strcpy(explain, "");
  }
  if (!onlyData) {
    if (ninA->dim != ninB->dim) {
      *differ = ninA->dim < ninB->dim ? -1 : 1;
      if (explain) {
        sprintf(explain, "nin{A,B}->dim %u %s %u",
                ninA->dim, *differ < 0 ? "<" : ">", ninB->dim);
      }
      return 0;
    }
  }
  if (ninA->type != ninB->type) {
    *differ = ninA->type < ninB->type ? -1 : 1;
    if (explain) {
      sprintf(explain, "nin{A,B}->type %s %s %s",
              airEnumStr(nrrdType, ninA->type), *differ < 0 ? "<" : ">",
              airEnumStr(nrrdType, ninB->type));
    }
    return 0;
  }
  numA = nrrdElementNumber(ninA);
  numB = nrrdElementNumber(ninB);
  if (numA != numB) {
    char stmp1[AIR_STRLEN_SMALL], stmp2[AIR_STRLEN_SMALL];
    *differ = numA < numB ? -1 : 1;
    sprintf(explain, "element # {A,B} %s %s %s",
            airSprintSize_t(stmp1, numA), *differ < 0 ? "<" : ">",
            airSprintSize_t(stmp2, numB));
    return 0;
  }
  /* this will always set *differ */
  if (nrrdArrayCompare(ninA->type, ninA->data, ninB->data, numA,
                       epsilon, differ, explain)) {
    biffAddf(NRRD, "%s: problem comparing values", me);
    return 1;
  }
  if (onlyData || *differ) {
    /* If caller only cared about data values,
       we're done whether or not they were different.
       else, if the data values are different, we're done.
       The explanation of any difference in values is
       already in "explain" (if non-NULL) */
    return 0;
  }

  for (axi=0; axi<ninA->dim; axi++) {
    /* this always sets *differ */
    if (nrrdAxisInfoCompare(ninA->axis + axi, ninB->axis + axi,
                            differ, explain)) {
      biffAddf(NRRD, "%s: problem comparing axis %u", me, axi);
      return 1;
    }
    if (*differ) {
      char tmpexplain[AIR_STRLEN_LARGE];
      /* the explanation, if wanted, is in "explain", but we add context */
      sprintf(tmpexplain, "(axis %u) %s", axi, explain);
      airStrcpy(explain, AIR_STRLEN_LARGE, tmpexplain);
      return 0;
    }
  }

#define STRING_COMPARE(VAL, STR)                                      \
  *differ = airStrcmp(ninA->VAL, ninB->VAL);                          \
  if (*differ) {                                                      \
    if (explain) {                                                    \
      /* can't print whole string because of fixed-size of explain */ \
      sprintf(explain, "ninA->%s %s ninB->%s",                        \
              STR, *differ < 0 ? "<" : ">", STR);                     \
    }                                                                 \
    return 0;                                                         \
  }

  STRING_COMPARE(content, "content");
  STRING_COMPARE(sampleUnits, "sampleUnits");
  if (ninA->space != ninB->space) {
    *differ = ninA->space < ninB->space ? -1 : 1;
    if (explain) {
      sprintf(explain, "ninA->space %s %s ninB->space %s",
              airEnumStr(nrrdSpace, ninA->space),
              *differ < 0 ? "<" : ">",
              airEnumStr(nrrdSpace, ninB->space));
    }
    return 0;
  }
  if (ninA->spaceDim != ninB->spaceDim) {
    *differ = ninA->spaceDim < ninB->spaceDim ? -1 : 1;
    if (explain) {
      sprintf(explain, "ninA->spaceDim %u %s ninB->spaceDim %u",
              ninA->spaceDim, *differ < 0 ? "<" : ">", ninB->spaceDim);
    }
    return 0;
  }
  if (ninA->blockSize != ninB->blockSize) {
    *differ = ninA->blockSize < ninB->blockSize ? -1 : 1;
    if (explain) {
      char stmp1[AIR_STRLEN_SMALL], stmp2[AIR_STRLEN_SMALL];
      sprintf(explain, "ninA->blockSize %s %s ninB->blockSize %s",
              airSprintSize_t(stmp1, ninA->blockSize),
              *differ < 0 ? "<" : ">",
              airSprintSize_t(stmp2, ninB->blockSize));
    }
    return 0;
  }

#define DOUBLE_COMPARE(VAL, STR)                                       \
  *differ = _nrrdDblcmp(ninA->VAL, ninB->VAL);                         \
  if (*differ) {                                                       \
    if (explain) {                                                     \
      sprintf(explain, "ninA->%s %.17g %s ninB->%s %.17g",             \
              STR, ninA->VAL, *differ < 0 ? "<" : ">",                 \
              STR, ninB->VAL);                                         \
    }                                                                  \
    return 0;                                                          \
  }

  for (saxi=0; saxi<NRRD_SPACE_DIM_MAX; saxi++) {
    char stmp[AIR_STRLEN_SMALL];
    unsigned int saxj;
    sprintf(stmp, "spaceOrigin[%u]", saxi);
    DOUBLE_COMPARE(spaceOrigin[saxi], stmp);
    sprintf(stmp, "spaceUnits[%u]", saxi);
    STRING_COMPARE(spaceUnits[saxi], stmp);
    for (saxj=0; saxj<NRRD_SPACE_DIM_MAX; saxj++) {
      sprintf(stmp, "measurementFrame[%u][%u]", saxi, saxj);
      DOUBLE_COMPARE(measurementFrame[saxi][saxj], stmp);
    }
  }
  DOUBLE_COMPARE(oldMin, "oldMin");
  DOUBLE_COMPARE(oldMax, "oldMax");
#undef DOUBLE_COMPARE

  if (ninA->cmtArr->len != ninB->cmtArr->len) {
    *differ = ninA->cmtArr->len < ninB->cmtArr->len ? -1 : 1;
    if (explain) {
      sprintf(explain, "ninA # comments %u %s ninB # comments %u",
              ninA->cmtArr->len, *differ < 0 ? "<" : ">", ninB->cmtArr->len);
    }
    return 0;
  } else {
    unsigned int ii;
    char stmp[AIR_STRLEN_SMALL];
    for (ii=0; ii<ninA->cmtArr->len; ii++) {
      sprintf(stmp, "comment[%u]", ii);
      STRING_COMPARE(cmt[ii], stmp);
    }
  }
  if (ninA->kvpArr->len != ninB->kvpArr->len) {
    *differ = ninA->kvpArr->len < ninB->kvpArr->len ? -1 : 1;
    if (explain) {
      sprintf(explain, "ninA # key/values %u %s ninB # key/values %u",
              ninA->kvpArr->len, *differ < 0 ? "<" : ">", ninB->kvpArr->len);
    }
    return 0;
  } else {
    unsigned int ii;
    char stmp[AIR_STRLEN_SMALL];
    for (ii=0; ii<ninA->kvpArr->len; ii++) {
      sprintf(stmp, "key/value key[%u]", ii);
      STRING_COMPARE(kvp[2*ii + 0], stmp);
      sprintf(stmp, "key/value value[%u]", ii);
      STRING_COMPARE(kvp[2*ii + 1], stmp);
    }
  }
#undef STRING_COMPARE
  /* ninA->ptr and ninB->ptr are not to be accessed */

  /* if we've gotten this far, all fields were equal */
  *differ = 0;
  return 0;
}

/*
******** nrrdPPM()
**
** for making a nrrd suitable for holding PPM data
**
** "don't mess with peripheral information"
*/
int
nrrdPPM(Nrrd *ppm, size_t sx, size_t sy) {
  static const char me[]="nrrdPPM";
  char stmp[2][AIR_STRLEN_SMALL];

  if (nrrdMaybeAlloc_va(ppm, nrrdTypeUChar, 3,
                        AIR_CAST(size_t, 3), sx, sy)) {
    biffAddf(NRRD, "%s: couldn't allocate %s x %s 24-bit image", me,
             airSprintSize_t(stmp[0], sx),
             airSprintSize_t(stmp[1], sy));
    return 1;
  }
  return 0;
}

/*
******** nrrdPGM()
**
** for making a nrrd suitable for holding PGM data
**
** "don't mess with peripheral information"
*/
int
nrrdPGM(Nrrd *pgm, size_t sx, size_t sy) {
  static const char me[]="nrrdPGM";
  char stmp[2][AIR_STRLEN_SMALL];

  if (nrrdMaybeAlloc_va(pgm, nrrdTypeUChar, 2,
                        sx, sy)) {
    biffAddf(NRRD, "%s: couldn't allocate %s x %s 8-bit image", me,
             airSprintSize_t(stmp[0], sx),
             airSprintSize_t(stmp[1], sy));
    return 1;
  }
  return 0;
}

/* ---- END non-NrrdIO */
