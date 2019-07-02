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


#include <teem/air.h>
#include <teem/biff.h>
#include "../nrrd.h"

void
usage(char *me) {   /*    0     1   2    3       4   (5)  */
  fprintf(stderr, "usage: %s <min> <N> <max> <fileOut>\n", me);
  exit(1);
}

int
main(int argc, char *argv[]) {
  char *me;
  unsigned int ii, NN;
  double min, max, *out;
  Nrrd *nout;

  me = argv[0];
  if (5 != argc) {
    usage(me);
  }
  if (3 != (sscanf(argv[2], "%u", &NN)
            + sscanf(argv[1], "%lf", &min)
            + sscanf(argv[3], "%lf", &max))) {
    fprintf(stderr, "%s: couldn't parse %s %s %s double uint double",
            me, argv[1], argv[2], argv[3]);
    usage(me);
  }
  nout = nrrdNew();
  if (nrrdAlloc_va(nout, nrrdTypeDouble, 2,
                   AIR_CAST(size_t, 5),
                   AIR_CAST(size_t, NN))) {
    fprintf(stderr, "%s: trouble allocating:\n%s", me,
            biffGetDone(NRRD));
    exit(1);
  }
  out = AIR_CAST(double *, nout->data);
  for (ii=0; ii<NN; ii++) {
    double xx, rr, ff, gg;
    xx = AIR_AFFINE(0, ii, NN-1, min, max);
    rr = exp(xx);
    ff = airFastExp(xx);
    gg = airExp(xx);
    if (rr < 0 || ff < 0 || gg < 0
        || !AIR_EXISTS(rr) || !AIR_EXISTS(ff) || !AIR_EXISTS(gg)) {
      fprintf(stderr, "%s: problem: %f -> real %f, approx %f %f\n",
              me, xx, rr, ff, gg);
      exit(1);
    }
    out[0] = rr;
    out[1] = ff;
    out[2] = (ff-rr)/rr;
    out[3] = gg;
    out[4] = (gg-rr)/rr;
    out += 5;
  }

  if (nrrdSave(argv[4], nout, NULL)) {
    fprintf(stderr, "%s: trouble saving:\n%s", me,
            biffGetDone(NRRD));
    exit(1);
  }
  exit(0);
}
