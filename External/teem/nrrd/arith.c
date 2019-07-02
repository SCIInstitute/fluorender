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
******** nrrdArithGamma()
**
** map the values in a nrrd through a power function; essentially:
** val = pow(val, 1/gamma), but this is after the val has been normalized
** to be in the range of 0.0 to 1.0 (assuming that the given min and
** max really are the full range of the values in the nrrd).  Thus,
** the given min and max values are fixed points of this
** transformation.  Using a negative gamma means that after the pow()
** function has been applied, the value is inverted with respect to
** min and max (like in xv).
*/
int
nrrdArithGamma(Nrrd *nout, const Nrrd *nin,
               const NrrdRange *_range, double gamma) {
  static const char me[]="nrrdArithGamma", func[]="gamma";
  double val, min, max;
  size_t I, num;
  NrrdRange *range;
  airArray *mop;
  double (*lup)(const void *, size_t);
  double (*ins)(void *, size_t, double);

  if (!(nout && nin)) {
    /* _range can be NULL */
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( AIR_EXISTS(gamma) )) {
    biffAddf(NRRD, "%s: gamma doesn't exist", me);
    return 1;
  }
  if (!( nrrdTypeBlock != nin->type && nrrdTypeBlock != nout->type )) {
    biffAddf(NRRD, "%s: can't deal with %s type", me,
             airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      biffAddf(NRRD, "%s: couldn't initialize by copy to output", me);
      return 1;
    }
  }
  mop = airMopNew();
  if (_range) {
    range = nrrdRangeCopy(_range);
    nrrdRangeSafeSet(range, nin, nrrdBlind8BitRangeState);
  } else {
    range = nrrdRangeNewSet(nin, nrrdBlind8BitRangeTrue);
  }
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  min = range->min;
  max = range->max;
  if (min == max) {
    /* this is stupid.  We want min < max to avoid making NaNs */
    max += 1;
  }
  lup = nrrdDLookup[nin->type];
  ins = nrrdDInsert[nout->type];
  gamma = 1/gamma;
  num = nrrdElementNumber(nin);
  if (gamma < 0.0) {
    gamma = -gamma;
    for (I=0; I<num; I++) {
      val = lup(nin->data, I);
      val = AIR_AFFINE(min, val, max, 0.0, 1.0);
      val = pow(val, gamma);
      val = AIR_AFFINE(1.0, val, 0.0, min, max);
      ins(nout->data, I, val);
    }
  } else {
    for (I=0; I<num; I++) {
      val = lup(nin->data, I);
      val = AIR_AFFINE(min, val, max, 0.0, 1.0);
      val = pow(val, gamma);
      val = AIR_AFFINE(0.0, val, 1.0, min, max);
      ins(nout->data, I, val);
    }
  }
  if (nrrdContentSet_va(nout, func, nin, "%g,%g,%g", min, max, gamma)) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  if (nout != nin) {
    nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
  }
  /* basic info handled by nrrdCopy above */

  airMopOkay(mop);
  return 0;
}

/* ---------------------------- unary -------------- */

static double _nrrdUnaryOpNegative(double a)   {return -a;}
static double _nrrdUnaryOpReciprocal(double a) {return 1.0/a;}
static double _nrrdUnaryOpSin(double a)        {return sin(a);}
static double _nrrdUnaryOpCos(double a)        {return cos(a);}
static double _nrrdUnaryOpTan(double a)        {return tan(a);}
static double _nrrdUnaryOpAsin(double a)       {return asin(a);}
static double _nrrdUnaryOpAcos(double a)       {return acos(a);}
static double _nrrdUnaryOpAtan(double a)       {return atan(a);}
static double _nrrdUnaryOpExp(double a)        {return exp(a);}
static double _nrrdUnaryOpLog(double a)        {return log(a);}
static double _nrrdUnaryOpLog2(double a)       {return log(a)/0.69314718;}
static double _nrrdUnaryOpLog10(double a)      {return log10(a);}
/* This code for log1p and expm1 comes from
   http://www.plunk.org/~hatch/rightway.php which in turn references
   http://www.cs.berkeley.edu/~wkahan/Math128/Sumnfp.pdf from the
   great Kahan of IEEE 754 fame, but sadly that URL no longer works
   (though the Math128 directory is still there, as are other documents) */
static double _nrrdUnaryOpLog1p(double a) {
  double b;

  b = 1.0 + a;
  if (b == 1.0) {
    /* "a" was so close to zero that we had underflow when adding to 1,
       resulting in something that is exactly equal to 1.  So, use the
       first term of Taylor expansion of log(x+1) around 0 == x */
    return a;
  }
  /* else "a" was not so near zero; but GLK doesn't fully grasp
     the design of this expression */
  return log(b)*a/(b-1);
}
static double _nrrdUnaryOpExpm1(double x) {
  double u;

  u = exp(x);
  if (u == 1.0) {
    /* "x" was so close to 0.0 that exp return exactly 1; subtracting
       1 from this will give a constant for a range of x's.  Instead,
       use the Taylor expansion of exp(x)-1 around 0 == x */
    return x;
  } else if (u-1.0 == -1.0) {
    /* "x" was close enough to -inf that exp returned something so close
       to 0 that subtracting 1 resulted in exactly -1; return that */
    return -1.0;
  }
  /* else "x" was neither near 0.0 or -inf, but GLK doesn't fully grasp
     the design of this expression */
  return (u-1.0)*x/log(u);
}
static double _nrrdUnaryOpSqrt(double a)       {return sqrt(a);}
static double _nrrdUnaryOpCbrt(double a)       {return airCbrt(a);}
static double _nrrdUnaryOpErf(double a)        {return airErf(a);}
static double _nrrdUnaryOpNerf(double a)       {return (1+airErf(a))/2;}
static double _nrrdUnaryOpCeil(double a)       {return ceil(a);}
static double _nrrdUnaryOpFloor(double a)      {return floor(a);}
static double _nrrdUnaryOpRoundUp(double a)    {return AIR_ROUNDUP(a);}
static double _nrrdUnaryOpRoundDown(double a)  {return AIR_ROUNDDOWN(a);}
static double _nrrdUnaryOpAbs(double a)        {return AIR_ABS(a);}
static double _nrrdUnaryOpSgn(double a) {
  return (a < 0.0 ? -1 : (a > 0.0 ? 1 : 0));}
static double _nrrdUnaryOpExists(double a)     {return AIR_EXISTS(a);}
static double _nrrdUnaryOpRand(double a) {
  AIR_UNUSED(a);
  return airDrandMT();
}
static double _nrrdUnaryOpNormalRand(double a) {
  double v;
  AIR_UNUSED(a);
  airNormalRand(&v, NULL);
  return v;
}
static double _nrrdUnaryOpIf(double a) { return (a ? 1 : 0); }
static double _nrrdUnaryOpZero(double a) {
  AIR_UNUSED(a);
  return 0.0;
}
static double _nrrdUnaryOpOne(double a) {
  AIR_UNUSED(a);
  return 1.0;
}

double (*_nrrdUnaryOp[NRRD_UNARY_OP_MAX+1])(double) = {
  NULL,
  _nrrdUnaryOpNegative,
  _nrrdUnaryOpReciprocal,
  _nrrdUnaryOpSin,
  _nrrdUnaryOpCos,
  _nrrdUnaryOpTan,
  _nrrdUnaryOpAsin,
  _nrrdUnaryOpAcos,
  _nrrdUnaryOpAtan,
  _nrrdUnaryOpExp,
  _nrrdUnaryOpLog,
  _nrrdUnaryOpLog2,
  _nrrdUnaryOpLog10,
  _nrrdUnaryOpLog1p,
  _nrrdUnaryOpExpm1,
  _nrrdUnaryOpSqrt,
  _nrrdUnaryOpCbrt,
  _nrrdUnaryOpErf,
  _nrrdUnaryOpNerf,
  _nrrdUnaryOpCeil,
  _nrrdUnaryOpFloor,
  _nrrdUnaryOpRoundUp,
  _nrrdUnaryOpRoundDown,
  _nrrdUnaryOpAbs,
  _nrrdUnaryOpSgn,
  _nrrdUnaryOpExists,
  _nrrdUnaryOpRand,
  _nrrdUnaryOpNormalRand,
  _nrrdUnaryOpIf,
  _nrrdUnaryOpZero,
  _nrrdUnaryOpOne
};

int
nrrdArithUnaryOp(Nrrd *nout, int op, const Nrrd *nin) {
  static const char me[]="nrrdArithUnaryOp";
  size_t N, I;
  int size[NRRD_DIM_MAX];
  double (*insert)(void *v, size_t I, double d),
    (*lookup)(const void *v, size_t I), (*uop)(double), val;

  if (!(nout && nin)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    biffAddf(NRRD, "%s: can't operate on type %s", me,
             airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }
  if (airEnumValCheck(nrrdUnaryOp, op)) {
    biffAddf(NRRD, "%s: unary op %d invalid", me, op);
    return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
  }
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  uop = _nrrdUnaryOp[op];

  N = nrrdElementNumber(nin);
  lookup = nrrdDLookup[nin->type];
  insert = nrrdDInsert[nin->type];
  for (I=0; I<N; I++) {
    val = lookup(nin->data, I);
    insert(nout->data, I, uop(val));
  }
  if (nrrdContentSet_va(nout, airEnumStr(nrrdUnaryOp, op), nin, "")) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }
  nrrdBasicInfoInit(nout,
                    NRRD_BASIC_INFO_ALL ^ (NRRD_BASIC_INFO_OLDMIN_BIT
                                           | NRRD_BASIC_INFO_OLDMAX_BIT));
  return 0;
}

/* ---------------------------- binary -------------- */

static double _nrrdBinaryOpAdd(double a, double b)       {return a + b;}
static double _nrrdBinaryOpSubtract(double a, double b)  {return a - b;}
static double _nrrdBinaryOpMultiply(double a, double b)  {return a * b;}
static double _nrrdBinaryOpDivide(double a, double b)    {return a / b;}
static double _nrrdBinaryOpPow(double a, double b)       {return pow(a,b);}
static double _nrrdBinaryOpSgnPow(double a, double b)  {return airSgnPow(a,b);}
static double _nrrdBinaryOpFlippedSgnPow(double a, double b)  {return airFlippedSgnPow(a,b);}
static double _nrrdBinaryOpMod(double a, double b) {
  return AIR_MOD((int)a,(int)b);}
static double _nrrdBinaryOpFmod(double a, double b)      {return fmod(a,b);}
static double _nrrdBinaryOpAtan2(double a, double b)     {return atan2(a,b);}
static double _nrrdBinaryOpMin(double a, double b)       {return AIR_MIN(a,b);}
static double _nrrdBinaryOpMax(double a, double b)       {return AIR_MAX(a,b);}
static double _nrrdBinaryOpLT(double a, double b)        {return (a < b);}
static double _nrrdBinaryOpLTE(double a, double b)       {return (a <= b);}
static double _nrrdBinaryOpGT(double a, double b)        {return (a > b);}
static double _nrrdBinaryOpGTE(double a, double b)       {return (a >= b);}
static double _nrrdBinaryOpCompare(double a, double b) {
  return (a < b ? -1 : (a > b ? 1 : 0));}
static double _nrrdBinaryOpEqual(double a, double b)     {return (a == b);}
static double _nrrdBinaryOpNotEqual(double a, double b)  {return (a != b);}
static double _nrrdBinaryOpExists(double a, double b)  {return (AIR_EXISTS(a)
                                                                ? a : b);}
static double _nrrdBinaryOpIf(double a, double b)        {return (a ? a : b);}
static double _nrrdBinaryOpNormalRandScaleAdd(double a, double b) {
  double v;
  airNormalRand(&v, NULL);
  return a + b*v;
}
static double _nrrdBinaryOpRicianRand(double a, double b) {
  double vr, vi, rr, ri;
  airNormalRand(&rr, &ri);
  vr = a + b*rr;
  vi = b*ri;
  return sqrt(vr*vr + vi*vi);
}


double (*_nrrdBinaryOp[NRRD_BINARY_OP_MAX+1])(double, double) = {
  NULL,
  _nrrdBinaryOpAdd,
  _nrrdBinaryOpSubtract,
  _nrrdBinaryOpMultiply,
  _nrrdBinaryOpDivide,
  _nrrdBinaryOpPow,
  _nrrdBinaryOpSgnPow,
  _nrrdBinaryOpFlippedSgnPow,
  _nrrdBinaryOpMod,
  _nrrdBinaryOpFmod,
  _nrrdBinaryOpAtan2,
  _nrrdBinaryOpMin,
  _nrrdBinaryOpMax,
  _nrrdBinaryOpLT,
  _nrrdBinaryOpLTE,
  _nrrdBinaryOpGT,
  _nrrdBinaryOpGTE,
  _nrrdBinaryOpCompare,
  _nrrdBinaryOpEqual,
  _nrrdBinaryOpNotEqual,
  _nrrdBinaryOpExists,
  _nrrdBinaryOpIf,
  _nrrdBinaryOpNormalRandScaleAdd,
  _nrrdBinaryOpRicianRand
};

/*
******** nrrdArithBinaryOp
**
** this is a simplified version of nrrdArithIterBinaryOp, written after
** that, in a hurry, to operate directly on two nrrds, instead with
** the NrrdIter nonsense
*/
int
nrrdArithBinaryOp(Nrrd *nout, int op, const Nrrd *ninA, const Nrrd *ninB) {
  static const char me[]="nrrdArithBinaryOp";
  char *contA, *contB;
  size_t N, I, size[NRRD_DIM_MAX];
  double (*ins)(void *v, size_t I, double d),
    (*lupA)(const void *v, size_t I), (*lupB)(const void *v, size_t I),
    (*bop)(double a, double b), valA, valB;

  if (!( nout && !nrrdCheck(ninA) && !nrrdCheck(ninB) )) {
    biffAddf(NRRD, "%s: NULL pointer or invalid args", me);
    return 1;
  }
  if (nrrdTypeBlock == ninA->type || nrrdTypeBlock == ninB->type) {
    biffAddf(NRRD, "%s: can't operate on type %s", me,
             airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }
  if (!nrrdSameSize(ninA, ninB, AIR_TRUE)) {
    biffAddf(NRRD, "%s: size mismatch between arguments", me);
    return 1;
  }
  if (airEnumValCheck(nrrdBinaryOp, op)) {
    biffAddf(NRRD, "%s: binary op %d invalid", me, op);
    return 1;
  }

  nrrdAxisInfoGet_nva(ninA, nrrdAxisInfoSize, size);
  if (!( nout == ninA || nout == ninB)) {
    if (_nrrdMaybeAllocMaybeZero_nva(nout, ninA->type, ninA->dim, size,
                                     AIR_FALSE /* zero when no realloc */)) {
      biffAddf(NRRD, "%s: couldn't allocate output nrrd", me);
      return 1;
    }
    if (nrrdAxisInfoCopy(nout, ninA, NULL, NRRD_AXIS_INFO_NONE)) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
    nrrdBasicInfoCopy(nout, ninA, (NRRD_BASIC_INFO_DATA_BIT
                                   | NRRD_BASIC_INFO_TYPE_BIT
                                   | NRRD_BASIC_INFO_DIMENSION_BIT
                                   | NRRD_BASIC_INFO_CONTENT_BIT
                                   | NRRD_BASIC_INFO_COMMENTS_BIT
                                   | (nrrdStateKeyValuePairsPropagate
                                      ? 0
                                      : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT)));
  }
  nrrdBasicInfoInit(nout,
                    NRRD_BASIC_INFO_ALL ^ (NRRD_BASIC_INFO_OLDMIN_BIT
                                           | NRRD_BASIC_INFO_OLDMAX_BIT));
  bop = _nrrdBinaryOp[op];

  N = nrrdElementNumber(ninA);
  lupA = nrrdDLookup[ninA->type];
  lupB = nrrdDLookup[ninB->type];
  ins = nrrdDInsert[nout->type];
  for (I=0; I<N; I++) {
    /* HEY: there is a loss of precision issue here with 64-bit ints */
    valA = lupA(ninA->data, I);
    valB = lupB(ninB->data, I);
    ins(nout->data, I, bop(valA, valB));
  }

  contA = _nrrdContentGet(ninA);
  contB = _nrrdContentGet(ninB);
  if (_nrrdContentSet_va(nout, airEnumStr(nrrdBinaryOp, op),
                         contA, "%s", contB)) {
    biffAddf(NRRD, "%s:", me);
    free(contA); free(contB); return 1;
  }
  free(contA);
  free(contB);
  return 0;
}

int
nrrdArithIterBinaryOpSelect(Nrrd *nout, int op,
                            NrrdIter *inA, NrrdIter *inB,
                            unsigned int which) {
  static const char me[]="nrrdArithIterBinaryOpSelect";
  char *contA, *contB;
  size_t N, I, size[NRRD_DIM_MAX];
  int type;
  double (*insert)(void *v, size_t I, double d),
    (*bop)(double a, double b), valA, valB;
  const Nrrd *nin;

  if (!(nout && inA && inB)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdBinaryOp, op)) {
    biffAddf(NRRD, "%s: binary op %d invalid", me, op);
    return 1;
  }
  if (!( 0 == which || 1 == which )) {
    biffAddf(NRRD, "%s: which %u not 0 or 1", me, which);
    return 1;
  }
  nin = (0 == which
         ? _NRRD_ITER_NRRD(inA)
         : _NRRD_ITER_NRRD(inB));
  if (!nin) {
    biffAddf(NRRD, "%s: selected input %u is a fixed value", me, which);
    return 1;
  }
  type = nin->type;
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);

  if (_nrrdMaybeAllocMaybeZero_nva(nout, type, nin->dim, size,
                                   AIR_FALSE /* zero when no realloc */)) {
    biffAddf(NRRD, "%s: couldn't allocate output nrrd", me);
    return 1;
  }
  nrrdBasicInfoCopy(nout, nin, (NRRD_BASIC_INFO_DATA_BIT
                                | NRRD_BASIC_INFO_TYPE_BIT
                                | NRRD_BASIC_INFO_DIMENSION_BIT
                                | NRRD_BASIC_INFO_CONTENT_BIT
                                | NRRD_BASIC_INFO_COMMENTS_BIT
                                | (nrrdStateKeyValuePairsPropagate
                                   ? 0
                                   : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT)));
  nrrdBasicInfoInit(nout,
                    NRRD_BASIC_INFO_ALL ^ (NRRD_BASIC_INFO_OLDMIN_BIT
                                           | NRRD_BASIC_INFO_OLDMAX_BIT));
  bop = _nrrdBinaryOp[op];

  /*
  fprintf(stderr, "%s: inA->left = %d, inB->left = %d\n", me,
          (int)(inA->left), (int)(inB->left));
  */
  N = nrrdElementNumber(nin);
  insert = nrrdDInsert[type];
  for (I=0; I<N; I++) {
    /* HEY: there is a loss of precision issue here with 64-bit ints */
    valA = nrrdIterValue(inA);
    valB = nrrdIterValue(inB);
    insert(nout->data, I, bop(valA, valB));
  }
  contA = nrrdIterContent(inA);
  contB = nrrdIterContent(inB);
  if (_nrrdContentSet_va(nout, airEnumStr(nrrdBinaryOp, op),
                         contA, "%s", contB)) {
    biffAddf(NRRD, "%s:", me);
    free(contA); free(contB); return 1;
  }
  if (nout != nin) {
    nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
  }
  free(contA);
  free(contB);
  return 0;
}

int
nrrdArithIterBinaryOp(Nrrd *nout, int op, NrrdIter *inA, NrrdIter *inB) {
  static const char me[]="nrrdArithIterBinaryOp";
  unsigned int which;

  if (!(nout && inA && inB)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  which = (_NRRD_ITER_NRRD(inA)
           ? 0
           : (_NRRD_ITER_NRRD(inB)
              ? 1
              : 2));
  if (2 == which) {
    biffAddf(NRRD, "%s: can't operate on two fixed values", me);
    return 1;
  }
  if (nrrdArithIterBinaryOpSelect(nout, op, inA, inB, which)) {
    biffAddf(NRRD, "%s: trouble", me);
    return 1;
  }
  return 0;
}

/* ---------------------------- ternary -------------- */

static double _nrrdTernaryOpAdd(double a, double b, double c) {
  return a + b + c;
}
static double _nrrdTernaryOpMultiply(double a, double b, double c)  {
  return a * b * c;
}
static double _nrrdTernaryOpMin(double a, double b, double c) {
  b = AIR_MIN(b, c);
  return AIR_MIN(a, b);
}
/*
** minsmooth(x, w, M) is like min(x,M), but starting at value M-w, values
** are lowered (via erf), so that the output is asymptotic to M
*/
static double _nrrdTernaryOpMinSmooth(double x, double width, double max) {
  double tran;
  tran = max - width;
  return (tran < max          /* using the function as intended */
          ? (x < tran
             ? x
             : airErf((x-tran)*0.886226925452758/(max - tran))*(max - tran) + tran)
          : AIR_MIN(x, max)); /* transition in wrong place; revert to simple max() */
}
static double _nrrdTernaryOpMax(double a, double b, double c) {
  b = AIR_MAX(b, c);
  return AIR_MAX(a, b);
}
/*
** maxsmooth(m, w, x) is like max(m,x), but starting at value m+w, values
** are raised (via erf), so that the output is asymptotic to m
*/
static double _nrrdTernaryOpMaxSmooth(double min, double width, double x) {
  double tran;
  tran = min + width;
  return (min < tran          /* using the function as intended */
          ? (tran < x
             ? x
             : airErf((x-tran)*0.886226925452758/(min - tran))*(min - tran) + tran)
          : AIR_MAX(x, min)); /* transition in wrong place; revert to simple max() */
}
static double _nrrdTernaryOpLTSmooth(double a, double w, double b) {
  return AIR_AFFINE(-1.0, airErf((b-a)/w), 1.0, 0.0, 1.0);
}
static double _nrrdTernaryOpGTSmooth(double a, double w, double b) {
  return AIR_AFFINE(-1.0, airErf((a-b)/w), 1.0, 0.0, 1.0);
}
static double _nrrdTernaryOpClamp(double a, double b, double c) {
  return AIR_CLAMP(a, b, c);
}
static double _nrrdTernaryOpIfElse(double a, double b, double c) {
  return (a ? b : c);
}
static double _nrrdTernaryOpLerp(double a, double b, double c) {
  /* we do something more than the simple lerp here because
     we want to facilitate usage as something which can get around
     non-existent values (b and c as NaN or Inf) without
     getting polluted by them. */

  if (0.0 == a) {
    return b;
  } else if (1.0 == a) {
    return c;
  } else {
    return AIR_LERP(a, b, c);
  }
}
static double _nrrdTernaryOpExists(double a, double b, double c) {
  return (AIR_EXISTS(a) ? b : c);
}
static double _nrrdTernaryOpInOpen(double a, double b, double c) {
  return (AIR_IN_OP(a, b, c));
}
static double _nrrdTernaryOpInClosed(double a, double b, double c) {
  return (AIR_IN_CL(a, b, c));
}
static double _nrrdTernaryOpGaussian(double x, double mu, double sig) {
  return airGaussian(x, mu, sig);
}
static double _nrrdTernaryOpRician(double x, double mu, double sig) {
  return airRician(x, mu, sig);
}
double (*_nrrdTernaryOp[NRRD_TERNARY_OP_MAX+1])(double, double, double) = {
  NULL,
  _nrrdTernaryOpAdd,
  _nrrdTernaryOpMultiply,
  _nrrdTernaryOpMin,
  _nrrdTernaryOpMinSmooth,
  _nrrdTernaryOpMax,
  _nrrdTernaryOpMaxSmooth,
  _nrrdTernaryOpLTSmooth,
  _nrrdTernaryOpGTSmooth,
  _nrrdTernaryOpClamp,
  _nrrdTernaryOpIfElse,
  _nrrdTernaryOpLerp,
  _nrrdTernaryOpExists,
  _nrrdTernaryOpInOpen,
  _nrrdTernaryOpInClosed,
  _nrrdTernaryOpGaussian,
  _nrrdTernaryOpRician
};

/*
******** nrrdArithTerneryOp
**
** HEY: UNTESTED UNTESTED UNTESTED UNTESTED UNTESTED UNTESTED UNTESTED
**
** this is a simplified version of nrrdArithIterTernaryOp, written after
** that, in a hurry, to operate directly on three nrrds, instead with
** the NrrdIter nonsense
*/
int
nrrdArithTernaryOp(Nrrd *nout, int op, const Nrrd *ninA,
                   const Nrrd *ninB, const Nrrd *ninC) {
  static const char me[]="nrrdArithTernaryOp";
  char *contA, *contB, *contC;
  size_t N, I, size[NRRD_DIM_MAX];
  double (*ins)(void *v, size_t I, double d),
    (*lupA)(const void *v, size_t I), (*lupB)(const void *v, size_t I),
    (*lupC)(const void *v, size_t I),
    (*top)(double a, double b, double c), valA, valB, valC;

  if (!( nout && !nrrdCheck(ninA) && !nrrdCheck(ninB) && !nrrdCheck(ninC) )) {
    biffAddf(NRRD, "%s: NULL pointer or invalid args", me);
    return 1;
  }
  if (!( nrrdSameSize(ninA, ninB, AIR_TRUE) &&
         nrrdSameSize(ninA, ninC, AIR_TRUE) )) {
    biffAddf(NRRD, "%s: size mismatch between arguments", me);
    return 1;
  }
  if (airEnumValCheck(nrrdTernaryOp, op)) {
    biffAddf(NRRD, "%s: ternary op %d invalid", me, op);
    return 1;
  }

  nrrdAxisInfoGet_nva(ninA, nrrdAxisInfoSize, size);
  if (!( nout == ninA || nout == ninB || nout == ninC)) {
    if (_nrrdMaybeAllocMaybeZero_nva(nout, ninA->type, ninA->dim, size,
                                     AIR_FALSE /* zero when no realloc */)) {
      biffAddf(NRRD, "%s: couldn't allocate output nrrd", me);
      return 1;
    }
    if (nrrdAxisInfoCopy(nout, ninA, NULL, NRRD_AXIS_INFO_NONE)) {
      biffAddf(NRRD, "%s:", me);
      return 1;
    }
    nrrdBasicInfoCopy(nout, ninA, (NRRD_BASIC_INFO_DATA_BIT
                                   | NRRD_BASIC_INFO_TYPE_BIT
                                   | NRRD_BASIC_INFO_DIMENSION_BIT
                                   | NRRD_BASIC_INFO_CONTENT_BIT
                                   | NRRD_BASIC_INFO_COMMENTS_BIT
                                   | (nrrdStateKeyValuePairsPropagate
                                      ? 0
                                      : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT)));
  }
  nrrdBasicInfoInit(nout,
                    NRRD_BASIC_INFO_ALL ^ (NRRD_BASIC_INFO_OLDMIN_BIT
                                           | NRRD_BASIC_INFO_OLDMAX_BIT));
  top = _nrrdTernaryOp[op];

  N = nrrdElementNumber(ninA);
  lupA = nrrdDLookup[ninA->type];
  lupB = nrrdDLookup[ninB->type];
  lupC = nrrdDLookup[ninC->type];
  ins = nrrdDInsert[nout->type];
  for (I=0; I<N; I++) {
    /* HEY: there is a loss of precision issue here with 64-bit ints */
    valA = lupA(ninA->data, I);
    valB = lupB(ninB->data, I);
    valC = lupC(ninC->data, I);
    ins(nout->data, I, top(valA, valB, valC));
  }

  contA = _nrrdContentGet(ninA);
  contB = _nrrdContentGet(ninB);
  contC = _nrrdContentGet(ninC);
  if (_nrrdContentSet_va(nout, airEnumStr(nrrdTernaryOp, op),
                         contA, "%s,%s", contB, contC)) {
    biffAddf(NRRD, "%s:", me);
    free(contA); free(contB); free(contC); return 1;
  }
  free(contA);
  free(contB);
  free(contC);

  return 0;
}

int
nrrdArithIterTernaryOpSelect(Nrrd *nout, int op,
                             NrrdIter *inA, NrrdIter *inB, NrrdIter *inC,
                             unsigned int which) {
  static const char me[]="nrrdArithIterTernaryOpSelect";
  char *contA, *contB, *contC;
  size_t N, I, size[NRRD_DIM_MAX];
  int type;
  double (*insert)(void *v, size_t I, double d),
    (*top)(double a, double b, double c), valA, valB, valC;
  const Nrrd *nin;

  if (!(nout && inA && inB && inC)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdTernaryOp, op)) {
    biffAddf(NRRD, "%s: ternary op %d invalid", me, op);
    return 1;
  }
  if (!( 0 == which || 1 == which || 2 == which )) {
    biffAddf(NRRD, "%s: which %u not valid, want 0, 1, or 2", me, which);
    return 1;
  }
  nin = (0 == which
         ? _NRRD_ITER_NRRD(inA)
         : (1 == which
            ? _NRRD_ITER_NRRD(inB)
            : _NRRD_ITER_NRRD(inC)));
  if (!nin) {
    biffAddf(NRRD, "%s: selected input %u is a fixed value", me, which);
    return 1;
  }
  type = nin->type;
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  if (_nrrdMaybeAllocMaybeZero_nva(nout, type, nin->dim, size,
                                   AIR_FALSE /* zero when no realloc */)) {
    biffAddf(NRRD, "%s: couldn't allocate output nrrd", me);
    return 1;
  }
  nrrdBasicInfoCopy(nout, nin, (NRRD_BASIC_INFO_DATA_BIT
                                | NRRD_BASIC_INFO_TYPE_BIT
                                | NRRD_BASIC_INFO_DIMENSION_BIT
                                | NRRD_BASIC_INFO_CONTENT_BIT
                                | NRRD_BASIC_INFO_COMMENTS_BIT
                                | (nrrdStateKeyValuePairsPropagate
                                   ? 0
                                   : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT)));
  nrrdBasicInfoInit(nout,
                    NRRD_BASIC_INFO_ALL ^ (NRRD_BASIC_INFO_OLDMIN_BIT
                                           | NRRD_BASIC_INFO_OLDMAX_BIT));
  top = _nrrdTernaryOp[op];

  /*
  fprintf(stderr, "%!s: inA->left = %d, inB->left = %d\n", me,
          (int)(inA->left), (int)(inB->left));
  */
  N = nrrdElementNumber(nin);
  insert = nrrdDInsert[type];
  for (I=0; I<N; I++) {
    /* HEY: there is a loss of precision issue here with 64-bit ints */
    valA = nrrdIterValue(inA);
    valB = nrrdIterValue(inB);
    valC = nrrdIterValue(inC);
    /*
    if (!(I % 1000)) {
      fprintf(stderr, "!%s: %d: top(%g,%g,%g) = %g\n", me, (int)I,
              valA, valB, valC,
              top(valA, valB, valC));
    }
    */
    insert(nout->data, I, top(valA, valB, valC));
  }
  contA = nrrdIterContent(inA);
  contB = nrrdIterContent(inB);
  contC = nrrdIterContent(inC);
  if (_nrrdContentSet_va(nout, airEnumStr(nrrdTernaryOp, op),
                         contA, "%s,%s", contB, contC)) {
    biffAddf(NRRD, "%s:", me);
    free(contA); free(contB); free(contC); return 1;
  }
  if (nout != nin) {
    nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_NONE);
  }
  free(contA);
  free(contB);
  free(contC);
  return 0;
}

int
nrrdArithIterTernaryOp(Nrrd *nout, int op,
                       NrrdIter *inA, NrrdIter *inB, NrrdIter *inC) {
  static const char me[]="nrrdArithIterTernaryOp";
  unsigned int which;

  if (!(nout && inA && inB && inC)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  which = (_NRRD_ITER_NRRD(inA)
           ? 0
           : (_NRRD_ITER_NRRD(inB)
              ? 1
              : (_NRRD_ITER_NRRD(inC)
                 ? 2
                 : 3 )));
  if (3 == which) {
    biffAddf(NRRD, "%s: can't operate on 3 fixed values", me);
    return 1;
  }
  if (nrrdArithIterTernaryOpSelect(nout, op, inA, inB, inC, which)) {
    biffAddf(NRRD, "%s: trouble", me);
    return 1;
  }
  return 0;
}

int
nrrdArithAffine(Nrrd *nout, double minIn,
                const Nrrd *nin, double maxIn,
                double minOut, double maxOut, int clamp) {
  static const char me[]="nrrdArithAffine";
  size_t I, N;
  double (*ins)(void *v, size_t I, double d),
    (*lup)(const void *v, size_t I);

  if ( !nout || nrrdCheck(nin) ) {
    biffAddf(NRRD, "%s: got NULL pointer or invalid input", me);
    return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      biffAddf(NRRD, "%s: couldn't initialize output", me);
      return 1;
    }
  }
  N = nrrdElementNumber(nin);
  ins = nrrdDInsert[nout->type];
  lup = nrrdDLookup[nin->type];
  for (I=0; I<N; I++) {
    double val;
    val = lup(nin->data, I);
    val = AIR_AFFINE(minIn, val, maxIn, minOut, maxOut);
    if (clamp) {
      val = AIR_CLAMP(minOut, val, maxOut);
    }
    ins(nout->data, I, val);
  }
  /* HEY: it would be much better if the ordering here was the same as in
     AIR_AFFINE, but that's not easy with the way the content functions are
     now set up */
  if (nrrdContentSet_va(nout, "affine", nin,
                        "%g,%g,%g,%g", minIn, maxIn,
                        minOut, maxOut)) {
    biffAddf(NRRD, "%s:", me);
  }
  return 0;
}

int
nrrdArithIterAffine(Nrrd *nout, NrrdIter *minIn,
                    NrrdIter *in, NrrdIter *maxIn,
                    NrrdIter *minOut, NrrdIter *maxOut, int clamp) {
  static const char me[]="nrrdArithInterAffine";
  double (*ins)(void *v, size_t I, double d),
    mini, vin, maxi, mino, maxo, vout;
  const Nrrd *nin;
  char *contA, *contB, *contC, *contD, *contE;
  size_t I, N;

  if (!(nout && minIn && in && maxIn && minOut && maxOut)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  nin = (_NRRD_ITER_NRRD(in)
         ? _NRRD_ITER_NRRD(in)
         : (_NRRD_ITER_NRRD(minIn)
            ? _NRRD_ITER_NRRD(minIn)
            : (_NRRD_ITER_NRRD(maxIn)
               ? _NRRD_ITER_NRRD(maxIn)
               : (_NRRD_ITER_NRRD(minOut)
                  ? _NRRD_ITER_NRRD(minOut)
                  : _NRRD_ITER_NRRD(maxOut)))));
  if (!nin) {
    biffAddf(NRRD, "%s: can't operate solely on fixed values", me);
    return 1;
  }
  if (nrrdCopy(nout, nin)) {
    biffAddf(NRRD, "%s: couldn't initialize output", me);
    return 1;
  }
  N = nrrdElementNumber(nin);
  ins = nrrdDInsert[nout->type];
  for (I=0; I<N; I++) {
    mini = nrrdIterValue(minIn);
    vin = nrrdIterValue(in);
    maxi = nrrdIterValue(maxIn);
    mino = nrrdIterValue(minOut);
    maxo = nrrdIterValue(maxOut);
    vout = AIR_AFFINE(mini, vin, maxi, mino, maxo);
    if (clamp) {
      vout = AIR_CLAMP(mino, vout, maxo);
    }
    ins(nout->data, I, vout);
  }
  contA = nrrdIterContent(in);
  contB = nrrdIterContent(minIn);
  contC = nrrdIterContent(maxIn);
  contD = nrrdIterContent(maxOut);
  contE = nrrdIterContent(maxOut);
  /* HEY: same annoyance about order of arguments as in function above */
  if (_nrrdContentSet_va(nout, "affine", contA, "%s,%s,%s,%s",
                         contB, contC, contD, contE)) {
    biffAddf(NRRD, "%s:", me);
    free(contA); free(contB); free(contC); free(contD); free(contE); return 1;
  }
  free(contA); free(contB); free(contC); free(contD); free(contE);

  return 0;
}

unsigned int
nrrdCRC32(const Nrrd *nin, int endian) {
  size_t nn;

  /* NULL nrrd or data */
  if (!nin
      || !(nin->data)
      || !(nn = nrrdElementSize(nin)*nrrdElementNumber(nin))
      || airEnumValCheck(airEndian, endian)) {
    return 0;
  }

  return airCRC32(AIR_CAST(const unsigned char *, nin->data),
                  nn, nrrdElementSize(nin),
                  endian == airMyEndian() ? AIR_FALSE : AIR_TRUE);
}

