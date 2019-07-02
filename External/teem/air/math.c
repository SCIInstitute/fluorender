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


#include "air.h"

/*
** by the way, the organization of functions into files is a little
** arbitrary around here
*/

/*
** from: Gavin C Cawley, "On a Fast, Compact Approximation of the
** Exponential Function" Neural Computation, 2000, 12(9), 2009-2012.
**
** which in turn is based on: N N Schraudolph, "A fast, compact approximation
** of the exponential function." Neural Computation, 1999, 11(4), 853-862.
** http://cnl.salk.edu/~schraudo/pubs/Schraudolph99.pdf
**
** some possible code which does not depend on endian
** http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html
*/

typedef union {
  double dd;
  int nn[2];
} eco_t;

#define EXPA (1048576/0.69314718055994530942)
#define EXPC 60801
double
airFastExp(double val) {
  eco_t eco;
  double ret;
  int tmpI, EXPI;

  /* HEY: COPY AND PASTE from airMyEndian */
  tmpI = 1;
  EXPI = *(AIR_CAST(char*, &tmpI));
  eco.nn[EXPI] = AIR_CAST(int, (EXPA*(val)) + (1072693248 - EXPC));
  eco.nn[1-EXPI] = 0;
  ret = (eco.dd > 0.0
         ? eco.dd
         /* seems that only times this happens is when the real exp()
            returns either 0 or +inf */
         : (val < 0 ? 0 : AIR_POS_INF));
  return ret;
}
#undef EXPA
#undef EXPC

/*
** based on MiniMaxApproximation, but this has failed in its goal
** of being faster than libc's exp(), so should be considered
** a work-in-progress
*/
double
airExp(double x) {
  double num, den, ee;
  unsigned int pp;

  if (AIR_IN_CL(-1, x, 1)) {
    num = 1 + x*(0.500241 + x*(0.107193 + (0.0118938 + 0.000591457*x)*x));
    den = 1 + x*(-0.499759 + x*(0.106952 + (-0.0118456 + 0.000587495*x)*x));
    ee = num/den;
  } else if (x > 1) {
    pp = 0;
    while (x > 2) {
      x /= 2;
      pp += 1;
    }
    num = 1 + x*(0.552853 + x*(0.135772 + (0.0183685 + 0.00130944*x)*x));
    den = 1 + x*(-0.44714 + x*(0.0828937 + (-0.00759541 + 0.000291662*x)*x));
    ee = num/den;
    while (pp) {
      ee *= ee;
      pp -= 1;
    }
  } else if (x < -1) {
    pp = 0;
    while (x < -2) {
      x /= 2;
      pp += 1;
    }
    num = 0.999999+x*(0.44726+x*(0.0829439 + (0.00760326 + 0.000292122*x)*x));
    den = 1 + x*(-0.552732 + x*(0.135702 + (-0.0183511 + 0.00130689*x)*x));
    ee = num/den;
    while (pp) {
      ee *= ee;
      pp -= 1;
    }
  } else {
    /* x is probably not finite */
    ee = exp(x);
  }
  return ee;
}

/*
******** airNormalRand
**
** generates two numbers with normal distribution (mean 0, stdv 1)
** using the Box-Muller transform.
**
** on (seemingly sound) advice of
** <http://www.taygeta.com/random/gaussian.html>,
** I'm using the polar form of the Box-Muller transform, instead of the
** Cartesian one described at
** <http://mathworld.wolfram.com/Box-MullerTransformation.html>
**
** this is careful to not write into given NULL pointers
*/
void
airNormalRand(double *z1, double *z2) {
  double w, x1, x2;

  do {
    x1 = 2*airDrandMT() - 1;
    x2 = 2*airDrandMT() - 1;
    w = x1*x1 + x2*x2;
  } while ( w >= 1 );
  w = sqrt((-2*log(w))/w);
  if (z1) {
    *z1 = x1*w;
  }
  if (z2) {
    *z2 = x2*w;
  }
  return;
}

void
airNormalRand_r(double *z1, double *z2, airRandMTState *state) {
  double w, x1, x2;

  do {
    x1 = 2*airDrandMT_r(state) - 1;
    x2 = 2*airDrandMT_r(state) - 1;
    w = x1*x1 + x2*x2;
  } while ( w >= 1 );
  w = sqrt((-2*log(w))/w);
  if (z1) {
    *z1 = x1*w;
  }
  if (z2) {
    *z2 = x2*w;
  }
  return;
}

/*
******** airShuffle()
**
** generates a random permutation of integers [0..N-1] if perm is non-zero,
** otherwise, just fills buff with [0..N-1] in order
**
** see http://en.wikipedia.org/wiki/Knuth_shuffle
*/
void
airShuffle(unsigned int *buff, unsigned int N, int perm) {
  unsigned int i;

  if (!(buff && N > 0)) {
    return;
  }

  for (i=0; i<N; i++) {
    buff[i] = i;
  }
  if (perm) {
    unsigned int swp, tmp;
    while (N > 1) {
      swp = airRandInt(N);
      N--;
      tmp = buff[N];
      buff[N] = buff[swp];
      buff[swp] = tmp;
    }
  }
}

void
airShuffle_r(airRandMTState *state,
             unsigned int *buff, unsigned int N, int perm) {
  unsigned int i;

  /* HEY !!! COPY AND PASTE !!!! */
  if (!(buff && N > 0)) {
    return;
  }

  for (i=0; i<N; i++) {
    buff[i] = i;
  }
  if (perm) {
    unsigned int swp, tmp;
    while (N > 1) {
      swp = airRandInt_r(state, N);
      N--;
      tmp = buff[N];
      buff[N] = buff[swp];
      buff[swp] = tmp;
    }
  }
  /* HEY !!! COPY AND PASTE !!!! */
}

double
airSgnPow(double v, double p) {

  return (p == 1
          ? v
          : (v >= 0
             ? pow(v, p)
             : -pow(-v, p)));
}

double
airFlippedSgnPow(double vv, double pp) {
  double sn=1.0;

  if (1.0 == pp) {
    return vv;
  }
  if (vv < 0) {
    sn = -1.0;
    vv = -vv;
  }
  return sn*(1.0 - pow(1.0 - AIR_MIN(1.0, vv), pp));
}

double
airIntPow(double v, int p) {
  double sq, ret;

  if (p > 0) {
    sq = v;
    while (!(p & 1)) {
      /* while the low bit is zero */
      p >>= 1;
      sq *= sq;
    }
    /* must terminate because we know p != 0, and when
       it terminates we know that the low bit is 1 */
    ret = sq;
    while (p >>= 1) {
      /* while there are any non-zero bits in p left */
      sq *= sq;
      if (p & 1) {
        ret *= sq;
      }
    }
  } else if (p < 0) {
    ret = airIntPow(1.0/v, -p);
  } else {
    ret = 1.0;
  }

  return ret;
}

/*
******** airLog2()
**
** silly little function which returns log_2(n) if n is exactly a power of 2,
** and -1 otherwise
*/
int
airLog2(size_t _nn) {
  size_t nn;
  int ret;

  nn = _nn;
  if (0 == nn) {
    /* 0 is not a power of 2 */
    ret = -1;
  } else {
    int alog = 0;
    /* divide by 2 while its non-zero and the low bit is off */
    while (!!nn && !(nn & 1)) {
      alog += 1;
      nn /= 2;
    }
    if (1 == nn) {
      ret = alog;
    } else {
      ret = -1;
    }
  }
  return ret;
}

int
airSgn(double v) {
  return (v > 0
          ? 1
          : (v < 0
             ? -1
             : 0));
}

/*
******** airCbrt
**
** cbrt() isn't in C89, so any hacks to create a stand-in for cbrt()
** are done here.
*/
double
airCbrt(double v) {
#if defined(_WIN32) || defined(__STRICT_ANSI__)
  return (v < 0.0 ? -pow(-v,1.0/3.0) : pow(v,1.0/3.0));
#else
  return cbrt(v);
#endif
}

/*
** skewness of three numbers, scaled to fit in [-1,+1]
** -1: small, big, big
** +1: small, small, big
*/
double
airMode3_d(const double _v[3]) {
  double num, den, mean, v[3];

  mean = (_v[0] + _v[1] + _v[2])/3;
  v[0] = _v[0] - mean;
  v[1] = _v[1] - mean;
  v[2] = _v[2] - mean;
  num = (v[0] + v[1] - 2*v[2])*(2*v[0] - v[1] - v[2])*(v[0] - 2*v[1] + v[2]);
  den = v[0]*v[0] + v[1]*v[1] + v[2]*v[2] - v[1]*v[2] - v[0]*v[1] - v[0]*v[2];
  den = sqrt(den);
  return (den ? num/(2*den*den*den) : 0);
}

double
airMode3(double v0, double v1, double v2) {
  double v[3];

  v[0] = v0;
  v[1] = v1;
  v[2] = v2;
  return airMode3_d(v);
}

double
airGaussian(double x, double mean, double stdv) {

  x = x - mean;
  return exp(-(x*x)/(2*stdv*stdv))/(stdv*sqrt(2*AIR_PI));
}

/*
** The function approximations below were done by GLK in Mathematica,
** using its MiniMaxApproximation[] function.  The functional forms
** used for the Bessel functions were copied from Numerical Recipes
** (which were in turn copied from the "Handbook of Mathematical
** Functions with Formulas, Graphs, and Mathematical Tables" by
** Abramowitz and Stegun), but the coefficients here represent
** an increase in accuracy.
**
** The rational functions (crudely copy/paste from Mathematica into
** this file) upon which the approximations are based have a relative
** error of less than 10^-9, at least on the intervals on which they
** were created (in the case of the second branch of the
** approximation, the lower end of the interval was chosen as close to
** zero as possible). The relative error of the total approximation
** may be greater.
*/

double
airErfc(double x) {
  double ax, y, b;

  ax = AIR_ABS(x);
  if (ax < 0.9820789566638689) {
    b = (0.9999999999995380997 + ax*(-1.0198241793287349401 +
        ax*(0.37030717279808919457 + ax*(-0.15987839763633308864 +
        ax*(0.12416682580357861661 + (-0.04829622197742573233 +
        0.0066094852952188890901*ax)*ax)))))/
        (1 + ax*(0.1085549876246959456 + ax*(0.49279836663925410323 +
        ax*(0.020058474597886988472 + ax*(0.10597158000597863862 +
        (-0.0012466514192679810876 + 0.0099475501252703646772*ax)*ax)))));
  } else if (ax < 2.020104167011169) {
    y = ax - 1;
    b = (0.15729920705029613528 + y*(-0.37677358667097191131 +
        y*(0.3881956571123873123 + y*(-0.22055886537359936478 +
        y*(0.073002666454740425451 + (-0.013369369366972563804 +
        0.0010602024397541548717*y)*y)))))/
        (1 + y*(0.243700597525225235 + y*(0.47203101881562848941 +
        y*(0.080051054975943863026 + y*(0.083879049958465759886 +
        (0.0076905426306038205308 + 0.0058528196473365970129*y)*y)))));
  } else {
    y = 2/ax;
    b = (-2.7460876468061399519e-14 + y*(0.28209479188874503125 +
        y*(0.54260398586720212019 + y*(0.68145162781305697748 +
        (0.44324741856237800393 + 0.13869182273440856508*y)*y))))/
        (1 + y*(1.9234811027995435174 + y*(2.5406810534399069812 +
        y*(1.8117409273494093139 + (0.76205066033991530997 +
        0.13794679143736608167*y)*y))));
    b *= exp(-x*x);
  }
  if (x < 0) {
    b = 2-b;
  }
  return b;
}

double
airErf(double x) {
  return 1.0 - airErfc(x);
}

/*
******** airBesselI0
**
** modified Bessel function of the first kind, order 0
*/
double
airBesselI0(double x) {
  double b, ax, y;

  ax = AIR_ABS(x);
  if (ax < 5.664804810929075) {
    y = x/5.7;
    y *= y;
    b = (0.9999999996966272 + y*(7.7095783675529646 +
        y*(13.211021909077445 + y*(8.648398832703904 +
        (2.5427099920536578 + 0.3103650754941674*y)*y))))/
        (1 + y*(-0.41292170755003793 + (0.07122966874756179
        - 0.005182728492608365*y)*y));
  } else {
    y = 5.7/ax;
    b = (0.398942280546057 + y*(-0.749709626164583 +
        y*(0.507462772839054 + y*(-0.0918770649691261 +
        (-0.00135238228377743 - 0.0000897561853670307*y)*y))))/
        (1 + y*(-1.90117313211089 + (1.31154807540649
        - 0.255339661975509*y)*y));
    b *= (exp(ax)/sqrt(ax));
  }
  return b;
}

/*
******** airBesselI0ExpScaled
**
** modified Bessel function of the first kind, order 0,
** scaled by exp(-abs(x)).
*/
double
airBesselI0ExpScaled(double x) {
  double b, ax, y;

  ax = AIR_ABS(x);
  if (ax < 5.664804810929075) {
    y = x/5.7;
    y *= y;
    b = (0.9999999996966272 + y*(7.7095783675529646 +
        y*(13.211021909077445 + y*(8.648398832703904 +
        (2.5427099920536578 + 0.3103650754941674*y)*y))))/
        (1 + y*(-0.41292170755003793 + (0.07122966874756179
        - 0.005182728492608365*y)*y));
    b *= exp(-ax);
  } else {
    y = 5.7/ax;
    b = (0.398942280546057 + y*(-0.749709626164583 +
        y*(0.507462772839054 + y*(-0.0918770649691261 +
        (-0.00135238228377743 - 0.0000897561853670307*y)*y))))/
        (1 + y*(-1.90117313211089 + (1.31154807540649
        - 0.255339661975509*y)*y));
    b *= (1/sqrt(ax));
  }
  return b;
}


/*
******** airBesselI1
**
** modified Bessel function of the first kind, order 1
*/
double
airBesselI1(double x) {
  double b, ax, y;

  ax = AIR_ABS(x);
  if (ax < 6.449305566387246) {
    y = x/6.45;
    y *= y;
    b = ax*(0.4999999998235554 + y*(2.370331499358438 +
        y*(3.3554331305863787 + y*(2.0569974969268707 +
        (0.6092719473097832 + 0.0792323006694466*y)*y))))/
        (1 + y*(-0.4596495788370524 + (0.08677361454866868 \
        - 0.006777712190188699*y)*y));
  } else {
    y = 6.45/ax;
    b = (0.398942280267484 + y*(-0.669339325353065 +
        y*(0.40311772245257 + y*(-0.0766281832045885 +
        (0.00248933264397244 + 0.0000703849046144657*y)*y))))/
        (1 + y*(-1.61964537617937 + (0.919118239717915 -
        0.142824922601647*y)*y));
    b *= exp(ax)/sqrt(ax);
  }
  return x < 0.0 ? -b : b;
}

/*
******** airBesselI1ExpScaled
**
** modified Bessel function of the first kind, order 1,
** scaled by exp(-abs(x))
*/
double
airBesselI1ExpScaled(double x) {
  double b, ax, y;

  ax = AIR_ABS(x);
  if (ax < 6.449305566387246) {
    y = x/6.45;
    y *= y;
    b = ax*(0.4999999998235554 + y*(2.370331499358438 +
        y*(3.3554331305863787 + y*(2.0569974969268707 +
        (0.6092719473097832 + 0.0792323006694466*y)*y))))/
        (1 + y*(-0.4596495788370524 + (0.08677361454866868 \
        - 0.006777712190188699*y)*y));
    b *= exp(-ax);
  } else {
    y = 6.45/ax;
    b = (0.398942280267484 + y*(-0.669339325353065 +
        y*(0.40311772245257 + y*(-0.0766281832045885 +
        (0.00248933264397244 + 0.0000703849046144657*y)*y))))/
        (1 + y*(-1.61964537617937 + (0.919118239717915 -
        0.142824922601647*y)*y));
    b *= 1/sqrt(ax);
  }
  return x < 0.0 ? -b : b;
}

/*
******** airLogBesselI0
**
** natural logarithm of airBesselI0
*/
double
airLogBesselI0(double x) {
  double b, ax, y;

  ax = AIR_ABS(x);
  if (ax < 4.985769687853781) {
    y = x/5.0;
    y *= y;
    b = (5.86105592521167098e-27 + y*(6.2499999970669017 +
        y*(41.1327842713925212 + y*(80.9030404787605539 +
        y*(50.7576267390706893 + 6.88231907401413133*y)))))/
        (1 + y*(8.14374525361325784 + y*(21.3288286560361152 +
        y*(20.0880670952325953 + (5.5138982784800139 +
        0.186784275148079847*y)*y))));
  } else {
    y = 5.0/ax;
    b = (-0.91893853280169872884 + y*(2.7513907055333657679 +
        y*(-3.369024122613176551 + y*(1.9164545708124343838 +
        (-0.46136261965797010108 + 0.029092365715948197066*y)*y))))/
        (1 + y*(-2.9668913151685312745 + y*(3.5882191453626541066 +
        y*(-1.9954040017063882247 + (0.45606687718126481603 -
        0.0231678041994100784*y)*y))));
    b += ax - log(ax)/2;
  }
  return b;
}

/*
******** airLogRician
**
** natural logarithm of Rician distribution
** mes is measured value
** tru is "true" underlying value
** sig is sigma of 2-D Gaussian
*/
double
airLogRician(double mes, double tru, double sig) {
  double lb, ss;

  ss = sig*sig;
  lb = airLogBesselI0(mes*tru/ss);
  return lb + log(mes/ss) - (mes*mes + tru*tru)/(2*ss);
}

double
airRician(double mes, double tru, double sig) {
  return exp(airLogRician(mes, tru, sig));
}

/*
******** airBesselI1By0
**
** the quotient airBesselI1(x)/airBesselI0(x)
*/
double
airBesselI1By0(double x) {
  double q, ax, y;

  ax = AIR_ABS(x);
  if (ax < 2.2000207427754046) {
    y = ax/2.2;
    q = (1.109010375603908e-29 + y*(1.0999999994454934 +
        y*(0.05256560007682146 + y*(0.3835178789165919 +
        (0.011328636410807382 + 0.009066934622942833*y)*y))))/
        (1 + y*(0.047786822784523904 + y*(0.9536550439261017 +
        (0.03918380275938573 + 0.09730715527121027*y)*y)));
  } else if (ax < 5.888258985638512) {
    y = (ax-2.2)/3.68;
    q = (0.7280299135046744 + y*(2.5697382341657002 +
        y*(3.69819451510548 + y*(3.131374238190916 +
        (1.2811958061688737 + 0.003601218043466571*y)*y))))/
        (1 + y*(2.8268553393021527 + y*(4.164742157157812 +
        y*(3.2377768820326756 + 1.3051900460060342*y))));
  } else {
    y = 5.88/ax;
    q = (1.000000000646262020372530870790956088593 +
         y*(-2.012513842496824157039372120680781513697 +
         y*(1.511644590219033259220408231325838531123 +
         (-0.3966391319921114140077576390415605232003 +
         0.02651815520696779849352690755529178056941*y)*y)))/
         (1 + y*(-1.927479858946526082413004924812844224781 +
         y*(1.351359456116228102988125069310621733956 +
         (-0.288087717540546638165144937495654019162 +
         0.005906535730887518966127383058238522133819*y)*y)));
  }
  return x < 0.0 ? -q : q;
}

/*
******** airBesselIn
**
** modified Bessel function of the first kind, order n.
**
*/
double
airBesselIn(int nn, double xx) {
  double tax, bb, bi, bim, bip;
  int ii, an, top;

  an = AIR_ABS(nn);
  if (0 == an) {
    return airBesselI0(xx);
  } else if (1 == an) {
    return airBesselI1(xx);
  }

  if (0.0 == xx) {
    return 0.0;
  }

  tax = 2.0/AIR_ABS(xx);
  bip = bb = 0.0;
  bi = 1.0;
  top = 2*(an + AIR_CAST(int, sqrt(40.0*an)));
  for (ii=top; ii > 0; ii--) {
    bim = bip + ii*tax*bi;
    bip = bi;
    bi = bim;
    if (AIR_ABS(bi) > 1.0e10) {
      bb *= 1.0e-10;
      bi *= 1.0e-10;
      bip*= 1.0e-10;
    }
    if (ii == an) {
      bb = bip;
    }
  }
  bb *= airBesselI0(xx)/bi;
  return (xx < 0.0 ? -bb : bb);
}

/*
******** airBesselInExpScaled
**
** modified Bessel function of the first kind, order n,
** scaled by exp(-abs(x))
**
*/
double
airBesselInExpScaled(int nn, double xx) {
  double tax, bb, bi, bim, bip;
  int top, ii, an;

  an = AIR_ABS(nn);
  if (0 == an) {
    return airBesselI0ExpScaled(xx);
  } else if (1 == an) {
    return airBesselI1ExpScaled(xx);
  }

  if (0 == xx) {
    return 0.0;
  }

  tax = 2.0/AIR_ABS(xx);
  bip = bb = 0.0;
  bi = 1.0;
  /* HEY: GLK tried to increase sqrt(40.0*an) to sqrt(100.0*an) to avoid
     jagged discontinuities in (e.g.) airBesselInExpScaled(n, 17*17); the
     problem was detected because of glitches in the highest blurring
     levels for scale-space feature detection; but that didn't quite
     work either; this will have to be debugged further! */
  top = 2*(an + AIR_CAST(int, sqrt(40.0*an)));
  for (ii=top; ii > 0; ii--) {
    bim = bip + ii*tax*bi;
    bip = bi;
    bi = bim;
    if (AIR_ABS(bi) > 1.0e10) {
      bb *= 1.0e-10;
      bi *= 1.0e-10;
      bip*= 1.0e-10;
    }
    if (ii == an) {
      bb = bip;
    }
  }
  bb *= airBesselI0ExpScaled(xx)/bi;
  return (xx < 0.0 ? -bb : bb);
}

/*  http://en.wikipedia.org/wiki/Halton_sequences */
double
airVanDerCorput(unsigned int indx, unsigned int base) {
  double result=0.0, ff;
  unsigned int ii;

  ff = 1.0/base;
  ii = indx;
  while (ii) {
    result += ff*(ii % base);
    ii /= base;
    ff /= base;
  }
  return result;
}

void
airHalton(double *out, unsigned int indx,
          const unsigned int *base, unsigned int num) {
  unsigned int nn, bb;

  for (nn=0; nn<num; nn++) {
    double ff, result = 0.0;
    unsigned int ii;
    bb = base[nn];
    ff = 1.0/bb;
    ii = indx;
    while (ii) {
      result += ff*(ii % bb);
      ii /= bb;
      ff /= bb;
    }
    out[nn] = result;
  }
  return;
}

/* via "Table[Prime[n], {n, 1000}]" in Mathematica */
const unsigned int airPrimeList[AIR_PRIME_NUM] = {
  2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61,
  67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137,
  139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
  223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283,
  293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379,
  383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461,
  463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563,
  569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643,
  647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739,
  743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829,
  839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937,
  941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021,
  1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093,
  1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181,
  1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259,
  1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
  1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433,
  1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493,
  1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579,
  1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657,
  1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741,
  1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831,
  1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913,
  1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003,
  2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087,
  2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161,
  2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269,
  2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347,
  2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417,
  2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531,
  2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621,
  2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693,
  2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767,
  2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851,
  2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953,
  2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041,
  3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163,
  3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251,
  3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329,
  3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413,
  3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517,
  3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583,
  3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673,
  3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767,
  3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853,
  3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931,
  3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027,
  4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129,
  4133, 4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229,
  4231, 4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297,
  4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421,
  4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513,
  4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603,
  4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691,
  4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793,
  4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909,
  4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987,
  4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077,
  5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171,
  5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279,
  5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393,
  5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471,
  5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557,
  5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647, 5651, 5653,
  5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741,
  5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839,
  5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903,
  5923, 5927, 5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043,
  6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131,
  6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221,
  6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311,
  6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379,
  6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521,
  6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607,
  6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703,
  6709, 6719, 6733, 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803,
  6823, 6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871, 6883, 6899,
  6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983,
  6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079,
  7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207,
  7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307,
  7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417, 7433,
  7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517, 7523,
  7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589,
  7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687,
  7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789,
  7793, 7817, 7823, 7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883,
  7901, 7907, 7919};

/*
** CRC code available in various places, including:
** http://pubs.opengroup.org/onlinepubs/007904875/utilities/cksum.html
** which curiously has no copyright declaration?
*/

static unsigned int
crcTable[] = {
  0x00000000,
  0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
  0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6,
  0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac,
  0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8, 0x6ed82b7f,
  0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a,
  0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58,
  0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033,
  0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027, 0xddb056fe,
  0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4,
  0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
  0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5,
  0x2ac12072, 0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 0x7897ab07,
  0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c,
  0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1,
  0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b,
  0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698,
  0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d,
  0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 0xc6bcf05f,
  0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
  0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80,
  0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a,
  0x58c1663d, 0x558240e4, 0x51435d53, 0x251d3b9e, 0x21dc2629,
  0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c,
  0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e,
  0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65,
  0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601, 0xdea580d8,
  0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2,
  0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
  0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74,
  0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 0x7b827d21,
  0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a,
  0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e, 0x18197087,
  0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d,
  0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce,
  0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb,
  0xdbee767c, 0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 0x89b8fd09,
  0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
  0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf,
  0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

/* note "c" is used once */
#define CRC32(crc, c) (crc) = (((crc) << 8) ^ crcTable[((crc) >> 24) ^ (c)])

unsigned int
airCRC32(const unsigned char *cdata, size_t len, size_t unit, int swap) {
  unsigned int crc=0;
  size_t ii, jj, nn, mm;
  const unsigned char *crev;

  if (!(cdata && len)) {
    return 0;
  }
  if (swap) {
    /* if doing swapping, we need make sure we have a unit size,
       and that it divides into len */
    if (!(unit && !(len % unit))) {
      return 0;
    }
  }
  nn = len;

  if (!swap) {
    /* simple case: feed "len" bytes from "cdata" into CRC32 */
    for (ii=0; ii<nn; ii++) {
      CRC32(crc, *(cdata++));
    }
  } else {
    /* have to swap, work "unit" bytes at a time, working down
       the bytes within each unit */
    mm = len / unit;
    for (jj=0; jj<mm; jj++) {
      crev = cdata + jj*unit + unit-1;
      for (ii=0; ii<unit; ii++) {
        CRC32(crc, *(crev--));
      }
    }
  }

  /* include length of data in result */
  for (; nn; nn >>= 8) {
    CRC32(crc, (nn & 0xff));
  }

  return ~crc;
}
