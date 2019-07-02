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

/*
** These kernels are the cardinal B-splines of different orders
** Using them with convolution assumes that the data has been pre-filtered
** so that the spline interpolates the original values.
*/

/* helper macros for doing abs() and remembering sign */
#define ABS_SGN(ax, sgn, x)                     \
  if (x < 0) {                                  \
    sgn = -1;                                   \
    ax = -x;                                    \
  } else {                                      \
    sgn = 1;                                    \
    ax = x;                                     \
  }

/* helper macro for listing the various members of the kernel */
#define BSPL_DECL(ord, deriv)                   \
  0,                                            \
    _bspl##ord##_sup,                           \
    (0 == deriv ? returnOne : returnZero),      \
    _bspl##ord##d##deriv##_1f,                  \
    _bspl##ord##d##deriv##_Nf,                  \
    _bspl##ord##d##deriv##_1d,                  \
    _bspl##ord##d##deriv##_Nd

/* (the following preceded some of the function definitions before they
   were all packed up in the _METHODS macros; but the question about the
   type of the locals is still relevant)
   HEY: there is a possibly interesting question to be answered here
   about whether, to distinguish the float-specific and
   double-specific versions of the kernel eval functions, if the
   float-specific versions should actually only use floats for locals,
   so that no casting to float is needed at return, or, if its too
   much of a precision loss to do so, at no real economy of speed, so
   doubles should be used for all intermediate calculations, prior to
   the final cast to float.  The macros below do the casting,
   whether or not is as actually needed, so this can be experimented
   with by just changing the type of the locals (without changing the
   macro definitions) */

#define BSPL_EVEN_METHODS(basename, macro)               \
  static double                                          \
  basename##_1d(double x, const double *parm) {          \
    double ax, tmp, r;                                   \
    AIR_UNUSED(parm);                                    \
                                                         \
    ax = AIR_ABS(x);                                     \
    macro(r, double, tmp, ax);                           \
    return r;                                            \
  }                                                      \
                                                         \
  static float                                           \
  basename##_1f(float x, const double *parm) {           \
    float ax, tmp, r;                                    \
    AIR_UNUSED(parm);                                    \
                                                         \
    ax = AIR_ABS(x);                                     \
    macro(r, float, tmp, ax);                            \
    return r;                                            \
  }                                                      \
                                                         \
  static void                                            \
  basename##_Nd(double *f, const double *x, size_t len,  \
                const double *parm) {                    \
    double ax, tmp, r;                                   \
    size_t i;                                            \
    AIR_UNUSED(parm);                                    \
                                                         \
    for (i=0; i<len; i++) {                              \
      ax = x[i]; ax = AIR_ABS(ax);                       \
      macro(r, double, tmp, ax);                         \
      f[i] = r;                                          \
    }                                                    \
  }                                                      \
                                                         \
  static void                                            \
  basename##_Nf(float *f, const float *x, size_t len,    \
                const double *parm) {                    \
    float ax, tmp, r;                                    \
    size_t i;                                            \
    AIR_UNUSED(parm);                                    \
                                                         \
    for (i=0; i<len; i++) {                              \
      ax = x[i]; ax = AIR_ABS(ax);                       \
      macro(r, float, tmp, ax);                          \
      f[i] = r;                                          \
    }                                                    \
  }

#define BSPL_ODD_METHODS(basename, macro)                \
  static double                                          \
  basename##_1d(double x, const double *parm) {          \
    double ax, tmp, r;                                   \
    int sgn;                                             \
    AIR_UNUSED(parm);                                    \
                                                         \
    ABS_SGN(ax, sgn, x);                                 \
    macro(r, double, tmp, ax);                           \
    return sgn*r;                                        \
  }                                                      \
                                                         \
  static float                                           \
  basename##_1f(float x, const double *parm) {           \
    float sgn, ax, tmp, r;                               \
    AIR_UNUSED(parm);                                    \
                                                         \
    ABS_SGN(ax, sgn, x);                                 \
    macro(r, float, tmp, ax);                            \
    return sgn*r;                                        \
  }                                                      \
                                                         \
  static void                                            \
  basename##_Nd(double *f, const double *x, size_t len,  \
                const double *parm) {                    \
    double sgn, ax, tmp, r;                              \
    size_t i;                                            \
    AIR_UNUSED(parm);                                    \
                                                         \
    for (i=0; i<len; i++) {                              \
      ABS_SGN(ax, sgn, x[i]);                            \
      macro(r, double, tmp, ax);                         \
      f[i] = sgn*r;                                      \
    }                                                    \
  }                                                      \
                                                         \
  static void                                            \
  basename##_Nf(float *f, const float *x, size_t len,    \
                const double *parm) {                    \
    float sgn, ax, tmp, r;                               \
    size_t i;                                            \
    AIR_UNUSED(parm);                                    \
                                                         \
    for (i=0; i<len; i++) {                              \
      ABS_SGN(ax, sgn, x[i]);                            \
      macro(r, float, tmp, ax);                          \
      f[i] = sgn*r;                                      \
    }                                                    \
  }

/* ============================= order *1* ============================= */

static double
_bspl1_sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

/* ---------------------- order *1* deriv *0* -------------------------- */

#define BSPL1D0(ret, TT, t, x)                 \
  AIR_UNUSED(t);                               \
  if (x < 1) {                                 \
    ret = AIR_CAST(TT, 1.0 - x);               \
  } else {                                     \
    ret = 0;                                   \
  }

BSPL_EVEN_METHODS(_bspl1d0, BSPL1D0)

static NrrdKernel
_nrrdKernelBSpline1 = {
  "bspl1",
  BSPL_DECL(1, 0)
};
NrrdKernel *const
nrrdKernelBSpline1 = &_nrrdKernelBSpline1;

/* ---------------------- order *1* deriv *1* -------------------------- */

#define BSPL1D1(ret, TT, t, x)                 \
  AIR_UNUSED(t);                               \
  if (x < 1) {                                 \
    ret = AIR_CAST(TT, -1.0);                  \
  } else {                                     \
    ret = 0;                                   \
  }

BSPL_ODD_METHODS(_bspl1d1, BSPL1D1)

static NrrdKernel
_nrrdKernelBSpline1D = {
  "bspl1d",
  BSPL_DECL(1, 1)
};
NrrdKernel *const
nrrdKernelBSpline1D = &_nrrdKernelBSpline1D;

/* ============================= order *2* ============================= */

static double
_bspl2_sup(const double *parm) {
  AIR_UNUSED(parm);
  return 1.5;
}

/* ---------------------- order *2* deriv *0* -------------------------- */

#define BSPL2D0(ret, TT, t, x)                 \
  if (x < 0.5) {                               \
    ret = AIR_CAST(TT, 3.0/4.0 - x*x);         \
  } else if (x < 1.5) {                        \
    t = (3 - 2*x);                             \
    ret = AIR_CAST(TT, t*t/8);                 \
  } else {                                     \
    ret = 0;                                   \
  }

BSPL_EVEN_METHODS(_bspl2d0, BSPL2D0)

static NrrdKernel
_nrrdKernelBSpline2 = {
  "bspl2",
  BSPL_DECL(2, 0)
};
NrrdKernel *const
nrrdKernelBSpline2 = &_nrrdKernelBSpline2;

/* ---------------------- order *2* deriv *1* -------------------------- */

#define BSPL2D1(ret, TT, t, x)                 \
  AIR_UNUSED(t);                               \
  if (x < 0.5) {                               \
    ret = AIR_CAST(TT, -2*x);                  \
  } else if (x < 1.5) {                        \
    ret = AIR_CAST(TT, -3.0/2.0 + x);          \
  } else {                                     \
    ret = 0;                                   \
  }

BSPL_ODD_METHODS(_bspl2d1, BSPL2D1)

static NrrdKernel
_nrrdKernelBSpline2D = {
  "bspl2d",
  BSPL_DECL(2, 1)
};
NrrdKernel *const
nrrdKernelBSpline2D = &_nrrdKernelBSpline2D;

/* ---------------------- order *2* deriv *2* -------------------------- */

#define BSPL2D2(ret, TT, t, x)                 \
  AIR_UNUSED(t);                               \
  if (x < 0.5) {                               \
    ret = AIR_CAST(TT, -2.0);                  \
  } else if (x < 1.5) {                        \
    ret = AIR_CAST(TT, 1.0);                   \
  } else {                                     \
    ret = 0;                                   \
  }

BSPL_EVEN_METHODS(_bspl2d2, BSPL2D2)

static NrrdKernel
_nrrdKernelBSpline2DD = {
  "bspl2dd",
  BSPL_DECL(2, 2)
};
NrrdKernel *const
nrrdKernelBSpline2DD = &_nrrdKernelBSpline2DD;

/* ============================= order *3* ============================= */

static double
_bspl3_sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2.0;
}

/* ---------------------- order *3* deriv *0* -------------------------- */

#define BSPL3D0(ret, TT, t, x)                  \
  if (x < 1) {                                  \
    ret = AIR_CAST(TT, (4 + 3*(-2 + x)*x*x)/6); \
  } else if (x < 2) {                           \
    t = (-2 + x);                               \
    ret = AIR_CAST(TT, -t*t*t/6);               \
  } else {                                      \
    ret = 0;                                    \
  }

BSPL_EVEN_METHODS(_bspl3d0, BSPL3D0)

static NrrdKernel
_nrrdKernelBSpline3 = {
  "bspl3",
  BSPL_DECL(3, 0)
};
NrrdKernel *const
nrrdKernelBSpline3 = &_nrrdKernelBSpline3;

/* ---------------------- order *3* deriv *1* -------------------------- */

#define BSPL3D1(ret, TT, t, x)                 \
  if (x < 1) {                                 \
    ret = AIR_CAST(TT, (-4 + 3*x)*x/2);        \
  } else if (x < 2) {                          \
    t = (-2 + x);                              \
    ret = AIR_CAST(TT, -t*t/2);                \
  } else {                                     \
    ret = 0;                                   \
  }

BSPL_ODD_METHODS(_bspl3d1, BSPL3D1)

static NrrdKernel
_nrrdKernelBSpline3D = {
  "bspl3d",
  BSPL_DECL(3, 1)
};
NrrdKernel *const
nrrdKernelBSpline3D = &_nrrdKernelBSpline3D;

/* ---------------------- order *3* deriv *2* -------------------------- */

/* NOTE: the tmp variable wasn't actually needed here, and this will
** likely be optimized out.  But the tmp argument to the macro is kept
** here (and the macro uses it to avoid a unused variable warning) to
** facilitate copy-and-paste for higher-order splines
*/
#define BSPL3D2(ret, TT, t, x)                 \
  AIR_UNUSED(t);                               \
  if (x < 1) {                                 \
    ret = AIR_CAST(TT, -2 + 3*x);              \
  } else if (x < 2) {                          \
    ret = AIR_CAST(TT, 2 - x);                 \
  } else {                                     \
    ret = 0;                                   \
  }

BSPL_EVEN_METHODS(_bspl3d2, BSPL3D2)

static NrrdKernel
_nrrdKernelBSpline3DD = {
  "bspl3dd",
  BSPL_DECL(3, 2)
};
NrrdKernel *const
nrrdKernelBSpline3DD = &_nrrdKernelBSpline3DD;

/* ---------------------- order *3* deriv *3* -------------------------- */

#define BSPL3D3(ret, TT, t, x)                  \
  AIR_UNUSED(t);                               \
  if (x < 1) {                                 \
    ret = 3;                                   \
  } else if (x < 2) {                          \
    ret = -1;                                  \
  } else {                                     \
    ret = 0;                                   \
  }

BSPL_ODD_METHODS(_bspl3d3, BSPL3D3)

static NrrdKernel
_nrrdKernelBSpline3DDD = {
  "bspl3ddd",
  BSPL_DECL(3, 3)
};
NrrdKernel *const
nrrdKernelBSpline3DDD = &_nrrdKernelBSpline3DDD;

/* ------------- order *3* approximate numerical inverse -------------- */
/* still need to implement:
**   Unser et al B-Spline Signal Processing: Part I & II, IEEE
**   Transactions on Signal Processing, 1993, 41(2):821-833, 834--848
** but until then here's a slower way of approximating the prefiltering,
** which is still faster than doing iterative deconvolution.  These
** weights were determined by GLK with Mathematica, by inverting the
** matrix representing discrete convolution with the spline
**
** Note that with all the approx inverse kernels, the support really
** does end at a half-integer (they are piece-wise constant on unit
** intervals centered at integers)
*/

#define BSPL3_AI_LEN 12
static double
_bspl3_ANI_kvals[BSPL3_AI_LEN] = {
  2672279.0/1542841.0,
  -(716035.0/1542841.0),
  191861.0/1542841.0,
  -(51409.0/1542841.0),
  13775.0/1542841.0,
  -(3691.0/1542841.0),
  989.0/1542841.0,
  -(265.0/1542841.0),
  71.0/1542841.0,
  -(19.0/1542841.0),
  5.0/1542841.0,
  -(1.0/1542841.0)};

static double
_bspl3_ANI_sup(const double *parm) {
  AIR_UNUSED(parm);
  return BSPL3_AI_LEN + 0.5;
}

static double
_bspl3_ANI_int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

#define BSPL3_ANI(ret, tmp, x)                  \
  tmp = AIR_CAST(unsigned int, x+0.5);          \
  if (tmp < BSPL3_AI_LEN) {                     \
    ret = _bspl3_ANI_kvals[tmp];                \
  } else {                                      \
    ret = 0.0;                                  \
  }

static double
_bspl3_ANI_1d(double x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);

  ax = AIR_ABS(x);
  BSPL3_ANI(r, tmp, ax);
  return r;
}

static float
_bspl3_ANI_1f(float x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);

  ax = AIR_ABS(x);
  BSPL3_ANI(r, tmp, ax);
  return AIR_CAST(float, r);
}

static void
_bspl3_ANI_Nd(double *f, const double *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);

  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    BSPL3_ANI(r, tmp, ax);
    f[i] = r;
  }
}

static void
_bspl3_ANI_Nf(float *f, const float *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);

  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    BSPL3_ANI(r, tmp, ax);
    f[i] = AIR_CAST(float, r);
  }
}

static NrrdKernel
_nrrdKernelBSpline3ApproxInverse = {
  "bspl3ai", 0,
  _bspl3_ANI_sup, _bspl3_ANI_int,
  _bspl3_ANI_1f, _bspl3_ANI_Nf,
  _bspl3_ANI_1d, _bspl3_ANI_Nd
};
NrrdKernel *const
nrrdKernelBSpline3ApproxInverse = &_nrrdKernelBSpline3ApproxInverse;

/* ============================= order *4* ============================= */

static double
_bspl4_sup(const double *parm) {
  AIR_UNUSED(parm);
  return 2.5;
}

/* ---------------------- order *4* deriv *0* -------------------------- */

#define BSPL4D0(ret, TT, t, x)                                          \
  if (x < 0.5) {                                                        \
    t = x*x;                                                            \
    ret = AIR_CAST(TT, 115.0/192.0 - 5*t/8 + t*t/4);                    \
  } else if (x < 1.5) {                                                 \
    ret = AIR_CAST(TT, (55.0 + 4*x*(5.0 - 2*x*(15.0 + 2*(-5 + x)*x)))/96); \
  } else if (x < 2.5) {                                                 \
    t = 5 - 2*x;                                                        \
    ret = AIR_CAST(TT, t*t*t*t/384.0);                                  \
  } else {                                                              \
    ret = 0;                                                            \
  }

BSPL_EVEN_METHODS(_bspl4d0, BSPL4D0)

static NrrdKernel
_nrrdKernelBSpline4 = {
  "bspl4",
  BSPL_DECL(4, 0)
};
NrrdKernel *const
nrrdKernelBSpline4 = &_nrrdKernelBSpline4;

/* ---------------------- order *4* deriv *1* -------------------------- */

#define BSPL4D1(ret, TT, t, x)                                          \
  if (x < 0.5) {                                                        \
    ret = AIR_CAST(TT, x*(-5.0/4.0 + x*x));                             \
  } else if (x < 1.5) {                                                 \
    ret = AIR_CAST(TT, (5.0 - 4*x*(15.0 + x*(-15.0 + 4*x)))/24.0);      \
  } else if (x < 2.5) {                                                 \
    t = -5 + 2*x;                                                        \
    ret = AIR_CAST(TT, t*t*t/48.0);                                     \
  } else {                                                              \
    ret = 0;                                                            \
  }

BSPL_ODD_METHODS(_bspl4d1, BSPL4D1)

static NrrdKernel
_nrrdKernelBSpline4D = {
  "bspl4d",
  BSPL_DECL(4, 1)
};
NrrdKernel *const
nrrdKernelBSpline4D = &_nrrdKernelBSpline4D;

/* ---------------------- order *4* deriv *2* -------------------------- */

#define BSPL4D2(ret, TT, t, x)                                          \
  if (x < 0.5) {                                                        \
    ret = AIR_CAST(TT, -5.0/4.0 + 3*x*x);                               \
  } else if (x < 1.5) {                                                 \
    ret = AIR_CAST(TT, -5.0/2.0 + (5.0 - 2*x)*x);                       \
  } else if (x < 2.5) {                                                 \
    t = 5 - 2*x;                                                        \
    ret = AIR_CAST(TT, t*t/8.0);                                        \
  } else {                                                              \
    ret = 0;                                                            \
  }

BSPL_EVEN_METHODS(_bspl4d2, BSPL4D2)

static NrrdKernel
_nrrdKernelBSpline4DD = {
  "bspl4dd",
  BSPL_DECL(4, 2)
};
NrrdKernel *const
nrrdKernelBSpline4DD = &_nrrdKernelBSpline4DD;

/* ---------------------- order *4* deriv *3* -------------------------- */

#define BSPL4D3(ret, TT, t, x)                                          \
  AIR_UNUSED(t);                                                        \
  if (x < 0.5) {                                                        \
    ret = AIR_CAST(TT, 6*x);                                            \
  } else if (x < 1.5) {                                                 \
    ret = AIR_CAST(TT, 5 - 4*x);                                        \
  } else if (x < 2.5) {                                                 \
    ret = AIR_CAST(TT, -5.0/2.0 + x);                                   \
  } else {                                                              \
    ret = 0;                                                            \
  }

BSPL_ODD_METHODS(_bspl4d3, BSPL4D3)

static NrrdKernel
_nrrdKernelBSpline4DDD = {
  "bspl4ddd",
  BSPL_DECL(4, 3)
};
NrrdKernel *const
nrrdKernelBSpline4DDD = &_nrrdKernelBSpline4DDD;

/* ============================= order *5* ============================= */

static double
_bspl5_sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3.0;
}

/* ---------------------- order *5* deriv *0* -------------------------- */

#define BSPL5D0(ret, TT, t, x)                                  \
  if (x < 1) {                                                  \
    t = x*x;                                                    \
    ret = AIR_CAST(TT, (33 - 5*t*(6 + (x-3)*t))/60);            \
  } else if (x < 2) {                                           \
    ret = AIR_CAST(TT, (51 + 5*x*(15 + x*(-42 + x*(30 + (-9 + x)*x))))/120); \
  } else if (x < 3) {                                                   \
    t = x - 3;                                                  \
    ret = AIR_CAST(TT, -t*t*t*t*t/120);                         \
  } else {                                                      \
    ret = 0;                                                    \
  }

BSPL_EVEN_METHODS(_bspl5d0, BSPL5D0)

static NrrdKernel
_nrrdKernelBSpline5 = {
  "bspl5",
  BSPL_DECL(5, 0)
};
NrrdKernel *const
nrrdKernelBSpline5 = &_nrrdKernelBSpline5;

/* ---------------------- order *5* deriv *1* -------------------------- */

#define BSPL5D1(ret, TT, t, x)                          \
  if (x < 1) {                                          \
    t = x*x*x;                                          \
    ret = AIR_CAST(TT, -x + t - (5*t*x)/12);             \
  } else if (x < 2) {                                   \
    ret = AIR_CAST(TT, (15 + x*(-84 + x*(90 + x*(-36 + 5*x))))/24);     \
  } else if (x < 3) {                                   \
    t = -3 + x;                                         \
    ret = AIR_CAST(TT, -t*t*t*t/24);                    \
  } else {                                              \
    ret = 0;                                            \
  }

BSPL_ODD_METHODS(_bspl5d1, BSPL5D1)

static NrrdKernel
_nrrdKernelBSpline5D = {
  "bspl5d",
  BSPL_DECL(5, 1)
};
NrrdKernel *const
nrrdKernelBSpline5D = &_nrrdKernelBSpline5D;

/* ---------------------- order *5* deriv *2* -------------------------- */

#define BSPL5D2(ret, TT, t, x)                  \
  if (x < 1) {                                  \
    t = x*x;                                    \
    ret = AIR_CAST(TT, -1 + 3*t - (5*t*x)/3);   \
  } else if (x < 2) {                           \
    ret = AIR_CAST(TT, (-21 + x*(45 + x*(-27 + 5*x)))/6);       \
  } else if (x < 3) {                                           \
    t = -3 + x;                                 \
    ret = AIR_CAST(TT, -t*t*t/6);               \
  } else {                                      \
    ret = 0;                                    \
  }

BSPL_EVEN_METHODS(_bspl5d2, BSPL5D2)

static NrrdKernel
_nrrdKernelBSpline5DD = {
  "bspl5dd",
  BSPL_DECL(5, 2)
};
NrrdKernel *const
nrrdKernelBSpline5DD = &_nrrdKernelBSpline5DD;

/* ---------------------- order *5* deriv *3* -------------------------- */

#define BSPL5D3(ret, TT, t, x)                          \
  if (x < 1) {                                          \
    ret = AIR_CAST(TT, (6 - 5*x)*x);                    \
  } else if (x < 2) {                                   \
    ret = AIR_CAST(TT, 15.0/2.0 - 9*x + 5*x*x/2);       \
  } else if (x < 3) {                                   \
    t = -3 + x;                                         \
    ret = AIR_CAST(TT, -t*t/2);                         \
  } else {                                              \
    ret = 0;                                            \
  }

BSPL_ODD_METHODS(_bspl5d3, BSPL5D3)

static NrrdKernel
_nrrdKernelBSpline5DDD = {
  "bspl5ddd",
  BSPL_DECL(5, 3)
};
NrrdKernel *const
nrrdKernelBSpline5DDD = &_nrrdKernelBSpline5DDD;

/* ------------- order *5* approximate numerical inverse -------------- */

#define BSPL5_AI_LEN 19
static double
_bspl5_ANI_kvals[BSPL5_AI_LEN] = {
  2.842170922021427870236333,
  -1.321729472987239796417307,
  0.5733258709611149890510146,
  -0.2470419274010479815114381,
  0.1063780046404650785440854,
  -0.04580408418467518130037713,
  0.01972212399699206014654736,
  -0.008491860984275658620122180,
  0.003656385950780789716770681,
  -0.001574349495225446217828165,
  0.0006778757185045443332966769,
  -0.0002918757322635763049702028,
  0.0001256725426338698784062181,
  -0.00005410696497728715841372199,
  0.00002328659592249373987497103,
  -0.00001000218170092531503506361,
  4.249940115067599514119408e-6,
  -1.698979738236873388431330e-6,
  4.475539012615912040164139e-7};

static double
_bspl5_ANI_sup(const double *parm) {
  AIR_UNUSED(parm);
  return BSPL5_AI_LEN + 0.5;
}

static double
_bspl5_ANI_int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

#define BSPL5_ANI_T(ret, TT, tmp, x)            \
  tmp = AIR_CAST(unsigned int, x+0.5);          \
  if (tmp < BSPL5_AI_LEN) {                     \
    ret = AIR_CAST(TT, _bspl5_ANI_kvals[tmp]);  \
  } else {                                      \
    ret = 0.0;                                  \
  }

static double
_bspl5_ANI_1d(double x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);

  ax = AIR_ABS(x);
  BSPL5_ANI_T(r, double, tmp, ax);
  return r;
}

static float
_bspl5_ANI_1f(float x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);

  ax = AIR_ABS(x);
  BSPL5_ANI_T(r, float, tmp, ax);
  return AIR_CAST(float, r);
}

static void
_bspl5_ANI_Nd(double *f, const double *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);

  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    BSPL5_ANI_T(r, double, tmp, ax);
    f[i] = r;
  }
}

static void
_bspl5_ANI_Nf(float *f, const float *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);

  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    BSPL5_ANI_T(r, float, tmp, ax);
    f[i] = AIR_CAST(float, r);
  }
}

static NrrdKernel
_nrrdKernelBSpline5ApproxInverse = {
  "bspl5ai", 0,
  _bspl5_ANI_sup, _bspl5_ANI_int,
  _bspl5_ANI_1f, _bspl5_ANI_Nf,
  _bspl5_ANI_1d, _bspl5_ANI_Nd
};
NrrdKernel *const
nrrdKernelBSpline5ApproxInverse = &_nrrdKernelBSpline5ApproxInverse;

/* ============================= order *6* ============================= */

static double
_bspl6_sup(const double *parm) {
  AIR_UNUSED(parm);
  return 3.5;
}

/* ---------------------- order *6* deriv *0* -------------------------- */

#define BSPL6D0_SEG0(x) 0.5110243055555555556 + x*x*(-0.40104166666666666667 + x*x*(0.14583333333333333333 - 0.027777777777777777778*x*x))
#define BSPL6D0_SEG1(x) 0.02083333333333333*(5.05890179802561 + (-4.47046426301056 + x)*x)*(5.07700929828288 + (-4.13708416717549 + x)*x)*(0.956452947962608 + x*(1.607548430186042 + x))
#define BSPL6D0_SEG2(x) -0.008333333333333*(-2.919623692889 + x)*(0.11036932382080 + x)*(8.451507829592 + (-5.787493668289 + x)*x)*(7.911791484411 + (-5.403251962643 + x)*x)
#define BSPL6D0(ret, TT, t, x)                                          \
  if (x < 0.5) {                                                        \
    ret = AIR_CAST(TT, BSPL6D0_SEG0(x));                                \
  } else if (x < 1.5) {                                                 \
    ret = AIR_CAST(TT, BSPL6D0_SEG1(x));                                \
  } else if (x < 2.5) {                                                 \
    ret = AIR_CAST(TT, BSPL6D0_SEG2(x));                                \
  } else if (x < 3.5) {                                                 \
    t = AIR_CAST(TT, x - 3.5);                                          \
    ret = AIR_CAST(TT, 0.00139*t*t*t*t*t*t );                           \
  } else {                                                              \
    ret = 0;                                                            \
  }

BSPL_EVEN_METHODS(_bspl6d0, BSPL6D0)

static NrrdKernel
_nrrdKernelBSpline6 = {
  "bspl6",
  BSPL_DECL(6, 0)
};
NrrdKernel *const
nrrdKernelBSpline6 = &_nrrdKernelBSpline6;

/* ---------------------- order *6* deriv *1* -------------------------- */

#define BSPL6D1_SEG0(x) x*(-0.8020833333333333333 + x*x*(0.5833333333333333333 - x*x*0.16666666666666666667))
#define BSPL6D1_SEG1(x) 0.1250000000000000*(-2.204221529535419 + x)*(0.01290998431413690 + x)*(0.5355244627388528 + x)*(4.784830284687429 + (-4.177546250850904 + x)*x)
#define BSPL6D1_SEG2(x) -0.050000000000000*(-0.39815802840054 + x)*(8.4005837632394 + (-5.7883654809137 + x)*x)*(7.8916975718499 + (-5.4801431573524 + x)*x)
#define BSPL6D1(ret, TT, t, x)                                          \
  if (x < 0.5) {                                                        \
    ret = AIR_CAST(TT, BSPL6D1_SEG0(x));                                \
  } else if (x < 1.5) {                                                 \
    ret = AIR_CAST(TT, BSPL6D1_SEG1(x));                                \
  } else if (x < 2.5) {                                                 \
    ret = AIR_CAST(TT, BSPL6D1_SEG2(x));                                \
  } else if (x < 3.5) {                                                 \
    t = AIR_CAST(TT, -3.5 + x);                                         \
    ret = AIR_CAST(TT, 0.00833*t*t*t*t*t);                              \
  } else {                                                              \
    ret = 0.0;                                                          \
  }

BSPL_ODD_METHODS(_bspl6d1, BSPL6D1)

static NrrdKernel
_nrrdKernelBSpline6D = {
  "bspl6d",
  BSPL_DECL(6, 1)
};
NrrdKernel *const
nrrdKernelBSpline6D = &_nrrdKernelBSpline6D;

/* ---------------------- order *6* deriv *2* -------------------------- */

#define BSPL6D2_SEG0(x) -0.80208333333333333333 + x*x*(1.75 - x*x*0.833333333333333333)
#define BSPL6D2_SEG1(x) 0.625*(-0.8093237825464294 + x)*(0.3133677888004832 + x)*(4.485127047744998 + (-4.170710672920720 + x)*x)
#define BSPL6D2_SEG2(x) -0.25*(-2.88072372021534 + x)*(-0.904025842763129 + x)*(7.89575131106459 + (-5.54858377035486 + x)*x)
#define BSPL6D2(ret, TT, t, x)                                          \
  if (x < 0.5) {                                                        \
    ret = AIR_CAST(TT, BSPL6D2_SEG0(x));                                \
  } else if (x < 1.5) {                                                 \
    ret = AIR_CAST(TT, BSPL6D2_SEG1(x));                                \
  } else if (x < 2.5) {                                                 \
    ret = AIR_CAST(TT, BSPL6D2_SEG2(x));                                \
  } else if (x < 3.5) {                                                 \
    t = AIR_CAST(TT, 7.0 - 2.0*x);                                      \
    ret = AIR_CAST(TT, 0.0026041666666666666667*t*t*t*t);               \
  } else {                                                              \
    ret = 0;                                                            \
  }

BSPL_EVEN_METHODS(_bspl6d2, BSPL6D2)

static NrrdKernel
_nrrdKernelBSpline6DD = {
  "bspl6dd",
  BSPL_DECL(6, 2)
};
NrrdKernel *const
nrrdKernelBSpline6DD = &_nrrdKernelBSpline6DD;

/* ---------------------- order *6* deriv *3* -------------------------- */

#define BSPL6D3(ret, TT, t, x)                                          \
  if (x < 0.5) {                                                        \
    ret = AIR_CAST(TT, x*(3.5 - 3.3333333333333333333*x*x));            \
  } else if (x < 1.5) {                                                 \
    ret = AIR_CAST(TT, 2.5*(-1.99263608511781188 + x)*(-1.40303873392913616 + x)*(-0.104325180953051958 + x)); \
  } else if (x < 2.5) {                                                 \
    ret = AIR_CAST(TT, -1*(-1.404627184534107 + x)*(7.890587235793465 + (-5.595372815465893 + x)*x)); \
  } else if (x < 3.5) {                                                 \
    t = AIR_CAST(TT, -7 + 2*x);                                         \
    ret = AIR_CAST(TT, 0.020833333333333333333*t*t*t);                  \
  } else {                                                              \
    ret = 0.0;                                                          \
  }

BSPL_ODD_METHODS(_bspl6d3, BSPL6D3)

static NrrdKernel
_nrrdKernelBSpline6DDD = {
  "bspl6ddd",
  BSPL_DECL(6, 3)
};
NrrdKernel *const
nrrdKernelBSpline6DDD = &_nrrdKernelBSpline6DDD;

/* ============================= order *7* ============================= */

static double
_bspl7_sup(const double *parm) {
  AIR_UNUSED(parm);
  return 4.0;
}

/* ---------------------- order *7* deriv *0* -------------------------- */

#define BSPL7D0(ret, TT, t, x)                                          \
  if (x < 1) {                                                          \
    ret = AIR_CAST(TT, 151.0/315.0 + x*x*(-48.0 + x*x*(16.0 + x*x*(-4 + x)))/144.0); \
  } else if (x < 2) {                                                   \
    ret = AIR_CAST(TT, (2472 - 7*x*(56 + x*(72 + x*(280 + 3*(-6 + x)*x*(20 + (-6 + x)*x)))))/5040.0); \
  } else if (x < 3) {                                                   \
    ret = AIR_CAST(TT, (-1112 + 7*x*(1736 + x*(-2760 + x*(1960 + x*(-760 + x*(168 + (-20 + x)*x))))))/5040.0); \
  } else if (x < 4) {                                                   \
    t = x - 4;                                                          \
    ret = AIR_CAST(TT, -t*t*t*t*t*t*t/5040);                            \
  } else {                                                              \
    ret = 0;                                                            \
  }

BSPL_EVEN_METHODS(_bspl7d0, BSPL7D0)

static NrrdKernel
_nrrdKernelBSpline7 = {
  "bspl7",
  BSPL_DECL(7, 0)
};
NrrdKernel *const
nrrdKernelBSpline7 = &_nrrdKernelBSpline7;

/* ---------------------- order *7* deriv *1* -------------------------- */

#define BSPL7D1(ret, TT, t, x)                                          \
  if (x < 1) {                                                          \
    ret = AIR_CAST(TT, x*(-96.0 + x*x*(64.0 + x*x*(-24.0 + 7.0*x)))/144.0); \
  } else if (x < 2) {                                                   \
    ret = AIR_CAST(TT, -7.0/90.0 - (-2 + x)*x*(-24 + (-2 + x)*x*(76 + x*(-44 + 7*x)))/240.0); \
  } else if (x < 3) {                                                   \
    ret = AIR_CAST(TT, (2 + (-4 + x)*x)*(868 + x*(-1024 + x*(458 + x*(-92 + 7*x))))/720.0); \
  } else if (x < 4) {                                                   \
    t = -4 + x;                                                         \
    ret = AIR_CAST(TT, -t*t*t*t*t*t/720);                               \
  } else {                                                              \
    ret = 0.0;                                                          \
  }

BSPL_ODD_METHODS(_bspl7d1, BSPL7D1)

static NrrdKernel
_nrrdKernelBSpline7D = {
  "bspl7d",
  BSPL_DECL(7, 1)
};
NrrdKernel *const
nrrdKernelBSpline7D = &_nrrdKernelBSpline7D;

/* ---------------------- order *7* deriv *2* -------------------------- */

#define BSPL7D2(ret, TT, t, x)                                          \
  if (x < 1) {                                                          \
    ret = AIR_CAST(TT, (-16.0 + x*x*(32 + x*x*(-20 + 7*x)))/24.0);      \
  } else if (x < 2) {                                                   \
    ret = AIR_CAST(TT, -1.0/5.0 - 7*x/3 + 6*x*x - 14*x*x*x/3 + 3*x*x*x*x/2 - 7*x*x*x*x*x/40); \
  } else if (x < 3) {                                                   \
    ret = AIR_CAST(TT, (-920 + x*(1960 + x*(-1520 + x*(560 + x*(-100 + 7*x)))))/120.0); \
  } else if (x < 4) {                                                   \
    t = -4 + x;                                                         \
    ret = AIR_CAST(TT, -t*t*t*t*t/120);                                 \
  } else {                                                              \
    ret = 0;                                                            \
  }

BSPL_EVEN_METHODS(_bspl7d2, BSPL7D2)

static NrrdKernel
_nrrdKernelBSpline7DD = {
  "bspl7dd",
  BSPL_DECL(7, 2)
};
NrrdKernel *const
nrrdKernelBSpline7DD = &_nrrdKernelBSpline7DD;

/* ---------------------- order *7* deriv *3* -------------------------- */

#define BSPL7D3(ret, TT, t, x)                                          \
  if (x < 1) {                                                          \
    ret = AIR_CAST(TT, x*(64 + 5*x*x*(-16 + 7*x))/24);                  \
  } else if (x < 2) {                                                   \
    ret = AIR_CAST(TT, -7.0/3.0 + x*(12 + x*(-14 + x*(6 - 7*x/8))));    \
  } else if (x < 3) {                                                   \
    ret = AIR_CAST(TT, (392 + x*(-608 + x*(336 + x*(-80 + 7*x))))/24);  \
  } else if (x < 4) {                                                   \
    t = -4 + x;                                                         \
    ret = AIR_CAST(TT, -t*t*t*t/24);                                    \
  } else {                                                              \
    ret = 0.0;                                                          \
  }

BSPL_ODD_METHODS(_bspl7d3, BSPL7D3)

static NrrdKernel
_nrrdKernelBSpline7DDD = {
  "bspl7ddd",
  BSPL_DECL(7, 3)
};
NrrdKernel *const
nrrdKernelBSpline7DDD = &_nrrdKernelBSpline7DDD;

/* ------------- order *7* approximate numerical inverse -------------- */

#define BSPL7_AI_LEN 26
static double
_bspl7_ANI_kvals[BSPL7_AI_LEN] = {
  4.964732886301469059137801,
  -3.091042499769118182213297,
  1.707958936669135515487259,
  -0.9207818274511302808978934,
  0.4936786139601599067344824,
  -0.2643548049418435742509870,
  0.1415160014538524997926456,
  -0.07575222270391683956827192,
  0.04054886334181815702759984,
  -0.02170503519322401084648773,
  0.01161828326728242899507066,
  -0.006219039932262414977444894,
  0.003328930278070297807163008,
  -0.001781910982713036390230280,
  0.0009538216015244754251250379,
  -0.0005105611456814427816916412,
  0.0002732917233015012426069489,
  -0.0001462845976614043380333786,
  0.00007829746549013888268504229,
  -0.00004190023413676309286922788,
  0.00002240807576972098806040711,
  -0.00001195669542335526044896263,
  6.329480796176889498331054e-6,
  -3.256910241436675950084186e-6,
  1.506132735770447868981087e-6,
  -4.260433183779953604188120e-7};

static double
_bspl7_ANI_sup(const double *parm) {
  AIR_UNUSED(parm);
  return BSPL7_AI_LEN + 0.5;
}

static double
_bspl7_ANI_int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

#define BSPL7_ANI(ret, tmp, x)                  \
  tmp = AIR_CAST(unsigned int, x+0.5);          \
  if (tmp < BSPL7_AI_LEN) {                     \
    ret = _bspl7_ANI_kvals[tmp];                \
  } else {                                      \
    ret = 0.0;                                  \
  }

static double
_bspl7_ANI_1d(double x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);

  ax = AIR_ABS(x);
  BSPL7_ANI(r, tmp, ax);
  return r;
}

static float
_bspl7_ANI_1f(float x, const double *parm) {
  double ax, r; unsigned int tmp;
  AIR_UNUSED(parm);

  ax = AIR_ABS(x);
  BSPL7_ANI(r, tmp, ax);
  return AIR_CAST(float, r);
}

static void
_bspl7_ANI_Nd(double *f, const double *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);

  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    BSPL7_ANI(r, tmp, ax);
    f[i] = r;
  }
}

static void
_bspl7_ANI_Nf(float *f, const float *x, size_t len, const double *parm) {
  double ax, r; unsigned int tmp;
  size_t i;
  AIR_UNUSED(parm);

  for (i=0; i<len; i++) {
    ax = x[i]; ax = AIR_ABS(ax);
    BSPL7_ANI(r, tmp, ax);
    f[i] = AIR_CAST(float, r);
  }
}

static NrrdKernel
_nrrdKernelBSpline7ApproxInverse = {
  "bspl7ai", 0,
  _bspl7_ANI_sup, _bspl7_ANI_int,
  _bspl7_ANI_1f, _bspl7_ANI_Nf,
  _bspl7_ANI_1d, _bspl7_ANI_Nd
};
NrrdKernel *const
nrrdKernelBSpline7ApproxInverse = &_nrrdKernelBSpline7ApproxInverse;

