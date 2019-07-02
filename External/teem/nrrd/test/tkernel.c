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

#include "../nrrd.h"

/*
** this program demonstrates parsing a string into a kernel with parms,
** verifies that all the kernel methods are consistent and/or equal,
** and produces a text file of the kernel evaluated many times.
** The output can be "plotted" with unu jhisto, as with:
** tkernel ctmr -2 0.0001 2 - | unu jhisto -b 800 300 | unu flip -a 1 | unu quantize -b 8 -o tmp.png
*/

void
usage(char *me) {
  /*                      0     1       2      3     4        5      (6)   (7)*/
  fprintf(stderr, "usage: %s <kernel> <min> <step> <max> <txtOut> [dkern]\n", me);
  exit(1);
}


#define CLOSE(a,b, eps)      (fabs((a)-(b)) < eps)

int
main(int argc, char *argv[]) {
  char *me, *kernS[2], *minS, *stepS, *maxS, *outS, *err, kstr[AIR_STRLEN_LARGE];
  const NrrdKernel *kern[2];
  NrrdKernelSpec *ksp[2];
  double parm[NRRD_KERNEL_PARMS_NUM], min, step, max, integral,
    *dom_d, *ran_d;
  float *dom_f, *ran_f, val, r_f, r_d;
  FILE *fout;
  int i, len;
  airArray *mop;
  unsigned int kii;

  me = argv[0];
  if (!( 6 == argc || 7 == argc )) {
    usage(me);
  }
  kernS[0] = argv[1];
  minS = argv[2];
  stepS = argv[3];
  maxS = argv[4];
  outS = argv[5];
  if (7 == argc) {
    kernS[1] = argv[6];
  } else {
    kernS[1] = NULL;
  }

  if (3 != (sscanf(minS, "%lf", &min) +
            sscanf(stepS, "%lf", &step) +
            sscanf(maxS, "%lf", &max))) {
    fprintf(stderr, "%s: couldn't parse \"%s\", \"%s\", \"%s\" as 3 doubles\n",
            me, minS, stepS, maxS);
    exit(1);
  }

  mop = airMopNew();
  for (kii=0; kii<=(kernS[1] ? 1 : 0); kii++) {
    if (nrrdKernelParse(&(kern[kii]), parm, kernS[kii])) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: (kii %u) trouble:\n%s\n", me, kii, err);
      airMopError(mop);
      exit(1);
    }
    ksp[kii] = nrrdKernelSpecNew();
    airMopAdd(mop, ksp[kii], (airMopper)nrrdKernelSpecNix, airMopAlways);
    nrrdKernelSpecSet(ksp[kii], kern[kii], parm);
    if (nrrdKernelSpecSprint(kstr, ksp[kii])) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s\n", me, err);
      airMopError(mop);
      exit(1);
    }
    fprintf(stderr, "%s: printed kernel as \"%s\"\n", me, kstr);
    if (!( min <= -kern[kii]->support(parm)
           && max >= kern[kii]->support(parm) )) {
      fprintf(stderr, "%s: WARNING: support=%g => lower min (%g) or raise max (%g)\n",
              me, kern[kii]->support(parm), min, max);
    }
    fprintf(stderr, "%s: support(%s) = %g\n", me, kstr, kern[kii]->support(parm));
  }

  /* see how many values are in the interval */
  len = 0;
  for (val=min; val<=max; val+=step) {
    len++;
  }
  /* allocate domain and range for both float and double */
  if (!( (dom_d = (double *)calloc(len, sizeof(double))) &&
         (ran_d = (double *)calloc(len, sizeof(double))) &&
         (dom_f = (float *)calloc(len, sizeof(float))) &&
         (ran_f = (float *)calloc(len, sizeof(float))) )) {
    fprintf(stderr, "%s: PANIC: couldn't allocate buffers\n", me);
    exit(1);
  }
  airMopAdd(mop, dom_d, airFree, airMopAlways);
  airMopAdd(mop, ran_d, airFree, airMopAlways);
  airMopAdd(mop, dom_f, airFree, airMopAlways);
  airMopAdd(mop, ran_f, airFree, airMopAlways);
  /* set values in both domains */
  i=0;
  for (val=min; val<=max; val+=step) {
    /* note that the value stored in dom_d[i] is only a
       single-precision float, so that it is really equal to dom_f[i] */
    dom_d[i] = val;
    dom_f[i] = val;
    i++;
  }
  /* do the vector evaluations */
  kern[0]->evalN_f(ran_f, dom_f, len, parm);
  kern[0]->evalN_d(ran_d, dom_d, len, parm);
  /* do the single evaluations, and make sure everything agrees */
  i = 0;
  integral = 0;
  for (val=min; val<=max; val+=step) {
    /* compare two single evaluations */
    r_f = kern[0]->eval1_f(val, parm);
    r_d = kern[0]->eval1_d(val, parm);
    if (!CLOSE(r_f,r_d, 0.00001)) {
      fprintf(stderr, "%s: (eval1_f(%g)== %f) != (eval1_d(%g)== %f)\n",
              me, val, r_f, val, r_d);
    }
    /* compare single float with vector float */
    if (!CLOSE(r_f,ran_f[i], 0.00001)) {
      fprintf(stderr, "%s: (eval1_f(%g)== %f) != (evalN_f[%d]== %f)\n",
              me, val, r_f, i, ran_f[i]);
    }
    /* compare single float with vector double */
    r_d = ran_d[i];
    if (!CLOSE(r_f,r_d, 0.00001)) {
      fprintf(stderr, "%s: (eval1_f(%g)== %f) != (evalN_d[%d]== %f)\n",
              me, val, r_f, i, r_d);
    }
    integral += step*ran_d[i];
    /* possibly check on given derivatives */
    if (kern[1]) {
      double numd;
      numd = (kern[0]->eval1_d(val+step/2, parm)
              - kern[0]->eval1_d(val-step/2, parm))/step;
      if (!CLOSE(numd, kern[1]->eval1_d(val+step, parm), 0.005)) {
        fprintf(stderr, "%s: |numerical f'(%g) %g - true %g| = %g > 0.005\n",
                me, val, numd, kern[1]->eval1_d(val+step, parm),
                fabs(numd - kern[1]->eval1_d(val+step, parm)));
        /* exit(1); */
      }
    }
    i++;
  }
  if (!CLOSE(integral, kern[0]->integral(parm), 0.0005)) {
    fprintf(stderr, "%s: HEY HEY HEY HEY HEY HEY!\n", me);
    fprintf(stderr,
            "%s: discrete integral %f != %f\n", me, integral, kern[0]->integral(parm));
    /* exit(1); */
  }

  /* it all checks out; write the file */
  if (!(fout = airFopen(outS, stdout, "w"))) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing\n", me, outS);
    exit(1);
  }
  for (i=0; i<=len-1; i++) {
    fprintf(fout, "%g %g\n", dom_f[i], ran_f[i]);
  }
  fclose(fout);

  airMopOkay(mop);
  exit(0);
}
