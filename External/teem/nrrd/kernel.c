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

/*
** summary of information about how the kernel parameter vector is set:

                                 numParm  parm[0]   parm[1]   parm[2]
                 nrrdKernelHann    2      scale    cut-off
             nrrdKernelBlackman    2      scale    cut-off
           nrrdKernelCatmullRom    0
             nrrdKernelBSpline3    0
nrrdKernelBSpline3ApproxInverse    0
             nrrdKernelBSpline5    0
nrrdKernelBSpline5ApproxInverse    0
             nrrdKernelBSpline7    0
nrrdKernelBSpline7ApproxInverse    0
                 nrrdKernelZero    1      scale
                  nrrdKernelBox    1      scale
nrrdKernelCatmullRomSupportDebug   1      support
      nrrdKernelBoxSupportDebug    1      support
     nrrdKernelCos4SupportDebug    1      support
                nrrdKernelCheap    1      scale
nrrdKernelHermiteScaleSpaceFlag    0
                 nrrdKernelTent    1      scale
             nrrdKernelForwDiff    1      scale
             nrrdKernelCentDiff    1      scale
              nrrdKernelBCCubic    3      scale       B        C
             nrrdKernelAQuartic    2      scale       A
            nrrdKernelC3Quintic    0
              nrrdKernelC4Hexic    0
 nrrdKernelC4HexicApproxInverse    0
             nrrdKernelC5Septic    0
             nrrdKernelGaussian    2      sigma    cut-off
     nrrdKernelDiscreteGaussian    2      sigma    cut-off
            nrrdKernelTMF[][][]    1       a

** Note that when parm[0] is named "scale", that parameter is optional,
** and the default is 1.0, when given in string form
** E.g. "tent" is understood as "tent:1",
** but "gauss:4" isn't complete and won't parse; while "gauss:1,4" is good
*/

/* these functions replace what had been a lot of
   identical functions for similar kernels */

static double
returnZero(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

static double
returnOne(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

static double
returnTwo(const double *parm) {
  AIR_UNUSED(parm);
  return 2.0;
}

static double
returnThree(const double *parm) {
  AIR_UNUSED(parm);
  return 3.0;
}

static double
returnFour(const double *parm) {
  AIR_UNUSED(parm);
  return 4.0;
}

/* ------------------------------------------------------------ */

/* learned: if you copy/paste the macros for these kernels into
** other code, you *have* to make sure that the arguments for the
** kernels that are supposed to be reals, are not passed as an
** integral type (had this problem trying to re-use _BCCUBIC
** with constant B="1" and C="0" and had trouble figuring out
** why the kernel was give garbage results
*/

/* the "zero" kernel is here more as a template than for anything else
   (as well as when you need the derivative of nrrdKernelForwDiff or
   any other piece-wise constant kernels)
   In particular the support method is pretty silly. */

#define _ZERO(x) 0

static double
_nrrdZeroSup(const double *parm) {
  double S;

  S = parm[0];
  return S;
}

static double
_nrrdZero1_d(double x, const double *parm) {
  double S;

  S = parm[0];
  x = AIR_ABS(x)/S;
  return _ZERO(x)/S;
}

static float
_nrrdZero1_f(float x, const double *parm) {
  float S;

  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return _ZERO(x)/S;
}

static void
_nrrdZeroN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;

  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = _ZERO(t)/S;
  }
}

static void
_nrrdZeroN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;

  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = _ZERO(t)/S;
  }
}

static NrrdKernel
_nrrdKernelZero = {
  "zero",
  1, _nrrdZeroSup, returnZero,
  _nrrdZero1_f, _nrrdZeroN_f, _nrrdZero1_d, _nrrdZeroN_d
};
NrrdKernel *const
nrrdKernelZero = &_nrrdKernelZero;

/* ------------------------------------------------------------ */

#define _BOX(x) (x > 0.5 ? 0 : (x < 0.5 ? 1 : 0.5))

static double
_nrrdBoxSup(const double *parm) {
  double S;

  S = parm[0];
  /* adding the 0.5 is to ensure that weights computed within the
     support really do catch all the non-zero values */
  return S/2 + 0.5;
}

static double
_nrrdBox1_d(double x, const double *parm) {
  double S;

  S = parm[0];
  x = AIR_ABS(x)/S;
  return _BOX(x)/S;
}

static float
_nrrdBox1_f(float x, const double *parm) {
  float S;

  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return AIR_CAST(float, _BOX(x)/S);
}

static void
_nrrdBoxN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;

  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = _BOX(t)/S;
  }
}

static void
_nrrdBoxN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;

  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = AIR_CAST(float, _BOX(t)/S);
  }
}

static NrrdKernel
_nrrdKernelBox = {
  "box",
  1, _nrrdBoxSup, returnOne,
  _nrrdBox1_f,  _nrrdBoxN_f,  _nrrdBox1_d,  _nrrdBoxN_d
};
NrrdKernel *const
nrrdKernelBox = &_nrrdKernelBox;

/* ------------------------------------------------------------ */

static double
_nrrdBoxSDSup(const double *parm) {

  return parm[0];
}

static double
_nrrdBoxSD1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _BOX(x);
}

static float
_nrrdBoxSD1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, _BOX(x));
}

static void
_nrrdBoxSDN_d(double *f, const double *x, size_t len, const double *parm) {
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    double t;
    t = AIR_ABS(x[i]);
    f[i] = _BOX(t);
  }
}

static void
_nrrdBoxSDN_f(float *f, const float *x, size_t len, const double *parm) {
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    float t;
    t = AIR_ABS(x[i]);
    f[i] = AIR_CAST(float, _BOX(t));
  }
}

static NrrdKernel
_nrrdKernelBoxSupportDebug = {
  "boxsup",
  1, _nrrdBoxSDSup, returnOne,
  _nrrdBoxSD1_f,  _nrrdBoxSDN_f,  _nrrdBoxSD1_d,  _nrrdBoxSDN_d
};
NrrdKernel *const
nrrdKernelBoxSupportDebug = &_nrrdKernelBoxSupportDebug;

/* ------------------------------------------------------------ */

#define COS4(x) (x > 0.5 \
                 ? 0.0 \
                 : cos(AIR_PI*x)*cos(AIR_PI*x)*cos(AIR_PI*x)*cos(AIR_PI*x))

static double
_nrrdCos4SDInt(const double *parm) {
  AIR_UNUSED(parm);
  return 3.0/8.0;
}

static double
_nrrdCos4SDSup(const double *parm) {

  return parm[0];
}

static double
_nrrdCos4SD1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return COS4(x);
}

static float
_nrrdCos4SD1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, COS4(x));
}

static void
_nrrdCos4SDN_d(double *f, const double *x, size_t len, const double *parm) {
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    double t;
    t = AIR_ABS(x[i]);
    f[i] = COS4(t);
  }
}

static void
_nrrdCos4SDN_f(float *f, const float *x, size_t len, const double *parm) {
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    float t;
    t = AIR_ABS(x[i]);
    f[i] = AIR_CAST(float, COS4(t));
  }
}

static NrrdKernel
_nrrdKernelCos4SupportDebug = {
  "cos4sup",
  1, _nrrdCos4SDSup, _nrrdCos4SDInt,
  _nrrdCos4SD1_f,  _nrrdCos4SDN_f,  _nrrdCos4SD1_d,  _nrrdCos4SDN_d
};
NrrdKernel *const
nrrdKernelCos4SupportDebug = &_nrrdKernelCos4SupportDebug;

/* ------------------------------------------------------------ */

#define DCOS4(x) (x > 0.5                                               \
                  ? 0.0                                                 \
                  : -4*AIR_PI*(cos(AIR_PI*x)*cos(AIR_PI*x)*cos(AIR_PI*x) \
                               *sin(AIR_PI*x)))

static double
_nrrdDCos4SDSup(const double *parm) {
  return parm[0];
}

static double
_nrrdDCos4SD1_d(double x, const double *parm) {
  int sgn;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; } else { sgn = 1; }
  return sgn*DCOS4(x);
}

static float
_nrrdDCos4SD1_f(float x, const double *parm) {
  int sgn;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; } else { sgn = 1; }
  return AIR_CAST(float, sgn*DCOS4(x));
}

static void
_nrrdDCos4SDN_d(double *f, const double *x, size_t len, const double *parm) {
  int sgn;
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*DCOS4(t);
  }
}

static void
_nrrdDCos4SDN_f(float *f, const float *x, size_t len, const double *parm) {
  int sgn;
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*DCOS4(t));
  }
}

static NrrdKernel
_nrrdKernelCos4SupportDebugD = {
  "cos4supD",
  1, _nrrdDCos4SDSup, returnZero,
  _nrrdDCos4SD1_f,   _nrrdDCos4SDN_f,   _nrrdDCos4SD1_d,   _nrrdDCos4SDN_d
};
NrrdKernel *const
nrrdKernelCos4SupportDebugD = &_nrrdKernelCos4SupportDebugD;

/* ------------------------------------------------------------ */

#define DDCOS4(x) (x > 0.5                                              \
                   ? 0.0                                                \
                   : -2*AIR_PI*AIR_PI*(cos(AIR_PI*2*x) + cos(AIR_PI*4*x)))

static double
_nrrdDDCos4SDSup(const double *parm) {
  return parm[0];
}

static double
_nrrdDDCos4SD1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return DDCOS4(x);
}

static float
_nrrdDDCos4SD1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, DDCOS4(x));
}

static void
_nrrdDDCos4SDN_d(double *f, const double *x, size_t len, const double *parm) {
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    double t;
    t = AIR_ABS(x[i]);
    f[i] = DDCOS4(t);
  }
}

static void
_nrrdDDCos4SDN_f(float *f, const float *x, size_t len, const double *parm) {
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    float t;
    t = AIR_ABS(x[i]);
    f[i] = AIR_CAST(float, DDCOS4(t));
  }
}

static NrrdKernel
_nrrdKernelCos4SupportDebugDD = {
  "cos4supDD",
  1, _nrrdDDCos4SDSup, returnZero,
  _nrrdDDCos4SD1_f,  _nrrdDDCos4SDN_f,  _nrrdDDCos4SD1_d,  _nrrdDDCos4SDN_d
};
NrrdKernel *const
nrrdKernelCos4SupportDebugDD = &_nrrdKernelCos4SupportDebugDD;

/* ------------------------------------------------------------ */

#define DDDCOS4(x) (x > 0.5                                             \
                    ? 0.0                                               \
                    : 4*AIR_PI*AIR_PI*AIR_PI*(sin(2*AIR_PI*x) + 2*sin(4*AIR_PI*x)))

static double
_nrrdDDDCos4SDSup(const double *parm) {
  return parm[0];
}

static double
_nrrdDDDCos4SD1_d(double x, const double *parm) {
  int sgn;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; } else { sgn = 1; }
  return sgn*DDDCOS4(x);
}

static float
_nrrdDDDCos4SD1_f(float x, const double *parm) {
  int sgn;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; } else { sgn = 1; }
  return AIR_CAST(float, sgn*DDDCOS4(x));
}

static void
_nrrdDDDCos4SDN_d(double *f, const double *x, size_t len, const double *parm) {
  int sgn;
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*DDDCOS4(t);
  }
}

static void
_nrrdDDDCos4SDN_f(float *f, const float *x, size_t len, const double *parm) {
  int sgn;
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*DDDCOS4(t));
  }
}

static NrrdKernel
_nrrdKernelCos4SupportDebugDDD = {
  "cos4supDDD",
  1, _nrrdDDDCos4SDSup, returnZero,
  _nrrdDDDCos4SD1_f, _nrrdDDDCos4SDN_f, _nrrdDDDCos4SD1_d, _nrrdDDDCos4SDN_d
};
NrrdKernel *const
nrrdKernelCos4SupportDebugDDD = &_nrrdKernelCos4SupportDebugDDD;

/* ------------------------------------------------------------ */

/* The point here is that post-kernel-evaluation, we need to see
   which sample is closest to the origin, and this is one way of
   enabling that
   SO: this kernel will not usefully report its integral or support! */
#define _CHEAP(x) AIR_ABS(x)

static double
_nrrdCheapSup(const double *parm) {
  double S;

  S = parm[0];
  /* adding the 0.5 is to insure that weights computed within the
     support really do catch all the non-zero values */
  return S/2 + 0.5;
}

static double
_nrrdCheap1_d(double x, const double *parm) {

  return _CHEAP(x)/parm[0];
}

static float
_nrrdCheap1_f(float x, const double *parm) {

  return AIR_CAST(float, _CHEAP(x)/parm[0]);
}

static void
_nrrdCheapN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;

  for (i=0; i<len; i++) {
    t = x[i];
    f[i] = _CHEAP(t)/parm[0];
  }
}

static void
_nrrdCheapN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;

  for (i=0; i<len; i++) {
    t = x[i];
    f[i] = AIR_CAST(float, _CHEAP(t)/parm[0]);
  }
}

static NrrdKernel
_nrrdKernelCheap = {
  "cheap",
  1, _nrrdCheapSup, returnOne,
  _nrrdCheap1_f,  _nrrdCheapN_f,  _nrrdCheap1_d,  _nrrdCheapN_d
};
NrrdKernel *const
nrrdKernelCheap = &_nrrdKernelCheap;

/* ------------------------------------------------------------ */

#define _TENT(x) (x >= 1 ? 0 : 1 - x)

static double
_nrrdTentSup(const double *parm) {
  double S;

  S = parm[0];
  return S;
}

static double
_nrrdTent1_d(double x, const double *parm) {
  double S;

  S = parm[0];
  x = AIR_ABS(x)/S;
  return S ? _TENT(x)/S : x == 0;
}

static float
_nrrdTent1_f(float x, const double *parm) {
  float S;

  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return S ? _TENT(x)/S : x == 0;
}

static void
_nrrdTentN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;

  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = S ? _TENT(t)/S : t == 0;
  }
}

static void
_nrrdTentN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;

  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = S ? _TENT(t)/S : t == 0;
  }
}

static NrrdKernel
_nrrdKernelTent = {
  "tent",
  1, _nrrdTentSup, returnOne,
  _nrrdTent1_f, _nrrdTentN_f, _nrrdTent1_d, _nrrdTentN_d
};
NrrdKernel *const
nrrdKernelTent = &_nrrdKernelTent;

/* ------------------------------------------------------------ */

/*
** NOTE: THERE IS NOT REALLY A HERMITE KERNEL (at least not yet,
** because it takes both values and derivatives as arguments, which
** the NrrdKernel currently can't handle).  This isn't really a
** kernel, its mostly a flag (hence the name), but it also has the
** role of generating weights according to linear interpolation, which
** is useful for the eventual spline evaluation.
**
** This hack is in sinister collusion with gage, to enable Hermite
** interpolation for its stack reconstruction.
*/

static double
_nrrdHermite1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _TENT(x);
}

static float
_nrrdHermite1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _TENT(x);
}

static void
_nrrdHermiteN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t);
    f[i] = _TENT(t);
  }
}

static void
_nrrdHermiteN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t);
    f[i] = _TENT(t);
  }
}

/* HEY: should just re-use fields from nrrdKernelTent, instead
   of creating new functions */
static NrrdKernel
_nrrdKernelHermiteScaleSpaceFlag = {
  "hermiteSS",
  0, returnOne, returnOne,
  _nrrdHermite1_f, _nrrdHermiteN_f, _nrrdHermite1_d, _nrrdHermiteN_d
};
NrrdKernel *const
nrrdKernelHermiteScaleSpaceFlag = &_nrrdKernelHermiteScaleSpaceFlag;

/* ------------------------------------------------------------ */

#define _FORDIF(x) (x < -1 ?  0 :        \
                   (x <  0 ?  1 :        \
                   (x <  1 ? -1 : 0 )))

static double
_nrrdFDSup(const double *parm) {
  double S;

  S = parm[0];
  return S+0.0001;  /* sigh */
}

static double
_nrrdFD1_d(double x, const double *parm) {
  double S;

  S = parm[0];
  x /= S;
  return _FORDIF(x)/(S*S);
}

static float
_nrrdFD1_f(float x, const double *parm) {
  float S;

  S = AIR_CAST(float, parm[0]);
  x /= S;
  return _FORDIF(x)/(S*S);
}

static void
_nrrdFDN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;

  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    f[i] = _FORDIF(t)/(S*S);
  }
}

static void
_nrrdFDN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;

  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    f[i] = _FORDIF(t)/(S*S);
  }
}

static NrrdKernel
_nrrdKernelFD = {
  "fordif",
  1, _nrrdFDSup, returnZero,
  _nrrdFD1_f,   _nrrdFDN_f,   _nrrdFD1_d,   _nrrdFDN_d
};
NrrdKernel *const
nrrdKernelForwDiff = &_nrrdKernelFD;

/* ------------------------------------------------------------ */

#define _CENDIF(x) (x <= -2 ?  0         :        \
                   (x <= -1 ?  0.5*x + 1 :        \
                   (x <=  1 ? -0.5*x     :        \
                   (x <=  2 ?  0.5*x - 1 : 0 ))))

static double
_nrrdCDSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

static double
_nrrdCD1_d(double x, const double *parm) {
  double S;

  S = parm[0];
  x /= S;
  return _CENDIF(x)/(S*S);
}

static float
_nrrdCD1_f(float x, const double *parm) {
  float S;

  S = AIR_CAST(float, parm[0]);
  x /= S;
  return AIR_CAST(float, _CENDIF(x)/(S*S));
}

static void
_nrrdCDN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;

  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    f[i] = _CENDIF(t)/(S*S);
  }
}

static void
_nrrdCDN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;

  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    f[i] = AIR_CAST(float, _CENDIF(t)/(S*S));
  }
}

static NrrdKernel
_nrrdKernelCD = {
  "cendif",
  1, _nrrdCDSup, returnZero,
  _nrrdCD1_f,   _nrrdCDN_f,   _nrrdCD1_d,   _nrrdCDN_d
};
NrrdKernel *const
nrrdKernelCentDiff = &_nrrdKernelCD;

/* ------------------------------------------------------------ */

#define _BCCUBIC(x, B, C)                                     \
  (x >= 2.0 ? 0 :                                             \
  (x >= 1.0                                                   \
   ? (((-B/6 - C)*x + B + 5*C)*x -2*B - 8*C)*x + 4*B/3 + 4*C  \
   : ((2 - 3*B/2 - C)*x - 3 + 2*B + C)*x*x + 1 - B/3))

static double
_nrrdBCSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

static double
_nrrdBC1_d(double x, const double *parm) {
  double S;
  double B, C;

  S = parm[0]; B = parm[1]; C = parm[2];
  x = AIR_ABS(x)/S;
  return _BCCUBIC(x, B, C)/S;
}

static float
_nrrdBC1_f(float x, const double *parm) {
  float B, C, S;

  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]);
  x = AIR_ABS(x)/S;
  return _BCCUBIC(x, B, C)/S;
}

static void
_nrrdBCN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, B, C;
  size_t i;

  S = parm[0]; B = parm[1]; C = parm[2];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _BCCUBIC(t, B, C)/S;
  }
}

static void
_nrrdBCN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, B, C;
  size_t i;

  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _BCCUBIC(t, B, C)/S;
  }
}

static NrrdKernel
_nrrdKernelBC = {
  "BCcubic",
  3, _nrrdBCSup, returnOne,
  _nrrdBC1_f,   _nrrdBCN_f,   _nrrdBC1_d,   _nrrdBCN_d
};
NrrdKernel *const
nrrdKernelBCCubic = &_nrrdKernelBC;

/* ------------------------------------------------------------ */

#define _DBCCUBIC(x, B, C)                        \
   (x >= 2.0 ? 0.0 :                              \
   (x >= 1.0                                      \
    ? ((-B/2 - 3*C)*x + 2*B + 10*C)*x -2*B - 8*C  \
    : ((6 - 9*B/2 - 3*C)*x - 6 + 4*B + 2*C)*x))

static double
_nrrdDBCSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

static double
_nrrdDBC1_d(double x, const double *parm) {
  double S;
  double B, C;
  int sgn = 1;

  S = parm[0]; B = parm[1]; C = parm[2];
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return sgn*_DBCCUBIC(x, B, C)/(S*S);
}

static float
_nrrdDBC1_f(float x, const double *parm) {
  float B, C, S;
  int sgn = 1;

  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]);
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return AIR_CAST(float, sgn*_DBCCUBIC(x, B, C)/(S*S));
}

static void
_nrrdDBCN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, B, C;
  size_t i;
  int sgn;

  S = parm[0]; B = parm[1]; C = parm[2];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DBCCUBIC(t, B, C)/(S*S);
  }
}

static void
_nrrdDBCN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, B, C;
  int sgn;
  size_t i;

  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DBCCUBIC(t, B, C)/(S*S));
  }
}

static NrrdKernel
_nrrdKernelDBC = {
  "BCcubicD",
  3, _nrrdDBCSup, returnZero,
  _nrrdDBC1_f,  _nrrdDBCN_f,  _nrrdDBC1_d,  _nrrdDBCN_d
};
NrrdKernel *const
nrrdKernelBCCubicD = &_nrrdKernelDBC;

/* ------------------------------------------------------------ */

#define _DDBCCUBIC(x, B, C)                    \
   (x >= 2.0 ? 0 :                             \
   (x >= 1.0                                   \
    ? (-B - 6*C)*x + 2*B + 10*C                \
    : (12 - 9*B - 6*C)*x - 6 + 4*B + 2*C  ))

static double
_nrrdDDBCSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

static double
_nrrdDDBC1_d(double x, const double *parm) {
  double S;
  double B, C;

  S = parm[0]; B = parm[1]; C = parm[2];
  x = AIR_ABS(x)/S;
  return _DDBCCUBIC(x, B, C)/(S*S*S);
}

static float
_nrrdDDBC1_f(float x, const double *parm) {
  float B, C, S;

  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]);
  x = AIR_ABS(x)/S;
  return _DDBCCUBIC(x, B, C)/(S*S*S);
}

static void
_nrrdDDBCN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, B, C;
  size_t i;

  S = parm[0]; B = parm[1]; C = parm[2];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDBCCUBIC(t, B, C)/(S*S*S);
  }
}

static void
_nrrdDDBCN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, B, C;
  size_t i;

  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDBCCUBIC(t, B, C)/(S*S*S);
  }
}

static NrrdKernel
_nrrdKernelDDBC = {
  "BCcubicDD",
  3, _nrrdDDBCSup, returnZero,
  _nrrdDDBC1_f, _nrrdDDBCN_f, _nrrdDDBC1_d, _nrrdDDBCN_d
};
NrrdKernel *const
nrrdKernelBCCubicDD = &_nrrdKernelDDBC;

/* ------------------------------------------------------------ */

/* if you've got the definition already, why not use it */
#define _CTMR(x) _BCCUBIC(x, 0.0, 0.5)

static double
_nrrdCTMR1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _CTMR(x);
}

static float
_nrrdCTMR1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, _CTMR(x));
}

static void
_nrrdCTMRN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _CTMR(t);
  }
}

static void
_nrrdCTMRN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _CTMR(t));
  }
}

static NrrdKernel
_nrrdKernelCatmullRom = {
  "catmull-rom",
  0, returnTwo, returnOne,
  _nrrdCTMR1_f,   _nrrdCTMRN_f,   _nrrdCTMR1_d,   _nrrdCTMRN_d
};
NrrdKernel *const
nrrdKernelCatmullRom = &_nrrdKernelCatmullRom;


static double
_nrrdCtmrSDSup(const double *parm) {

  return AIR_MAX(2.0, parm[0]);
}

static NrrdKernel
_nrrdKernelCatmullRomSupportDebug = {
  "ctmrsup",
  1, _nrrdCtmrSDSup, returnOne,
  _nrrdCTMR1_f,   _nrrdCTMRN_f,   _nrrdCTMR1_d,   _nrrdCTMRN_d
};
NrrdKernel *const
nrrdKernelCatmullRomSupportDebug = &_nrrdKernelCatmullRomSupportDebug;

/* ------------------------------------------------------------ */

/* if you've got the definition already, why not use it */
#define _DCTMR(x) _DBCCUBIC(x, 0.0, 0.5)

static double
_nrrdDCTMR1_d(double x, const double *parm) {
  int sgn;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; } else { sgn = 1; }
  return sgn*_DCTMR(x);
}

static float
_nrrdDCTMR1_f(float x, const double *parm) {
  int sgn;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; } else { sgn = 1; }
  return AIR_CAST(float, sgn*_DCTMR(x));
}

static void
_nrrdDCTMRN_d(double *f, const double *x, size_t len, const double *parm) {
  int sgn;
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DCTMR(t);
  }
}

static void
_nrrdDCTMRN_f(float *f, const float *x, size_t len, const double *parm) {
  int sgn;
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DCTMR(t));
  }
}

static NrrdKernel
_nrrdKernelCatmullRomD = {
  "catmull-romD",
  0, returnTwo, returnZero,
  _nrrdDCTMR1_f,   _nrrdDCTMRN_f,   _nrrdDCTMR1_d,   _nrrdDCTMRN_d
};
NrrdKernel *const
nrrdKernelCatmullRomD = &_nrrdKernelCatmullRomD;

static NrrdKernel
_nrrdKernelCatmullRomSupportDebugD = {
  "ctmrsupD",
  1, _nrrdCtmrSDSup, returnZero,
  _nrrdDCTMR1_f,   _nrrdDCTMRN_f,   _nrrdDCTMR1_d,   _nrrdDCTMRN_d
};
NrrdKernel *const
nrrdKernelCatmullRomSupportDebugD = &_nrrdKernelCatmullRomSupportDebugD;

/* ------------------------------------------------------------ */

/* if you've got the definition already, why not use it */
#define _DDCTMR(x) _DDBCCUBIC(x, 0.0, 0.5)

static double
_nrrdDDCTMR1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _DDCTMR(x);
}

static float
_nrrdDDCTMR1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, _DDCTMR(x));
}

static void
_nrrdDDCTMRN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _DDCTMR(t);
  }
}

static void
_nrrdDDCTMRN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _DDCTMR(t));
  }
}

static NrrdKernel
_nrrdKernelCatmullRomDD = {
  "catmull-romDD",
  0, returnTwo, returnZero,
  _nrrdDDCTMR1_f,   _nrrdDDCTMRN_f,   _nrrdDDCTMR1_d,   _nrrdDDCTMRN_d
};
NrrdKernel *const
nrrdKernelCatmullRomDD = &_nrrdKernelCatmullRomDD;

static NrrdKernel
_nrrdKernelCatmullRomSupportDebugDD = {
  "ctmrsupDD",
  1, _nrrdCtmrSDSup, returnZero,
  _nrrdDDCTMR1_f,   _nrrdDDCTMRN_f,   _nrrdDDCTMR1_d,   _nrrdDDCTMRN_d
};
NrrdKernel *const
nrrdKernelCatmullRomSupportDebugDD = &_nrrdKernelCatmullRomSupportDebugDD;

/* ------------------------------------------------------------ */

#define _AQUARTIC(x, A) \
   (x >= 3.0 ? 0 :      \
   (x >= 2.0            \
    ? A*(-54 + x*(81 + x*(-45 + x*(11 - x)))) \
    : (x >= 1.0                               \
       ? 4 - 6*A + x*(-10 + 25*A + x*(9 - 33*A                         \
                                 + x*(-3.5 + 17*A + x*(0.5 - 3*A))))   \
       : 1 + x*x*(-3 + 6*A + x*((2.5 - 10*A) + x*(-0.5 + 4*A))))))

static double
_nrrdA4Sup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

static double
_nrrdA41_d(double x, const double *parm) {
  double S;
  double A;

  S = parm[0]; A = parm[1];
  x = AIR_ABS(x)/S;
  return _AQUARTIC(x, A)/S;
}

static float
_nrrdA41_f(float x, const double *parm) {
  float A, S;

  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  x = AIR_ABS(x)/S;
  return AIR_CAST(float, _AQUARTIC(x, A)/S);
}

static void
_nrrdA4N_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, A;
  size_t i;

  S = parm[0]; A = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _AQUARTIC(t, A)/S;
  }
}

static void
_nrrdA4N_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, A;
  size_t i;

  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = AIR_CAST(float, _AQUARTIC(t, A)/S);
  }
}

static NrrdKernel
_nrrdKernelA4 = {
  "Aquartic",
  2, _nrrdA4Sup, returnOne,
  _nrrdA41_f,   _nrrdA4N_f,   _nrrdA41_d,   _nrrdA4N_d
};
NrrdKernel *const
nrrdKernelAQuartic = &_nrrdKernelA4;

/* ------------------------------------------------------------ */

#define _DAQUARTIC(x, A) \
   (x >= 3.0 ? 0 :       \
   (x >= 2.0             \
    ? A*(81 + x*(-90 + x*(33 - 4*x))) \
    : (x >= 1.0                       \
       ? -10 + 25*A + x*(18 - 66*A + x*(-10.5 + 51*A + x*(2 - 12*A))) \
       : x*(-6 + 12*A + x*(7.5 - 30*A + x*(-2 + 16*A))))))

static double
_nrrdDA4Sup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

static double
_nrrdDA41_d(double x, const double *parm) {
  double S;
  double A;
  int sgn = 1;

  S = parm[0]; A = parm[1];
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return sgn*_DAQUARTIC(x, A)/(S*S);
}

static float
_nrrdDA41_f(float x, const double *parm) {
  float A, S;
  int sgn = 1;

  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return AIR_CAST(float, sgn*_DAQUARTIC(x, A)/(S*S));
}

static void
_nrrdDA4N_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, A;
  size_t i;
  int sgn;

  S = parm[0]; A = parm[1];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DAQUARTIC(t, A)/(S*S);
  }
}

static void
_nrrdDA4N_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, A;
  size_t i;
  int sgn;

  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DAQUARTIC(t, A)/(S*S));
  }
}

static NrrdKernel
_nrrdKernelDA4 = {
  "AquarticD",
  2, _nrrdDA4Sup, returnZero,
  _nrrdDA41_f,  _nrrdDA4N_f,  _nrrdDA41_d,  _nrrdDA4N_d
};
NrrdKernel *const
nrrdKernelAQuarticD = &_nrrdKernelDA4;

/* ------------------------------------------------------------ */

#define _DDAQUARTIC(x, A) \
   (x >= 3.0 ? 0 :        \
   (x >= 2.0              \
    ? A*(-90 + x*(66 - x*12)) \
    : (x >= 1.0               \
       ? 18 - 66*A + x*(-21 + 102*A + x*(6 - 36*A))   \
       : -6 + 12*A + x*(15 - 60*A + x*(-6 + 48*A)))))

static double
_nrrdDDA4Sup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

static double
_nrrdDDA41_d(double x, const double *parm) {
  double S;
  double A;

  S = parm[0]; A = parm[1];
  x = AIR_ABS(x)/S;
  return _DDAQUARTIC(x, A)/(S*S*S);
}

static float
_nrrdDDA41_f(float x, const double *parm) {
  float S, A;

  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  x = AIR_ABS(x)/S;
  return _DDAQUARTIC(x, A)/(S*S*S);
}

static void
_nrrdDDA4N_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, A;
  size_t i;

  S = parm[0]; A = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDAQUARTIC(t, A)/(S*S*S);
  }
}

static void
_nrrdDDA4N_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, A;
  size_t i;

  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDAQUARTIC(t, A)/(S*S*S);
  }
}

static NrrdKernel
_nrrdKernelDDA4 = {
  "AquarticDD",
  2, _nrrdDDA4Sup, returnZero,
  _nrrdDDA41_f, _nrrdDDA4N_f, _nrrdDDA41_d, _nrrdDDA4N_d
};
NrrdKernel *const
nrrdKernelAQuarticDD = &_nrrdKernelDDA4;

/* ------------------------------------------------------------ */

/*
** This is the unique, four-sample support, quintic, C^3 kernel, with 1st and
** 3rd derivatives zero at origin, which integrates to unity on [-2,2], which
** exactly reconstructs 0th and 1st order polynomials.  Unfortunately it does
** NOT reconstruct 2nd order polynomials, so its not very useful.  It worse
** than "one step down" from c4hexic (see below), since while its support and
** polynomial power is one less than c4hexic, it cannot reconstruct
** parabolas; c4hexic can reconstruct cubics.
**
** The same kernel is also available as nrrdKernelTMF w/ D,C,A = -1,3,2 ==
** TMF_dn_c3_2ef == "tmf:n,3,2" == nrrdKernelTMF[0][4][2], the only advantage
** here being that you have access to the first and second derivatives of
** this quintic kernel as nrrdKernelC3QuinticD and nrrdKernelC3QuinticDD.
**
** By the way, TMF_d0_c3_3ef == TMF_dn_c3_3ef == "tmf:n,3,3", which can (by
** definition) reconstruct parabolas, has four-sample support, and has
** piecewise polynomial order *seven*
*/

#define _C3QUINTIC(x) \
  (x >= 2.0 ? 0 :                                             \
  (x >= 1.0                                                   \
   ? 0.8 + x*x*(-2 + x*(2 + x*(-0.75 + x*0.1)))               \
   : 0.7 + x*x*(-1 + x*x*(0.75 - x*0.3))))

static double
_c3quint1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _C3QUINTIC(x);
}

static float
_c3quint1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, _C3QUINTIC(x));
}

static void
_c3quintN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _C3QUINTIC(t);
  }
}

static void
_c3quintN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _C3QUINTIC(t));
  }
}

static NrrdKernel
_c3quint = {
  "C3Quintic",
  0, returnTwo, returnOne,
  _c3quint1_f,   _c3quintN_f,   _c3quint1_d,   _c3quintN_d
};
NrrdKernel *const
nrrdKernelC3Quintic = &_c3quint;

/* ------------------------------------------------------------ */

#define _DC3QUINTIC(x) \
  (x >= 2.0 ? 0 :                                             \
  (x >= 1.0                                                   \
   ? x*(-4 + x*(6 + x*(-3 + x*0.5)))                          \
   : x*(-2 + x*x*(3 - x*1.5))))

static double
_Dc3quint1_d(double x, const double *parm) {
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  return sgn*_DC3QUINTIC(x);
}

static float
_Dc3quint1_f(float x, const double *parm) {
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  return AIR_CAST(float, sgn*_DC3QUINTIC(x));
}

static void
_Dc3quintN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  int sgn;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DC3QUINTIC(t);
  }
}

static void
_Dc3quintN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  int sgn;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DC3QUINTIC(t));
  }
}

static NrrdKernel
_nrrdKernelDC3Quintic = {
  "C3QuinticD",
  0, returnTwo, returnZero,
  _Dc3quint1_f,  _Dc3quintN_f,  _Dc3quint1_d,  _Dc3quintN_d
};
NrrdKernel *const
nrrdKernelC3QuinticD = &_nrrdKernelDC3Quintic;

/* ------------------------------------------------------------ */

#define _DDC3QUINTIC(x) \
  (x >= 2.0 ? 0 :                                             \
  (x >= 1.0                                                   \
   ? -4 + x*(12 + x*(-9 + x*2))                               \
   : -2 + x*x*(9 - x*6)))

static double
_DDc3quint1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _DDC3QUINTIC(x);
}

static float
_DDc3quint1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, _DDC3QUINTIC(x));
}

static void
_DDc3quintN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _DDC3QUINTIC(t);
  }
}

static void
_DDc3quintN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _DDC3QUINTIC(t));
  }
}

static NrrdKernel
_DDc3quint = {
  "C3QuinticDD",
  0, returnTwo, returnZero,
  _DDc3quint1_f,   _DDc3quintN_f,   _DDc3quint1_d,   _DDc3quintN_d
};
NrrdKernel *const
nrrdKernelC3QuinticDD = &_DDc3quint;

/* ------------------------------------------------------------ */

/*
** This is the unique, 6-sample support, hexic, C^4 kernel, with 1st and 3rd
** derivatives zero at origin, which integrates to unity on [-3,3], with 3rd
** order Taylor accuracy (errors start showing up when reconstructing 4th
** order polynomials) It doesn't interpolate, but its close, and it rings
** once.
**
** this is awfully close to, but not quite the same as, "tmf:n,3,4" ==
** TMF_dn_c3_4ef == nrrdKernelTMF[0][4][4], which is only C^3 smooth
*/

#define _C4HEXIC(x) \
  (x >= 3.0 \
   ? 0 \
   : (x >= 2.0 \
      ? 1539.0/160.0 + x*(-189.0/8.0 + x*(747.0/32.0 + x*(-12.0 + x*(109.0/32.0 + x*(-61.0/120.0 + x/32.0))))) \
      : (x >= 1.0 \
         ? 3.0/160.0 + x*(35.0/8.0 + x*(-341.0/32.0 + x*(10.0 + x*(-147.0/32.0 + x*(25.0/24.0 - x*3.0/32.0))))) \
         : 69.0/80.0 + x*x*(-23.0/16.0 + x*x*(19.0/16.0 + x*(-7.0/12.0 + x/16.0)))  )))

static double
_c4hex1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _C4HEXIC(x);
}

static float
_c4hex1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, _C4HEXIC(x));
}

static void
_c4hexN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _C4HEXIC(t);
  }
}

static void
_c4hexN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _C4HEXIC(t));
  }
}

static NrrdKernel
_c4hex = {
  "C4Hexic",
  0, returnThree, returnOne,
  _c4hex1_f,   _c4hexN_f,   _c4hex1_d,   _c4hexN_d
};
NrrdKernel *const
nrrdKernelC4Hexic = &_c4hex;

/* ------------------------------------------------------------ */

#define _DC4HEXIC(x) \
  (x >= 3.0 \
   ? 0 \
   : (x >= 2.0 \
      ? -189.0/8.0 + x*(747.0/16.0 + x*(-36.0 + x*(109.0/8.0 + x*(-61.0/24.0 + x*(3.0/16.0))))) \
      : (x >= 1.0 \
         ? 35.0/8.0 + x*(-341.0/16.0 + x*(30 + x*(-147.0/8.0 + x*(125.0/24.0 + x*(-9.0/16.0))))) \
         : x*(-23.0/8.0 + x*x*(19.0/4.0 + x*(-35.0/12.0 + x*(3.0/8.0))))  )))

static double
_Dc4hex1_d(double x, const double *parm) {
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  return sgn*_DC4HEXIC(x);
}

static float
_Dc4hex1_f(float x, const double *parm) {
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  return AIR_CAST(float, sgn*_DC4HEXIC(x));
}

static void
_Dc4hexN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  int sgn;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DC4HEXIC(t);
  }
}

static void
_Dc4hexN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  int sgn;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DC4HEXIC(t));
  }
}

static NrrdKernel
_nrrdKernelDC4hexic = {
  "C4HexicD",
  0, returnThree, returnZero,
  _Dc4hex1_f,  _Dc4hexN_f,  _Dc4hex1_d,  _Dc4hexN_d
};
NrrdKernel *const
nrrdKernelC4HexicD = &_nrrdKernelDC4hexic;

/* ------------------------------------------------------------ */

#define _DDC4HEXIC(x) \
  (x >= 3.0 \
   ? 0 \
   : (x >= 2.0 \
      ? 747.0/16.0 + x*(-72.0 + x*(327.0/8.0 + x*(-61.0/6.0 + x*15.0/16.0))) \
      : (x >= 1.0 \
         ? -341.0/16.0 + x*(60 + x*(-441.0/8.0 + x*(125.0/6.0 - x*45.0/16.0))) \
         : -23.0/8.0 + x*x*(57.0/4.0 + x*(-35.0/3.0 + x*(15.0/8.0)))  )))

static double
_DDc4hex1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return _DDC4HEXIC(x);
}

static float
_DDc4hex1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  return AIR_CAST(float, _DDC4HEXIC(x));
}

static void
_DDc4hexN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _DDC4HEXIC(t);
  }
}

static void
_DDc4hexN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _DDC4HEXIC(t));
  }
}

static NrrdKernel
_DDc4hex = {
  "C4HexicDD",
  0, returnThree, returnZero,
  _DDc4hex1_f,   _DDc4hexN_f,   _DDc4hex1_d,   _DDc4hexN_d
};
NrrdKernel *const
nrrdKernelC4HexicDD = &_DDc4hex;

/* ------------------------------------------------------------ */

#define _DDDC4HEXIC(x)                                                  \
  (x >= 3.0                                                             \
   ? 0                                                                  \
   : (x >= 2.0                                                          \
      ? -72.0 + x*(327.0/4.0 + x*(-61.0/2.0 + 15*x/4))                  \
      : (x >= 1.0                                                       \
         ? 60 + x*(-441.0/4.0 + x*(125.0/2.0 - 45*x/4))                 \
         : x*(57.0/2.0  + x*(-35 + 15*x/2))                             \
         )))

static double
_DDDc4hex1_d(double x, const double *parm) {
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  return sgn*_DDDC4HEXIC(x);
}

static float
_DDDc4hex1_f(float x, const double *parm) {
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  return AIR_CAST(float, sgn*_DDDC4HEXIC(x));
}

static void
_DDDc4hexN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  int sgn;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DDDC4HEXIC(t);
  }
}

static void
_DDDc4hexN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  int sgn;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DDDC4HEXIC(t));
  }
}

static NrrdKernel
_nrrdKernelDDDC4hexic = {
  "C4HexicDDD",
  0, returnThree, returnZero,
  _DDDc4hex1_f,  _DDDc4hexN_f,  _DDDc4hex1_d,  _DDDc4hexN_d
};
NrrdKernel *const
nrrdKernelC4HexicDDD = &_nrrdKernelDDDC4hexic;


/* ------------- approximate inverse of c4h ------------------------- */
/* see comments around "_bspl3_ANI" in bsplKernel.c */

static double
_c4hex_ANI_kvals[12] = {
  1.1906949847244948336,
  -0.13537708971729194940,
  0.047024535491780434571,
  -0.0088462060502312555095,
  0.0022443498051487024049,
  -0.00048639680369511914410,
  0.00011421848629250278186,
  -0.000025727759438407893986,
  5.9204264168395963454e-6,
  -1.3468552403175349134e-6,
  3.0637649910394681441e-7,
  -5.5762487950611026674e-8};

static double
_c4hex_ANI_sup(const double *parm) {
  AIR_UNUSED(parm);
  return 12.5;
}

#define C4HEX_ANI(ret, tmp, x)                  \
  tmp = AIR_CAST(unsigned int, x+0.5);          \
  if (tmp < 12) {                               \
    ret = _c4hex_ANI_kvals[tmp];                \
  } else {                                      \
    ret = 0.0;                                  \
  }

static double
_c4hex_ANI_1d(double x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);
  ax = AIR_ABS(x);
  C4HEX_ANI(r, tmp, ax);
  return r;
}

static float
_c4hex_ANI_1f(float x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);
  ax = AIR_ABS(x);
  C4HEX_ANI(r, tmp, ax);
  return AIR_CAST(float, r);
}

static void
_c4hex_ANI_Nd(double *f, const double *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    C4HEX_ANI(r, tmp, ax);
    f[i] = r;
  }
}

static void
_c4hex_ANI_Nf(float *f, const float *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    C4HEX_ANI(r, tmp, ax);
    f[i] = AIR_CAST(float, r);
  }
}

static NrrdKernel
_nrrdKernelC4HexicApproxInverse = {
  "C4HexicAI", 0,
  _c4hex_ANI_sup, returnOne,
  _c4hex_ANI_1f, _c4hex_ANI_Nf,
  _c4hex_ANI_1d, _c4hex_ANI_Nd
};
NrrdKernel *const
nrrdKernelC4HexicApproxInverse = &_nrrdKernelC4HexicApproxInverse;

/* ------------------------- c5septic ------------------------------ */

/*
** This is the unique, 8-sample support, C^5 kernel, piecewise order-7
** with 4th order Taylor accuracy (errors start showing up when
** reconstructing 5th order polynomials).  Coincidentally, it has zero
** 1st and 3rd deriv at the origin, and it integrates to unity on
** [-4,4]. It doesn't interpolate, but its close; it rings twice.  */

#define _C5SEPT0(x) (0.9379776601998824 + x*x*(-1.654320987654321 + x*x*(1.073045267489712 + x*x*(-0.44997427983539096 + x*0.13978909465020575))))
#define _C5SEPT1(x) (0.04651675485008818 + x*(-0.7377829218106996 + x*(0.9699074074074074 + x*(0.18531378600823045 + x*(-0.7839506172839507 + x*(0.2357253086419753 + x*(0.12021604938271604 - x*0.054552469135802466)))))))
#define _C5SEPT2(x) (-0.01860670194003527 + x*(0.14022633744855967 + x*(-0.16296296296296298 + x*(-0.09825102880658436 + x*(0.28858024691358025 + x*(-0.18858024691358025 + x*(0.04405864197530864 - x*0.0013631687242798354)))))))
#define _C5SEPT3(x) (0.003101116990005879 + x*(-0.014223251028806585 + x*(0.02021604938271605 + x*(0.003729423868312757 + x*(-0.0411522633744856 + x*(0.04714506172839506 + x*(-0.023199588477366254 + x*0.004383450911228689)))))))
#define _C5SEPT(i, x)                           \
  (0 == i ? _C5SEPT0(x)                         \
   : (1 == i ? _C5SEPT1(x)                      \
      : (2 == i ? _C5SEPT2(x)                   \
         : (3 == i ? _C5SEPT3(x)                \
            : 0.0))))

static double
_c5sept1_d(double x, const double *parm) {
  unsigned int xi;
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  xi = AIR_CAST(unsigned int, x);
  x -= xi;
  return _C5SEPT(xi, x);
}

static float
_c5sept1_f(float x, const double *parm) {
  unsigned int xi;
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  xi = AIR_CAST(unsigned int, x);
  x -= AIR_CAST(float, xi);
  return AIR_CAST(float, _C5SEPT(xi, x));
}

static void
_c5septN_d(double *f, const double *x, size_t len, const double *parm) {
  unsigned int ti;
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    ti = AIR_CAST(unsigned int, t);
    t -= ti;
    f[i] = _C5SEPT(ti, t);
  }
}

static void
_c5septN_f(float *f, const float *x, size_t len, const double *parm) {
  unsigned int ti;
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    ti = AIR_CAST(unsigned int, t);
    t -= AIR_CAST(float, ti);
    f[i] = AIR_CAST(float, _C5SEPT(ti, t));
  }
}

static NrrdKernel
_c5sept = {
  "C5Septic",
  0, returnFour, returnOne,
  _c5sept1_f,   _c5septN_f,   _c5sept1_d,   _c5septN_d
};
NrrdKernel *const
nrrdKernelC5Septic = &_c5sept;

#define _DC5SEPT0(x) (x*(-3.308641975308642 + x*x*(4.292181069958848 + x*x*(-2.6998456790123457 + x*0.9785236625514403))))
#define _DC5SEPT1(x) (-0.7377829218106996 + x*(1.9398148148148149 + x*(0.5559413580246914 + x*(-3.1358024691358026 + x*(1.1786265432098766 + x*(0.7212962962962963 - x*0.3818672839506173))))))
#define _DC5SEPT2(x) (0.14022633744855967 + x*(-0.32592592592592595 + x*(-0.29475308641975306 + x*(1.154320987654321 + x*(-0.9429012345679012 + x*(0.26435185185185184 - x*0.009542181069958848))))))
#define _DC5SEPT3(x) (-0.014223251028806585 + x*(0.0404320987654321 + x*(0.011188271604938271 + x*(-0.1646090534979424 + x*(0.2357253086419753 + x*(-0.13919753086419753 + x*0.03068415637860082))))))
#define _DC5SEPT(i, x)                           \
  (0 == i ? _DC5SEPT0(x)                         \
   : (1 == i ? _DC5SEPT1(x)                      \
      : (2 == i ? _DC5SEPT2(x)                   \
         : (3 == i ? _DC5SEPT3(x)                \
            : 0.0))))

static double
_dc5sept1_d(double x, const double *parm) {
  unsigned int xi;
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  xi = AIR_CAST(unsigned int, x);
  x -= xi;
  return sgn*_DC5SEPT(xi, x);
}

static float
_dc5sept1_f(float x, const double *parm) {
  unsigned int xi;
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  xi = AIR_CAST(unsigned int, x);
  x -= AIR_CAST(float, xi);
  return AIR_CAST(float, sgn*_DC5SEPT(xi, x));
}

static void
_dc5septN_d(double *f, const double *x, size_t len, const double *parm) {
  unsigned int ti;
  double t;
  size_t i;
  int sgn;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    ti = AIR_CAST(unsigned int, t);
    t -= ti;
    f[i] = sgn*_DC5SEPT(ti, t);
  }
}

static void
_dc5septN_f(float *f, const float *x, size_t len, const double *parm) {
  unsigned int ti;
  float t;
  size_t i;
  int sgn = 1;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    ti = AIR_CAST(unsigned int, t);
    t -= AIR_CAST(float, ti);
    f[i] = AIR_CAST(float, sgn*_DC5SEPT(ti, t));
  }
}

static NrrdKernel
_dc5sept = {
  "C5SepticD",
  0, returnFour, returnZero,
  _dc5sept1_f,   _dc5septN_f,   _dc5sept1_d,   _dc5septN_d
};
NrrdKernel *const
nrrdKernelC5SepticD = &_dc5sept;

#define _DDC5SEPT0(x) (-3.308641975308642 + x*x*(12.876543209876543 + x*x*(-13.499228395061728 + x*5.871141975308642)))
#define _DDC5SEPT1(x) (1.9398148148148149 + x*(1.1118827160493827 + x*(-9.407407407407407 + x*(4.714506172839506 + x*(3.6064814814814814 - x*2.2912037037037036)))))
#define _DDC5SEPT2(x) (-0.32592592592592595 + x*(-0.5895061728395061 + x*(3.462962962962963 + x*(-3.771604938271605 + x*(1.3217592592592593 - x*0.05725308641975309)))))
#define _DDC5SEPT3(x) (0.0404320987654321 + x*(0.022376543209876542 + x*(-0.49382716049382713 + x*(0.9429012345679012 + x*(-0.6959876543209876 + x*0.18410493827160493)))))
#define _DDC5SEPT(i, x)                           \
  (0 == i ? _DDC5SEPT0(x)                         \
   : (1 == i ? _DDC5SEPT1(x)                      \
      : (2 == i ? _DDC5SEPT2(x)                   \
         : (3 == i ? _DDC5SEPT3(x)                \
            : 0.0))))

static double
_ddc5sept1_d(double x, const double *parm) {
  unsigned int xi;
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  xi = AIR_CAST(unsigned int, x);
  x -= xi;
  return _DDC5SEPT(xi, x);
}

static float
_ddc5sept1_f(float x, const double *parm) {
  unsigned int xi;
  AIR_UNUSED(parm);
  x = AIR_ABS(x);
  xi = AIR_CAST(unsigned int, x);
  x -= AIR_CAST(float, xi);
  return AIR_CAST(float, _DDC5SEPT(xi, x));
}

static void
_ddc5septN_d(double *f, const double *x, size_t len, const double *parm) {
  unsigned int ti;
  double t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    ti = AIR_CAST(unsigned int, t);
    t -= ti;
    f[i] = _DDC5SEPT(ti, t);
  }
}

static void
_ddc5septN_f(float *f, const float *x, size_t len, const double *parm) {
  unsigned int ti;
  float t;
  size_t i;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    ti = AIR_CAST(unsigned int, t);
    t -= AIR_CAST(float, ti);
    f[i] = AIR_CAST(float, _DDC5SEPT(ti, t));
  }
}

static NrrdKernel
_ddc5sept = {
  "C5SepticDD",
  0, returnFour, returnZero,
  _ddc5sept1_f,   _ddc5septN_f,   _ddc5sept1_d,   _ddc5septN_d
};
NrrdKernel *const
nrrdKernelC5SepticDD = &_ddc5sept;

#define _DDDC5SEPT0(x) (x*(25.75308641975309 + x*x*(-53.99691358024691 + x*29.35570987654321)))
#define _DDDC5SEPT1(x) (1.111882716049383 + x*(-18.81481481481481 + x*(14.14351851851852 + x*(14.42592592592593 - x*11.45601851851852))))
#define _DDDC5SEPT2(x) (-0.5895061728395062 + x*(6.925925925925926 + x*(-11.31481481481481 + x*(5.287037037037037 - x*0.2862654320987654))))
#define _DDDC5SEPT3(x) (0.02237654320987654 + x*(-0.9876543209876543 + x*(2.828703703703704 + x*(-2.783950617283951 + x*0.9205246913580247))))
#define _DDDC5SEPT(i, x)                           \
  (0 == i ? _DDDC5SEPT0(x)                         \
   : (1 == i ? _DDDC5SEPT1(x)                      \
      : (2 == i ? _DDDC5SEPT2(x)                   \
         : (3 == i ? _DDDC5SEPT3(x)                \
            : 0.0))))

static double
_dddc5sept1_d(double x, const double *parm) {
  unsigned int xi;
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  xi = AIR_CAST(unsigned int, x);
  x -= xi;
  return sgn*_DDDC5SEPT(xi, x);
}

static float
_dddc5sept1_f(float x, const double *parm) {
  unsigned int xi;
  int sgn = 1;
  AIR_UNUSED(parm);
  if (x < 0) { x = -x; sgn = -1; }
  xi = AIR_CAST(unsigned int, x);
  x -= AIR_CAST(float, xi);
  return AIR_CAST(float, sgn*_DDDC5SEPT(xi, x));
}

static void
_dddc5septN_d(double *f, const double *x, size_t len, const double *parm) {
  unsigned int ti;
  double t;
  size_t i;
  int sgn;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    ti = AIR_CAST(unsigned int, t);
    t -= ti;
    f[i] = sgn*_DDDC5SEPT(ti, t);
  }
}

static void
_dddc5septN_f(float *f, const float *x, size_t len, const double *parm) {
  unsigned int ti;
  float t;
  size_t i;
  int sgn = 1;
  AIR_UNUSED(parm);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    ti = AIR_CAST(unsigned int, t);
    t -= AIR_CAST(float, ti);
    f[i] = AIR_CAST(float, sgn*_DDDC5SEPT(ti, t));
  }
}

static NrrdKernel
_dddc5sept = {
  "C5SepticDDD",
  0, returnFour, returnZero,
  _dddc5sept1_f,   _dddc5septN_f,   _dddc5sept1_d,   _dddc5septN_d
};
NrrdKernel *const
nrrdKernelC5SepticDDD = &_dddc5sept;

/* note that this implies a much more accurate inverse than is given
   for the splines or other kernels; this is a consequence of GLK
   re-purposing the Mathematica expressions for the bpsln7 inverse,
   which is unfortunate: c5septic is nearly interpolating, so far
   fewer terms would suffice */
#define C5SEPT_AI_LEN 26
static double
_c5sept_ANI_kvals[C5SEPT_AI_LEN] = {
  1.072662863909143543451743,
  -0.05572032001443187610952953,
  0.02453993146603215267432700,
  -0.005922375635530229254855750,
  0.0009781882769025851448681918,
  -0.0002499281491108793120774480,
  0.00005196973116762945530292666,
  -0.00001090007030248955413371701,
  2.425581976693179040189747e-6,
  -5.143328756144314306529358e-7,
  1.109572595055083858393193e-7,
  -2.400323559797703961855318e-8,
  5.151959978856239469272136e-9,
  -1.111431289951609447815300e-9,
  2.394624249806782312051293e-10,
  -5.155654818408366273965675e-11,
  1.111091879440261302739584e-11,
  -2.393303168472914213194646e-12,
  5.155527554392687415035171e-13,
  -1.110707782883835600110634e-13,
  2.392644268109899456937863e-14,
  -5.154377464414088544322752e-15,
  1.110376957658466603291262e-15,
  -2.391459139266885907929865e-16,
  5.137528165538909945741180e-17,
  -9.024576392408067896802608e-18};

static double
_c5sept_ANI_sup(const double *parm) {
  AIR_UNUSED(parm);
  return C5SEPT_AI_LEN + 0.5;
}

static double
_c5sept_ANI_int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

#define C5SEPT_ANI(ret, tmp, x)                  \
  tmp = AIR_CAST(unsigned int, x+0.5);           \
  if (tmp < C5SEPT_AI_LEN) {                     \
    ret = _c5sept_ANI_kvals[tmp];                \
  } else {                                       \
    ret = 0.0;                                   \
  }

static double
_c5sept_ANI_1d(double x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);

  ax = AIR_ABS(x);
  C5SEPT_ANI(r, tmp, ax);
  return r;
}

static float
_c5sept_ANI_1f(float x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);

  ax = AIR_ABS(x);
  C5SEPT_ANI(r, tmp, ax);
  return AIR_CAST(float, r);
}

static void
_c5sept_ANI_Nd(double *f, const double *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);

  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    C5SEPT_ANI(r, tmp, ax);
    f[i] = r;
  }
}

static void
_c5sept_ANI_Nf(float *f, const float *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);

  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    C5SEPT_ANI(r, tmp, ax);
    f[i] = AIR_CAST(float, r);
  }
}

static NrrdKernel
_nrrdKernelC5SepticApproxInverse = {
  "C5SepticAI", 0,
  _c5sept_ANI_sup, _c5sept_ANI_int,
  _c5sept_ANI_1f, _c5sept_ANI_Nf,
  _c5sept_ANI_1d, _c5sept_ANI_Nd
};
NrrdKernel *const
nrrdKernelC5SepticApproxInverse = &_nrrdKernelC5SepticApproxInverse;

/* ------------------------------------------------------------ */

#define _GAUSS(x, sig, cut) ( \
   x >= sig*cut ? 0           \
   : exp(-x*x/(2.0*sig*sig))/(sig*2.50662827463100050241))

static double
_nrrdGInt(const double *parm) {
  double cut;

  cut = parm[1];
  return airErf(cut/sqrt(2.0));
}

static double
_nrrdGSup(const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  return sig*cut;
}

static double
_nrrdG1_d(double x, const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  x = AIR_ABS(x);
  return _GAUSS(x, sig, cut);
}

static float
_nrrdG1_f(float x, const double *parm) {
  float sig, cut;

  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  x = AIR_ABS(x);
  return AIR_CAST(float, _GAUSS(x, sig, cut));
}

static void
_nrrdGN_d(double *f, const double *x, size_t len, const double *parm) {
  double sig, cut, t;
  size_t i;

  sig = parm[0];
  cut = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _GAUSS(t, sig, cut);
  }
}

static void
_nrrdGN_f(float *f, const float *x, size_t len, const double *parm) {
  float sig, cut, t;
  size_t i;

  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _GAUSS(t, sig, cut));
  }
}

static NrrdKernel
_nrrdKernelG = {
  "gauss",
  2, _nrrdGSup,  _nrrdGInt,
  _nrrdG1_f,   _nrrdGN_f,   _nrrdG1_d,   _nrrdGN_d
};
NrrdKernel *const
nrrdKernelGaussian = &_nrrdKernelG;

/* ------------------------------------------------------------ */

#define _DISCRETEGAUSS(xx, sig, abscut)                         \
  (sig > 0                                                      \
   ? (xx > abscut                                               \
      ? 0                                                       \
      : airBesselInExpScaled(AIR_CAST(int, xx + 0.5), sig*sig)) \
   : xx <= 0.5)

#define _DGABSCUT(ret, sig, cut) \
  (ret) = 0.5 + ceil((sig)*(cut));  \
  (ret) = AIR_MAX(0.5, (ret))

static double
_nrrdDiscGaussianSup(const double *parm) {
  double ret;

  _DGABSCUT(ret, parm[0], parm[1]);
  return ret;
}

/* the functions are out of their usual definition order because we
   use the function evaluation to determine the integral, rather than
   trying to do it analytically */

static double
_nrrdDiscGaussian1_d(double xx, const double *parm) {
  double sig, cut;

  sig = parm[0];
  _DGABSCUT(cut, sig, parm[1]);
  xx = AIR_ABS(xx);
  return _DISCRETEGAUSS(xx, sig, cut);
}

static double
_nrrdDiscGaussianInt(const double *parm) {
  double sum, cut;
  int ii, supp;

  _DGABSCUT(cut, parm[0], parm[1]);
  supp = AIR_CAST(int, cut);
  sum = 0.0;
  for (ii=-supp; ii<=supp; ii++) {
    sum += _nrrdDiscGaussian1_d(ii, parm);
  }
  return sum;
}

static float
_nrrdDiscGaussian1_f(float xx, const double *parm) {
  double sig, cut;

  sig = parm[0];
  _DGABSCUT(cut, sig, parm[1]);
  xx = AIR_ABS(xx);
  return AIR_CAST(float, _DISCRETEGAUSS(xx, sig, cut));
}

static void
_nrrdDiscGaussianN_d(double *f, const double *x,
                  size_t len, const double *parm) {
  double sig, cut, tt;
  size_t ii;

  sig = parm[0];
  _DGABSCUT(cut, sig, parm[1]);
  for (ii=0; ii<len; ii++) {
    tt = AIR_ABS(x[ii]);
    f[ii] = _DISCRETEGAUSS(tt, sig, cut);
  }
}

static void
_nrrdDiscGaussianN_f(float *f, const float *x, size_t len, const double *parm) {
  double sig, cut, tt;
  size_t ii;

  sig = parm[0];
  _DGABSCUT(cut, sig, parm[1]);
  for (ii=0; ii<len; ii++) {
    tt = AIR_ABS(x[ii]);
    f[ii] = AIR_CAST(float, _DISCRETEGAUSS(tt, sig, cut));
  }
}

static NrrdKernel
_nrrdKernelDiscreteGaussian = {
  "discretegauss", 2,
  _nrrdDiscGaussianSup,  _nrrdDiscGaussianInt,
  _nrrdDiscGaussian1_f, _nrrdDiscGaussianN_f,
  _nrrdDiscGaussian1_d, _nrrdDiscGaussianN_d
};
NrrdKernel *const
nrrdKernelDiscreteGaussian = &_nrrdKernelDiscreteGaussian;

/* ------------------------------------------------------------ */

#define _DGAUSS(x, sig, cut) (                                               \
   x >= sig*cut ? 0                                                          \
   : -exp(-x*x/(2.0*sig*sig))*x/(sig*sig*sig*2.50662827463100050241))

static double
_nrrdDGInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0;
}

static double
_nrrdDGSup(const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  return sig*cut;
}

static double
_nrrdDG1_d(double x, const double *parm) {
  double sig, cut;
  int sgn = 1;

  sig = parm[0];
  cut = parm[1];
  if (x < 0) { x = -x; sgn = -1; }
  return sgn*_DGAUSS(x, sig, cut);
}

static float
_nrrdDG1_f(float x, const double *parm) {
  float sig, cut;
  int sgn = 1;

  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  if (x < 0) { x = -x; sgn = -1; }
  return AIR_CAST(float, sgn*_DGAUSS(x, sig, cut));
}

static void
_nrrdDGN_d(double *f, const double *x, size_t len, const double *parm) {
  double sig, cut, t;
  size_t i;
  int sgn;

  sig = parm[0];
  cut = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DGAUSS(t, sig, cut);
  }
}

static void
_nrrdDGN_f(float *f, const float *x, size_t len, const double *parm) {
  float sig, cut, t;
  size_t i;
  int sgn;

  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DGAUSS(t, sig, cut));
  }
}

static NrrdKernel
_nrrdKernelDG = {
  "gaussD",
  2, _nrrdDGSup,  _nrrdDGInt,
  _nrrdDG1_f,   _nrrdDGN_f,   _nrrdDG1_d,   _nrrdDGN_d
};
NrrdKernel *const
nrrdKernelGaussianD = &_nrrdKernelDG;

/* ------------------------------------------------------------ */

#define _DDGAUSS(x, sig, cut) ( \
   x >= sig*cut ? 0             \
   : exp(-x*x/(2.0*sig*sig))*(x*x-sig*sig) /       \
     (sig*sig*sig*sig*sig*2.50662827463100050241))

static double
_nrrdDDGInt(const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  return -0.79788456080286535587*cut*exp(-cut*cut/2)/(sig*sig);
}

static double
_nrrdDDGSup(const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  return sig*cut;
}

static double
_nrrdDDG1_d(double x, const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  x = AIR_ABS(x);
  return _DDGAUSS(x, sig, cut);
}

static float
_nrrdDDG1_f(float x, const double *parm) {
  float sig, cut;

  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  x = AIR_ABS(x);
  return AIR_CAST(float, _DDGAUSS(x, sig, cut));
}

static void
_nrrdDDGN_d(double *f, const double *x, size_t len, const double *parm) {
  double sig, cut, t;
  size_t i;

  sig = parm[0];
  cut = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _DDGAUSS(t, sig, cut);
  }
}

static void
_nrrdDDGN_f(float *f, const float *x, size_t len, const double *parm) {
  float sig, cut, t;
  size_t i;

  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _DDGAUSS(t, sig, cut));
  }
}

static NrrdKernel
_nrrdKernelDDG = {
  "gaussDD",
  2, _nrrdDDGSup,  _nrrdDDGInt,
  _nrrdDDG1_f,   _nrrdDDGN_f,   _nrrdDDG1_d,   _nrrdDDGN_d
};
NrrdKernel *const
nrrdKernelGaussianDD = &_nrrdKernelDDG;


/* ------------------------------------------------------------ */

NrrdKernel *
_nrrdKernelStrToKern(char *str) {

  if (!strcmp("zero", str))       return nrrdKernelZero;
  if (!strcmp("box", str))        return nrrdKernelBox;
  if (!strcmp("boxsup", str))     return nrrdKernelBoxSupportDebug;
  if (!strcmp("cos4sup", str))    return nrrdKernelCos4SupportDebug;
  if (!strcmp("cos4supd", str))   return nrrdKernelCos4SupportDebugD;
  if (!strcmp("cos4supdd", str))  return nrrdKernelCos4SupportDebugDD;
  if (!strcmp("cos4supddd", str)) return nrrdKernelCos4SupportDebugDDD;
  if (!strcmp("cheap", str))      return nrrdKernelCheap;
  if (!strcmp("hermiteflag", str))    return nrrdKernelHermiteScaleSpaceFlag;
  if (!strcmp("hermite", str))    return nrrdKernelHermiteScaleSpaceFlag;
  if (!strcmp("hermitess", str))    return nrrdKernelHermiteScaleSpaceFlag;
  if (!strcmp("herm", str))       return nrrdKernelHermiteScaleSpaceFlag;
  if (!strcmp("tent", str))       return nrrdKernelTent;
  if (!strcmp("tentd", str))      return nrrdKernelForwDiff;
  if (!strcmp("forwdiff", str))   return nrrdKernelForwDiff;
  if (!strcmp("fordif", str))     return nrrdKernelForwDiff;
  if (!strcmp("centdiff", str))   return nrrdKernelCentDiff;
  if (!strcmp("cendif", str))     return nrrdKernelCentDiff;
  if (!strcmp("bccubic", str))    return nrrdKernelBCCubic;
  if (!strcmp("cubic", str))      return nrrdKernelBCCubic;
  if (!strcmp("bccubicd", str))   return nrrdKernelBCCubicD;
  if (!strcmp("cubicd", str))     return nrrdKernelBCCubicD;
  if (!strcmp("bccubicdd", str))  return nrrdKernelBCCubicDD;
  if (!strcmp("cubicdd", str))    return nrrdKernelBCCubicDD;
  if (!strcmp("ctmr", str))       return nrrdKernelCatmullRom;
  if (!strcmp("catmull-rom", str)) return nrrdKernelCatmullRom;
  if (!strcmp("ctmrd", str))       return nrrdKernelCatmullRomD;
  if (!strcmp("catmull-romd", str)) return nrrdKernelCatmullRomD;
  if (!strcmp("ctmrdd", str))     return nrrdKernelCatmullRomDD;
  if (!strcmp("catmull-romdd", str)) return nrrdKernelCatmullRomDD;
  if (!strcmp("ctmrsup", str))    return nrrdKernelCatmullRomSupportDebug;
  if (!strcmp("ctmrsupd", str))   return nrrdKernelCatmullRomSupportDebugD;
  if (!strcmp("ctmrsupdd", str))   return nrrdKernelCatmullRomSupportDebugDD;
  if (!strcmp("aquartic", str))   return nrrdKernelAQuartic;
  if (!strcmp("quartic", str))    return nrrdKernelAQuartic;
  if (!strcmp("aquarticd", str))  return nrrdKernelAQuarticD;
  if (!strcmp("quarticd", str))   return nrrdKernelAQuarticD;
  if (!strcmp("aquarticdd", str)) return nrrdKernelAQuarticDD;
  if (!strcmp("quarticdd", str))  return nrrdKernelAQuarticDD;
  if (!strcmp("c3quintic", str))  return nrrdKernelC3Quintic;
  if (!strcmp("c3q", str))        return nrrdKernelC3Quintic;
  if (!strcmp("c3quinticd", str)) return nrrdKernelC3QuinticD;
  if (!strcmp("c3qd", str))       return nrrdKernelC3QuinticD;
  if (!strcmp("c3quinticdd", str)) return nrrdKernelC3QuinticDD;
  if (!strcmp("c3qdd", str))      return nrrdKernelC3QuinticDD;
  if (!strcmp("c4hexic", str))    return nrrdKernelC4Hexic;
  if (!strcmp("c4hai", str))      return nrrdKernelC4HexicApproxInverse;
  if (!strcmp("c4hexicai", str))  return nrrdKernelC4HexicApproxInverse;
  if (!strcmp("c4h", str))        return nrrdKernelC4Hexic;
  if (!strcmp("c4hexicd", str))   return nrrdKernelC4HexicD;
  if (!strcmp("c4hd", str))       return nrrdKernelC4HexicD;
  if (!strcmp("c4hexicdd", str))  return nrrdKernelC4HexicDD;
  if (!strcmp("c4hdd", str))      return nrrdKernelC4HexicDD;
  if (!strcmp("c4hexicddd", str)) return nrrdKernelC4HexicDDD;
  if (!strcmp("c4hddd", str))     return nrrdKernelC4HexicDDD;
  if (!strcmp("c5septic", str))   return nrrdKernelC5Septic;
  if (!strcmp("c5septicd", str))  return nrrdKernelC5SepticD;
  if (!strcmp("c5septicdd", str)) return nrrdKernelC5SepticDD;
  if (!strcmp("c5septicddd", str))return nrrdKernelC5SepticDDD;
  if (!strcmp("c5septicai", str)) return nrrdKernelC5SepticApproxInverse;
  if (!strcmp("gaussian", str))   return nrrdKernelGaussian;
  if (!strcmp("gauss", str))      return nrrdKernelGaussian;
  if (!strcmp("gaussiand", str))  return nrrdKernelGaussianD;
  if (!strcmp("gaussd", str))     return nrrdKernelGaussianD;
  if (!strcmp("gd", str))         return nrrdKernelGaussianD;
  if (!strcmp("gaussiandd", str)) return nrrdKernelGaussianDD;
  if (!strcmp("gaussdd", str))    return nrrdKernelGaussianDD;
  if (!strcmp("gdd", str))        return nrrdKernelGaussianDD;
  if (!strcmp("ds", str))         return nrrdKernelDiscreteGaussian;
  if (!strcmp("dscale", str))     return nrrdKernelDiscreteGaussian;
  if (!strcmp("dg", str))         return nrrdKernelDiscreteGaussian;
  if (!strcmp("dgauss", str))     return nrrdKernelDiscreteGaussian;
  if (!strcmp("dgaussian", str))  return nrrdKernelDiscreteGaussian;
  if (!strcmp("discretegauss", str))  return nrrdKernelDiscreteGaussian;
  if (!strcmp("hann", str))       return nrrdKernelHann;
  if (!strcmp("hannd", str))      return nrrdKernelHannD;
  if (!strcmp("hanndd", str))     return nrrdKernelHannDD;
  if (!strcmp("bkmn", str))       return nrrdKernelBlackman;
  if (!strcmp("black", str))      return nrrdKernelBlackman;
  if (!strcmp("blackman", str))   return nrrdKernelBlackman;
  if (!strcmp("bkmnd", str))      return nrrdKernelBlackmanD;
  if (!strcmp("blackd", str))     return nrrdKernelBlackmanD;
  if (!strcmp("blackmand", str))  return nrrdKernelBlackmanD;
  if (!strcmp("bkmndd", str))     return nrrdKernelBlackmanDD;
  if (!strcmp("blackdd", str))    return nrrdKernelBlackmanDD;
  if (!strcmp("blackmandd", str)) return nrrdKernelBlackmanDD;
  if (!strcmp("bspl1", str))      return nrrdKernelBSpline1;
  if (!strcmp("bspln1", str))     return nrrdKernelBSpline1;
  if (!strcmp("bspl1d", str))     return nrrdKernelBSpline1D;
  if (!strcmp("bspln1d", str))    return nrrdKernelBSpline1D;
  if (!strcmp("bspl2", str))      return nrrdKernelBSpline2;
  if (!strcmp("bspln2", str))     return nrrdKernelBSpline2;
  if (!strcmp("bspl2d", str))     return nrrdKernelBSpline2D;
  if (!strcmp("bspln2d", str))    return nrrdKernelBSpline2D;
  if (!strcmp("bspl2dd", str))    return nrrdKernelBSpline2DD;
  if (!strcmp("bspln2dd", str))   return nrrdKernelBSpline2DD;
  if (!strcmp("bspl3", str))      return nrrdKernelBSpline3;
  if (!strcmp("bspln3", str))     return nrrdKernelBSpline3;
  if (!strcmp("bspl3ai", str))    return nrrdKernelBSpline3ApproxInverse;
  if (!strcmp("bspln3ai", str))   return nrrdKernelBSpline3ApproxInverse;
  if (!strcmp("bspl3d", str))     return nrrdKernelBSpline3D;
  if (!strcmp("bspln3d", str))    return nrrdKernelBSpline3D;
  if (!strcmp("bspl3dd", str))    return nrrdKernelBSpline3DD;
  if (!strcmp("bspln3dd", str))   return nrrdKernelBSpline3DD;
  if (!strcmp("bspl3ddd", str))   return nrrdKernelBSpline3DDD;
  if (!strcmp("bspln3ddd", str))  return nrrdKernelBSpline3DDD;
  if (!strcmp("bspl4", str))      return nrrdKernelBSpline4;
  if (!strcmp("bspln4", str))     return nrrdKernelBSpline4;
  if (!strcmp("bspl4d", str))     return nrrdKernelBSpline4D;
  if (!strcmp("bspln4d", str))    return nrrdKernelBSpline4D;
  if (!strcmp("bspl4dd", str))    return nrrdKernelBSpline4DD;
  if (!strcmp("bspln4dd", str))   return nrrdKernelBSpline4DD;
  if (!strcmp("bspl4ddd", str))   return nrrdKernelBSpline4DDD;
  if (!strcmp("bspln4ddd", str))  return nrrdKernelBSpline4DDD;
  if (!strcmp("bspl5", str))      return nrrdKernelBSpline5;
  if (!strcmp("bspln5", str))     return nrrdKernelBSpline5;
  if (!strcmp("bspl5ai", str))    return nrrdKernelBSpline5ApproxInverse;
  if (!strcmp("bspln5ai", str))   return nrrdKernelBSpline5ApproxInverse;
  if (!strcmp("bspl5d", str))     return nrrdKernelBSpline5D;
  if (!strcmp("bspln5d", str))    return nrrdKernelBSpline5D;
  if (!strcmp("bspl5dd", str))    return nrrdKernelBSpline5DD;
  if (!strcmp("bspln5dd", str))   return nrrdKernelBSpline5DD;
  if (!strcmp("bspl5ddd", str))   return nrrdKernelBSpline5DDD;
  if (!strcmp("bspln5ddd", str))  return nrrdKernelBSpline5DDD;
  if (!strcmp("bspl6", str))      return nrrdKernelBSpline6;
  if (!strcmp("bspln6", str))     return nrrdKernelBSpline6;
  if (!strcmp("bspl6d", str))     return nrrdKernelBSpline6D;
  if (!strcmp("bspln6d", str))    return nrrdKernelBSpline6D;
  if (!strcmp("bspl6dd", str))    return nrrdKernelBSpline6DD;
  if (!strcmp("bspln6dd", str))   return nrrdKernelBSpline6DD;
  if (!strcmp("bspl6ddd", str))   return nrrdKernelBSpline6DDD;
  if (!strcmp("bspln6ddd", str))  return nrrdKernelBSpline6DDD;
  if (!strcmp("bspl7", str))      return nrrdKernelBSpline7;
  if (!strcmp("bspln7", str))     return nrrdKernelBSpline7;
  if (!strcmp("bspl7ai", str))    return nrrdKernelBSpline7ApproxInverse;
  if (!strcmp("bspln7ai", str))   return nrrdKernelBSpline7ApproxInverse;
  if (!strcmp("bspl7d", str))     return nrrdKernelBSpline7D;
  if (!strcmp("bspln7d", str))    return nrrdKernelBSpline7D;
  if (!strcmp("bspl7dd", str))    return nrrdKernelBSpline7DD;
  if (!strcmp("bspln7dd", str))   return nrrdKernelBSpline7DD;
  if (!strcmp("bspl7ddd", str))   return nrrdKernelBSpline7DDD;
  if (!strcmp("bspln7ddd", str))  return nrrdKernelBSpline7DDD;
  return NULL;
}

/* this returns a number between -1 and max;
   it does NOT do the increment-by-one;
   it does NOT do range checking */
int
_nrrdKernelParseTMFInt(int *val, char *str) {
  static const char me[]="nrrdKernelParseTMFInt";

  if (!strcmp("n", str)) {
    *val = -1;
  } else {
    if (1 != sscanf(str, "%d", val)) {
      biffAddf(NRRD, "%s: couldn't parse \"%s\" as int", me, str);
      return 1;
    }
  }
  return 0;
}

int
nrrdKernelParse(const NrrdKernel **kernelP,
                double *parm, const char *_str) {
  static const char me[]="nrrdKernelParse";
  char str[AIR_STRLEN_HUGE],
    kstr[AIR_STRLEN_MED], *_pstr=NULL, *pstr,
    *tmfStr[4] = {NULL, NULL, NULL, NULL};
  int tmfD, tmfC, tmfA;
  unsigned int jj, haveParm, needParm;
  airArray *mop;

  if (!(kernelP && parm && _str)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }

  /* [jorik] (if i understood this correctly) parm is always of length
  ** NRRD_KERNEL_PARMS_NUM. We have to clear all parameters here, since
  ** nrrdKernelSet copies all arguments into its own array later, and
  ** copying uninitialised memory is bad (it traps my memory debugger).
  */
  for (jj=0; jj<NRRD_KERNEL_PARMS_NUM; jj++) {
    parm[jj] = 0;
  }

  airStrcpy(str, AIR_STRLEN_HUGE, _str);
  strcpy(kstr, "");
  pstr = NULL;
  pstr = strchr(str, ':');
  if (pstr) {
    *pstr = '\0';
    _pstr = ++pstr;
  }
  strcpy(kstr, str);
  airToLower(kstr);
  mop = airMopNew();
  /* first see if its a TMF, then try parsing it as the other stuff */
  if (kstr == strstr(kstr, "tmf")) {
    if (4 == airParseStrS(tmfStr, pstr, ",", 4)) {
      airMopAdd(mop, tmfStr[0], airFree, airMopAlways);
      airMopAdd(mop, tmfStr[1], airFree, airMopAlways);
      airMopAdd(mop, tmfStr[2], airFree, airMopAlways);
      airMopAdd(mop, tmfStr[3], airFree, airMopAlways);
      /* a TMF with a parameter: D,C,A,a */
      if (1 != sscanf(tmfStr[3], "%lg", parm)) {
        biffAddf(NRRD, "%s: couldn't parse TMF parameter \"%s\" as double",
                 me, tmfStr[3]);
        airMopError(mop); return 1;
      }
    } else if (3 == airParseStrS(tmfStr, pstr, ",", 3)) {
      airMopAdd(mop, tmfStr[0], airFree, airMopAlways);
      airMopAdd(mop, tmfStr[1], airFree, airMopAlways);
      airMopAdd(mop, tmfStr[2], airFree, airMopAlways);
      /* a TMF without a parameter: D,C,A ==> a=0.0 */
      parm[0] = 0.0;
    } else {
      biffAddf(NRRD, "%s: TMF kernels require 3 arguments D, C, A "
               "in the form tmf:D,C,A", me);
      airMopError(mop); return 1;
    }
    if (_nrrdKernelParseTMFInt(&tmfD, tmfStr[0])
        || _nrrdKernelParseTMFInt(&tmfC, tmfStr[1])
        || _nrrdKernelParseTMFInt(&tmfA, tmfStr[2])) {
      biffAddf(NRRD, "%s: problem parsing \"%s,%s,%s\" as D,C,A "
               "for TMF kernel", me, tmfStr[0], tmfStr[1], tmfStr[2]);
      airMopError(mop); return 1;
    }
    if (!AIR_IN_CL(-1, tmfD, (int)nrrdKernelTMF_maxD)) {
      biffAddf(NRRD, "%s: derivative value %d outside range [-1,%d]",
               me, tmfD, nrrdKernelTMF_maxD);
      airMopError(mop); return 1;
    }
    if (!AIR_IN_CL(-1, tmfC, (int)nrrdKernelTMF_maxC)) {
      biffAddf(NRRD, "%s: continuity value %d outside range [-1,%d]",
               me, tmfC, nrrdKernelTMF_maxC);
      airMopError(mop); return 1;
    }
    if (!AIR_IN_CL(1, tmfA, (int)nrrdKernelTMF_maxA)) {
      biffAddf(NRRD, "%s: accuracy value %d outside range [1,%d]",
               me, tmfA, nrrdKernelTMF_maxA);
      airMopError(mop); return 1;
    }
    /*
    fprintf(stderr, "!%s: D,C,A = %d,%d,%d --> %d,%d,%d\n", me,
            tmfD, tmfC, tmfA, tmfD+1, tmfC+1, tmfA);
    */
    *kernelP = nrrdKernelTMF[tmfD+1][tmfC+1][tmfA];
  } else {
    /* its not a TMF */
    if (!(*kernelP = _nrrdKernelStrToKern(kstr))) {
      biffAddf(NRRD, "%s: kernel \"%s\" not recognized", me, kstr);
      airMopError(mop); return 1;
    }
    if ((*kernelP)->numParm > NRRD_KERNEL_PARMS_NUM) {
      biffAddf(NRRD, "%s: kernel \"%s\" requests %d parameters > max %d",
               me, kstr, (*kernelP)->numParm, NRRD_KERNEL_PARMS_NUM);
      airMopError(mop); return 1;
    }
    if (*kernelP == nrrdKernelGaussian ||
        *kernelP == nrrdKernelGaussianD ||
        *kernelP == nrrdKernelGaussianDD ||
        *kernelP == nrrdKernelDiscreteGaussian ||
        *kernelP == nrrdKernelBoxSupportDebug ||
        *kernelP == nrrdKernelCos4SupportDebug ||
        *kernelP == nrrdKernelCos4SupportDebugD ||
        *kernelP == nrrdKernelCos4SupportDebugDD ||
        *kernelP == nrrdKernelCos4SupportDebugDDD) {
      /* for these kernels, we need all the parameters given explicitly */
      needParm = (*kernelP)->numParm;
    } else {
      /*  For everything else (note that TMF kernels are handled
          separately), we can make do with one less than the required,
          by using the default spacing  */
      needParm = ((*kernelP)->numParm > 0
                  ? (*kernelP)->numParm - 1
                  : 0);
    }
    if (needParm > 0 && !pstr) {
      biffAddf(NRRD, "%s: didn't get any of %d required doubles after "
               "colon in \"%s\"",
               me, needParm, kstr);
      airMopError(mop); return 1;
    }
    for (haveParm=0; haveParm<(*kernelP)->numParm; haveParm++) {
      if (!pstr)
        break;
      if (1 != sscanf(pstr, "%lg", parm+haveParm)) {
        biffAddf(NRRD, "%s: trouble parsing \"%s\" as double (in \"%s\")",
                 me, _pstr, _str);
        airMopError(mop); return 1;
      }
      if ((pstr = strchr(pstr, ','))) {
        pstr++;
        if (!*pstr) {
          biffAddf(NRRD, "%s: nothing after last comma in \"%s\" (in \"%s\")",
                   me, _pstr, _str);
          airMopError(mop); return 1;
        }
      }
    }
    /* haveParm is now the number of parameters that were parsed. */
    if (haveParm < needParm) {
      biffAddf(NRRD, "%s: parsed only %d of %d required doubles "
               "from \"%s\" (in \"%s\")",
               me, haveParm, needParm, _pstr, _str);
      airMopError(mop); return 1;
    } else if (haveParm == needParm &&
               needParm == (*kernelP)->numParm-1) {
      /* shift up parsed values, and set parm[0] to default */
      for (jj=haveParm; jj>=1; jj--) {
        parm[jj] = parm[jj-1];
      }
      parm[0] = nrrdDefaultKernelParm0;
    } else {
      if (pstr) {
        biffAddf(NRRD, "%s: \"%s\" (in \"%s\") has more than %d doubles",
                 me, _pstr, _str, (*kernelP)->numParm);
        airMopError(mop); return 1;
      }
    }
  }
  /*
  fprintf(stderr, "%s: %g %g %g %g %g\n", me,
          parm[0], parm[1], parm[2], parm[3], parm[4]);
  */
  airMopOkay(mop);
  return 0;
}

int
nrrdKernelSpecParse(NrrdKernelSpec *ksp, const char *str) {
  static const char me[]="nrrdKernelSpecParse";
  const NrrdKernel *kern;
  double kparm[NRRD_KERNEL_PARMS_NUM];

  if (!( ksp && str )) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdKernelParse(&kern, kparm, str)) {
    biffAddf(NRRD, "%s: ", me);
    return 1;
  }
  nrrdKernelSpecSet(ksp, kern, kparm);
  return 0;
}

/*
** note that the given string has to be allocated for a certain size
** which is plenty big
*/
int
nrrdKernelSpecSprint(char str[AIR_STRLEN_LARGE], const NrrdKernelSpec *ksp) {
  static const char me[]="nrrdKernelSpecSprint";
  unsigned int warnLen = AIR_STRLEN_LARGE/3;
  char stmp[AIR_STRLEN_LARGE];

  if (!( str && ksp )) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (strlen(ksp->kernel->name) > warnLen) {
    biffAddf(NRRD, "%s: kernel name (len %s) might lead to overflow", me,
             airSprintSize_t(stmp, strlen(ksp->kernel->name)));
    return 1;
  }
  if (strstr(ksp->kernel->name, "TMF")) {
    /* these are handled differently; the identification of the
       kernel is actually packaged as kernel parameters */
    if (!(ksp->kernel->name == strstr(ksp->kernel->name, "TMF"))) {
      biffAddf(NRRD, "%s: TMF kernel name %s didn't start with TMF",
               me, ksp->kernel->name);
      return 1;
    }
    /* 0123456789012 */
    /* TMF_dX_cX_Xef */
    if (!( 13 == strlen(ksp->kernel->name)
           && '_' == ksp->kernel->name[3]
           && '_' == ksp->kernel->name[6]
           && '_' == ksp->kernel->name[9] )) {
      biffAddf(NRRD, "%s: sorry, expected strlen(%s) = 13 with 3 _s",
               me, ksp->kernel->name);
      return 1;
    }
    sprintf(str, "tmf:%c,%c,%c",
            ksp->kernel->name[5],
            ksp->kernel->name[8],
            ksp->kernel->name[10]);
    /* see if the single parm should be added on */
    if (0.0 != ksp->parm[0]) {
      sprintf(stmp, ",%.17g", ksp->parm[0]);
      strcat(str, stmp);
    }
  } else {
    strcpy(str, ksp->kernel->name);
    if (ksp->kernel->numParm) {
      unsigned int pi;
      for (pi=0; pi<ksp->kernel->numParm; pi++) {
        sprintf(stmp, "%c%.17g", (!pi ? ':' : ','), ksp->parm[pi]);
        if (strlen(str) + strlen(stmp) > warnLen) {
          biffAddf(NRRD, "%s: kernel parm %u could overflow", me, pi);
          return 1;
        }
        strcat(str, stmp);
      }
    }
  }
  return 0;
}

int
nrrdKernelSprint(char str[AIR_STRLEN_LARGE], const NrrdKernel *kernel,
                 const double kparm[NRRD_KERNEL_PARMS_NUM]) {
  static const char me[]="nrrdKernelSprint";
  NrrdKernelSpec ksp;

  nrrdKernelSpecSet(&ksp, kernel, kparm);
  if (nrrdKernelSpecSprint(str, &ksp)) {
    biffAddf(NRRD, "%s: trouble", me);
    return 1;
  }
  return 0;
}

int
nrrdKernelCompare(const NrrdKernel *kernA,
                  const double parmA[NRRD_KERNEL_PARMS_NUM],
                  const NrrdKernel *kernB,
                  const double parmB[NRRD_KERNEL_PARMS_NUM],
                  int *differ, char explain[AIR_STRLEN_LARGE]) {
  static const char me[]="nrrdKernelCompare";
  unsigned int pnum, pidx;

  if (!(kernA && kernB && differ)) {
    biffAddf(NRRD, "%s: got NULL pointer (%p, %p, or %p)", me,
             AIR_CVOIDP(kernA), AIR_CVOIDP(kernB), AIR_VOIDP(differ));
    return 1;
  }
  if (kernA != kernB) {
    *differ = kernA < kernB ? -1 : 1;
    if (explain) {
      sprintf(explain, "kernA %s kernB", *differ < 0 ? "<" : ">");
    }
    return 0;
  }
  pnum = kernA->numParm;
  if (!pnum) {
    /* actually, we're done: no parms and kernels are equal */
    *differ = 0;
    return 0;
  }
  if (!(parmA && parmB)) {
    biffAddf(NRRD, "%s: kernel %s needs %u parms but got NULL parm vectors",
             me, kernA->name ? kernA->name : "(unnamed)", pnum);
    return 0;
  }
  for (pidx=0; pidx<pnum; pidx++) {
    if (parmA[pidx] != parmB[pidx]) {
      *differ = parmA[pidx] < parmB[pidx] ? -1 : 1;
      if (explain) {
        sprintf(explain, "parmA[%u]=%f %s parmB[%u]=%f", pidx, parmA[pidx],
                *differ < 0 ? "<" : ">", pidx, parmB[pidx]);
      }
      return 0;
    }
  }

  /* so far nothing unequal */
  *differ = 0;
  return 0;
}

/*
******** nrrdKernelCheck
**
** Makes sure a given kernel is behaving as expected
*/
int
nrrdKernelCheck(const NrrdKernel *kern,
                const double parm[NRRD_KERNEL_PARMS_NUM],
                size_t evalNum, double epsilon,
                unsigned int diffOkEvalMax,
                unsigned int diffOkIntglMax,
                const NrrdKernel *ikern,
                const double iparm[NRRD_KERNEL_PARMS_NUM]) {
  const NrrdKernel *parsedkern;
  double parsedparm[NRRD_KERNEL_PARMS_NUM], supp, integral;
  static const char me[]="nrrdKernelCheck";
  char kstr[AIR_STRLEN_LARGE], explain[AIR_STRLEN_LARGE],
    stmp[AIR_STRLEN_SMALL];
  int differ;
  size_t evalIdx;
  double *dom_d, *ran_d, wee;
  float *dom_f, *ran_f;
  unsigned int diffOkEvalNum, diffOkIntglNum;
  airArray *mop;

  if (!kern) {
    biffAddf(NRRD, "%s: got NULL kernel", me);
    return 1;
  }
  if (!(evalNum > 20)) {
    biffAddf(NRRD, "%s: need evalNum > 20", me);
    return 1;
  }
  if (!(kern->name && kern->support && kern->integral
        && kern->eval1_f && kern->evalN_f
        && kern->eval1_d && kern->evalN_d)) {
    biffAddf(NRRD, "%s: kernel has NULL fields (%d,%d,%d,%d,%d,%d,%d)", me,
             !!(kern->name), !!(kern->support), !!(kern->integral),
             !!(kern->eval1_f), !!(kern->evalN_f),
             !!(kern->eval1_d), !!(kern->evalN_d));
    return 0;
  }
  if (nrrdKernelSprint(kstr, kern, parm)) {
    biffAddf(NRRD, "%s: trouble", me);
    return 1;
  }
  if (nrrdKernelParse(&parsedkern, parsedparm, kstr)) {
    biffAddf(NRRD, "%s: trouble parsing |%s| back to kern/parm pair",
             me, kstr);
    return 1;
  }
  if (nrrdKernelCompare(kern, parm, parsedkern, parsedparm,
                        &differ, explain)) {
    biffAddf(NRRD, "%s: trouble comparing", me);
    return 1;
  }
  if (differ) {
    biffAddf(NRRD, "%s: given and re-parsed kernels differ: %s", me, explain);
    return 1;
  }

  supp = kern->support(parm);
  /* wee is the step between evaluation points */
  wee = 2*supp/AIR_CAST(double, evalNum);
  if ( (kern->eval1_d)(supp+wee/1000, parm) ||
       (kern->eval1_d)(supp+wee, parm) ||
       (kern->eval1_d)(supp+10*wee, parm) ||
       (kern->eval1_d)(-supp-wee/1000, parm) ||
       (kern->eval1_d)(-supp-wee, parm) ||
       (kern->eval1_d)(-supp-10*wee, parm) ) {
    if (nrrdKernelCheap != kern) {
      /* the "cheap" kernel alone gets a pass on reporting its support */
      biffAddf(NRRD, "%s: kern %s is non-zero outside support %g",
               me, kstr, supp);
      return 1;
    }
  }
  mop = airMopNew();
  /* allocate domain and range for both float and double */
  dom_d = AIR_CALLOC(evalNum, double);
  airMopAdd(mop, dom_d, airFree, airMopAlways);
  ran_d = AIR_CALLOC(evalNum, double);
  airMopAdd(mop, ran_d, airFree, airMopAlways);
  dom_f = AIR_CALLOC(evalNum, float);
  airMopAdd(mop, dom_f, airFree, airMopAlways);
  ran_f = AIR_CALLOC(evalNum, float);
  airMopAdd(mop, ran_f, airFree, airMopAlways);
  if (!( dom_d && ran_d && dom_f && ran_f )) {
    biffAddf(NRRD, "%s: couldn't alloc buffers for %s values for %s",
             me, airSprintSize_t(stmp, evalNum), kstr);
    airMopError(mop); return 1;
  }
  for (evalIdx=0; evalIdx<evalNum; evalIdx++) {
    dom_d[evalIdx] = AIR_AFFINE(-0.5, evalIdx,
                                AIR_CAST(double, evalNum)-0.5,
                                -supp, supp);
    dom_f[evalIdx] = AIR_CAST(float, dom_d[evalIdx]);
  }
  /* do the vector evaluations */
  kern->evalN_f(ran_f, dom_f, evalNum, parm);
  kern->evalN_d(ran_d, dom_d, evalNum, parm);
  /*
  for (evalIdx=0; evalIdx<evalNum; evalIdx++) {
    fprintf(stderr, "%u %g --> %g\n", AIR_CAST(unsigned int, evalIdx),
            dom_d[evalIdx], ran_d[evalIdx]);
  }
  */
  /* compare evaluations (and maybe derivatives) and numerically compute
     integral */
  diffOkEvalNum = 0;
  diffOkIntglNum = 0;
  integral = 0.0;
  for (evalIdx=0; evalIdx<evalNum; evalIdx++) {
    double single_f, single_d;
    single_f = kern->eval1_f(dom_f[evalIdx], parm);
    single_d = kern->eval1_d(dom_d[evalIdx], parm);
    integral += single_d;
    /* single float vs vector float */
    if (single_f != ran_f[evalIdx]) {
      biffAddf(NRRD, "%s: %s (eval1_f(%.17g)=%.17g) != (evalN_f(%.17g)=%.17g)",
               me, kstr, dom_f[evalIdx], single_f,
               dom_f[evalIdx], ran_f[evalIdx]);
      airMopError(mop); return 1;
    }
    /* single double vs vector double */
    if (single_d != ran_d[evalIdx]) {
      biffAddf(NRRD, "%s: %s (eval1_d(%.17g)=%.17g) != (evalN_d(%.17g)=%.17g)",
               me, kstr, dom_d[evalIdx], single_d,
               dom_d[evalIdx], ran_d[evalIdx]);
      airMopError(mop); return 1;
    }
    /* single float vs single double */
    if (fabs(single_f - single_d) > epsilon) {
      diffOkEvalNum++;
      if (diffOkEvalNum > diffOkEvalMax) {
        biffAddf(NRRD,
                 "%s: %s |eval1_f(%.17g)=%.17g) - (eval1_d(%.17g)=%.17g)|"
                 " %.17g  >  epsilon %.17g too many times (%u > %u)", me,
                 kstr, dom_f[evalIdx], single_f, dom_d[evalIdx], single_d,
                 fabs(single_f - single_d), epsilon,
                 diffOkEvalNum, diffOkEvalMax);
        airMopError(mop); return 1;
      }
    }
    /* check whether we're the derivative of ikern */
    if (ikern) {
      double forw, back, ndrv;
      forw = ikern->eval1_d(dom_d[evalIdx] + wee/2, iparm);
      back = ikern->eval1_d(dom_d[evalIdx] - wee/2, iparm);
      ndrv = (forw - back)/wee;
      if (fabs(ndrv - single_d) > epsilon) {
        diffOkIntglNum++;
        if (diffOkIntglNum > diffOkIntglMax) {
          biffAddf(NRRD, "%s: %s(%.17g) |num deriv(%s) %.17g - %.17g| "
                   "%.17g > %.17g too many times (%u > %u)",
                   me, kstr, dom_d[evalIdx], ikern->name, ndrv, single_d,
                   fabs(ndrv - single_d), epsilon,
                   diffOkIntglNum, diffOkIntglMax);
          airMopError(mop); return 1;
        }
      }
    }
  }
  integral *= 2*supp/(AIR_CAST(double, evalNum));
  /* the "cheap" kernel alone gets a pass on reporting its integral */
  if (nrrdKernelCheap != kern) {
    double hackeps=10;
    /* hackeps is clearly a hack to permit the integral to have greater
       error than any single evaluation; there must be a more principled
       way to set this */
    if (fabs(integral - kern->integral(parm)) > hackeps*epsilon) {
      biffAddf(NRRD, "%s: %s |numerical integral %.17g - claimed %.17g| "
               "%.17g > %.17g", me, kstr, integral, kern->integral(parm),
               fabs(integral - kern->integral(parm)), hackeps*epsilon);
      airMopError(mop); return 1;
    }
  }

  /* HEY check being derivative of ikern/iparm */
  AIR_UNUSED(ikern);
  AIR_UNUSED(iparm);

  airMopOkay(mop);
  return 0;
}
