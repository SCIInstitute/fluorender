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

#if TEEM_FFTW3 /* =========================================================== */

#include <fftw3.h>

const int nrrdFFTWEnabled = AIR_TRUE;

int
nrrdFFTWWisdomRead(FILE *file) {
  static const char me[]="nrrdFFTWWisdomRead";

  if (!(file)) {
    biffAddf(NRRD, "%s: given file NULL", me);
    return 1;
  }
  if (!fftw_import_wisdom_from_file(file)) {
    biffAddf(NRRD, "%s: trouble importing wisdom", me);
    return 1;
  }
  return 0;
}

static void *
_nrrdFftwFreeWrapper(void *ptr) {
  fftw_free(ptr);
  return NULL;
}

static void
_nrrdDimsReverse(fftw_iodim *dims, unsigned int len) {
  fftw_iodim buff[NRRD_DIM_MAX];
  unsigned int ii;
  for (ii=0; ii<len; ii++) {
    buff[len-1-ii].n = dims[ii].n;
    buff[len-1-ii].is = dims[ii].is;
    buff[len-1-ii].os = dims[ii].os;
  }
  for (ii=0; ii<len; ii++) {
    dims[ii].n = buff[ii].n;
    dims[ii].is = buff[ii].is;
    dims[ii].os = buff[ii].os;
  }
}

/*
******** nrrdFFT
**
** First pass at a wrapper around FFTW.  This was implemented out of need for a
** specific project; and better decisions and different interfaces will become
** apparent with time and experience; these can be in Teem 2.0.
**
** currently *requires* that input be complex-valued, in that axis 0 has to
** have size 2.  nrrdKindComplex would be sensible for input axis 0 but we don't
** require it, though it is set on the output.
*/
int
nrrdFFT(Nrrd *nout, const Nrrd *_nin,
        unsigned int *axes, unsigned int axesNum,
        int sign, int rescale, int rigor) {
  static const char me[]="nrrdFFT";
  size_t inSize[NRRD_DIM_MAX], II, NN, nprod;
  double *inData, *outData;
  airArray *mop;
  Nrrd *nin;
  unsigned int axi, axisDo[NRRD_DIM_MAX];
  fftw_plan plan;
  void *dataBef;
  unsigned int txfRank, howRank, flags;
  size_t stride;
  fftw_iodim txfDims[NRRD_DIM_MAX], howDims[NRRD_DIM_MAX];

  if (!(nout && _nin && axes)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( _nin->dim > 1 && 2 == _nin->axis[0].size )) {
    biffAddf(NRRD, "%s: nin doesn't look like a complex-valued array", me);
    return 1;
  }
  if (!( axesNum >= 1 )) {
    biffAddf(NRRD, "%s: axesNum 0, no axes to transform?", me);
    return 1;
  }
  for (axi=0; axi<_nin->dim; axi++) {
    axisDo[axi] = 0;
  }
  for (axi=0; axi<axesNum; axi++) {
    if (0 == axes[axi]) {
      biffAddf(NRRD, "%s: can't transform axis 0 (axes[%u]) for "
               "real/complex values", me, axi);
      return 1;
    }
    if (!( axes[axi] < _nin->dim )) {
      biffAddf(NRRD, "%s: axis %u (axes[%u]) out of range [1,%u]", me,
               axes[axi], axi, _nin->dim-1);
      return 1;
    }
    axisDo[axes[axi]]++;
    if (2 == axisDo[axes[axi]]) {
      biffAddf(NRRD, "%s: axis %u (axes[%u]) already transformed",
               me, axes[axi], axi);
      return 1;
    }
  }

  NN = nrrdElementNumber(_nin);
  /* We always make a new buffer to hold the double-type copy of input for two
     reasons: if input is not double we have to convert it, and we want input
     to be const, and we can't have const with the plan creation over-writing
     the input (except with FFTW_ESTIMATE).  Given that, we might as well use
     the memory-alignment-savvy fftw_malloc; the freeing is handled by both
     _nrrdFftwFreeWrapper and nrrdNix. */
  /* (NN = 2 * number of complex values) */
  inData = AIR_CAST(double *, fftw_malloc(NN*sizeof(double)));
  if (!inData) {
    biffAddf(NRRD, "%s: couldn't allocate input data copy", me);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, inData, _nrrdFftwFreeWrapper, airMopAlways);
  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNix /* NOT Nuke */, airMopAlways);
  nrrdAxisInfoGet_nva(_nin, nrrdAxisInfoSize, inSize);
  /* we don't copy data yet; it may be over-written during plan creation */
  if (nrrdWrap_nva(nin, inData, nrrdTypeDouble, _nin->dim, inSize)) {
    biffAddf(NRRD, "%s: couldn't wrap or copy input", me);
    airMopError(mop);
    return 1;
  }
  /* But on the output, we just use regular malloc, because we don't (yet) have
     a way of telling nrrd to use fftw_malloc/fftw_free instead of the generic
     malloc/free, and we don't want two whole copies of the output (one that is
     memory-aligned, internal to this function, and one that isn't, in nout) */
  if (nrrdMaybeAlloc_nva(nout, nrrdTypeDouble, _nin->dim, inSize)) {
    biffAddf(NRRD, "%s: couldn't allocate output", me);
    airMopError(mop);
    return 1;
  }
  outData = AIR_CAST(double *, nout->data);

  /* As far as GLK can tell, the guru interface is needed, and the "advanced"
     fftw_plan_many_dft won't work, because its simplistic accounting of stride
     can't handle having non-contiguous non-transformed axes (e.g. transforming
     only axes 2 and not 1, 3 in a 3-D complex-valued array) */
  txfRank = howRank = 0;
  stride = 1;
  nprod = 1;
  for (axi=1; axi<nin->dim; axi++) {
    if (axisDo[axi]) {
      txfDims[txfRank].n = AIR_CAST(int, inSize[axi]);
      txfDims[txfRank].is = txfDims[txfRank].os = AIR_CAST(int, stride);
      nprod *= inSize[axi];
      txfRank++;
    } else {
      howDims[howRank].n = AIR_CAST(int, inSize[axi]);
      howDims[howRank].is = howDims[howRank].os = AIR_CAST(int, stride);
      howRank++;
    }
    stride *= inSize[axi];
  }
  _nrrdDimsReverse(txfDims, txfRank);
  _nrrdDimsReverse(howDims, howRank);

  /*
  fprintf(stderr, "!%s: ------------- txfRank %u, howRank %u\n",
          me, txfRank, howRank);
  for (axi=0; axi<txfRank; axi++) {
    fprintf(stderr, "!%s: txfDims[%u]: n %d; s %d\n", me, axi,
            txfDims[axi].n, txfDims[axi].is);
  }
  for (axi=0; axi<howRank; axi++) {
    fprintf(stderr, "!%s: howDims[%u]: n %d; s %d\n", me, axi,
            howDims[axi].n, howDims[axi].is);
  }
  */

  switch (rigor) {
  case nrrdFFTWPlanRigorEstimate:
    flags = FFTW_ESTIMATE;
    break;
  case nrrdFFTWPlanRigorMeasure:
    flags = FFTW_MEASURE;
    break;
  case nrrdFFTWPlanRigorPatient:
    flags = FFTW_PATIENT;
    break;
  case nrrdFFTWPlanRigorExhaustive:
    flags = FFTW_EXHAUSTIVE;
    break;
  default:
    biffAddf(NRRD, "%s: unsupported rigor %d", me, rigor);
    airMopError(mop); return 1;
  }
  /* HEY: figure out why fftw expects txfRank and howRank to be
     signed and not unsigned */
  plan = fftw_plan_guru_dft(AIR_CAST(int, txfRank), txfDims,
                            AIR_CAST(int, howRank), howDims,
                            AIR_CAST(fftw_complex *, inData),
                            AIR_CAST(fftw_complex *, outData),
                            sign, flags);
  if (!plan) {
    biffAddf(NRRD, "%s: unable to create plan", me);
    airMopError(mop); return 1;
  }

  /* only after planning is done (which can over-write contents of inData)
     do we copy the real input values over */
  dataBef = nin->data;
  if (nrrdConvert(nin, _nin, nrrdTypeDouble)) {
    biffAddf(NRRD, "%s: couldn't initialize input", me);
    airMopError(mop); return 1;
  }
  /* a reasonable assert: data buffer was allocated correctly */
  if (dataBef != nin->data) {
    biffAddf(NRRD, "%s: pre-allocation error; nrrdConvert reallocated data",
             me);
    airMopError(mop); return 1;
  }

  /* run the transform; HEY, no indication of success? */
  fftw_execute(plan);

  /* if wanted, remove the sqrt(nprod) scaling that fftw adds at each pass */
  if (rescale) {
    double scale;
    scale = sqrt(1.0/AIR_CAST(double, nprod));
    for (II=0; II<NN; II++) {
      outData[II] *= scale;
    }
  }

  if (nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE)
      || nrrdBasicInfoCopy(nout, nin,
                           NRRD_BASIC_INFO_DATA_BIT
                           | NRRD_BASIC_INFO_TYPE_BIT
                           | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                           | NRRD_BASIC_INFO_DIMENSION_BIT
                           | NRRD_BASIC_INFO_CONTENT_BIT
                           | NRRD_BASIC_INFO_COMMENTS_BIT
                           | (nrrdStateKeyValuePairsPropagate
                              ? 0
                              : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  nout->axis[0].kind = nrrdKindComplex;

  airMopOkay(mop);
  return 0;
}

int
nrrdFFTWWisdomWrite(FILE *file) {
  static const char me[]="nrrdFFTWWisdomWrite";

  if (!(file)) {
    biffAddf(NRRD, "%s: given file NULL", me);
    return 1;
  }
  /* HEY: weird that there's no return value on this function? */
  fftw_export_wisdom_to_file(file);
  return 0;
}


#else /* TEEM_FFTW3 ========================================================= */

/* we do NOT have the FFTW library to link against; have to
   supply the same symbols anyway */

const int nrrdFFTWEnabled = AIR_FALSE;

int
nrrdFFTWWisdomRead(FILE *file) {
  AIR_UNUSED(file);
  return 0;
}

int
nrrdFFT(Nrrd *nout, const Nrrd *nin,
        unsigned int *axes, unsigned int axesNum,
        int sign, int rescale, int rigor) {
  static const char me[]="nrrdFFT";

  AIR_UNUSED(nout);
  AIR_UNUSED(nin);
  AIR_UNUSED(axes);
  AIR_UNUSED(axesNum);
  AIR_UNUSED(sign);
  AIR_UNUSED(rescale);
  AIR_UNUSED(rigor);
  biffAddf(NRRD, "%s: sorry, non-fftw3 version not yet implemented\n", me);
  return 1;
}

int
nrrdFFTWWisdomWrite(FILE *file) {
  AIR_UNUSED(file);
  return 0;
}

#endif /* TEEM_FFTW3 ======================================================== */

