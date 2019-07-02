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

char *morphInfo = ("testing. ");

int
morph(Nrrd *nout, Nrrd *_nin, Nrrd *_nkern, float scl) {
  static const char me[]="morph";
  airArray *mop;
  Nrrd *nin, *nkern;
  int sx, sy, sz, kd, kr, xx, yy, zz, ii, jj, kk;
  float *in, *out, *kern;

  mop = airMopNew();
  if (nrrdTypeFloat == _nin->type) {
    nin = _nin;
  } else {
    nin = nrrdNew();
    airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(nin, _nin, nrrdTypeFloat)) {
      biffAddf(NRRD, "%s: trouble 1", me);
      airMopError(mop); return 1;
    }
  }
  if (nrrdTypeFloat == _nkern->type) {
    nkern = _nkern;
  } else {
    nkern = nrrdNew();
    airMopAdd(mop, nkern, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(nkern, _nkern, nrrdTypeFloat)) {
      biffAddf(NRRD, "%s: trouble 2", me);
      airMopError(mop); return 1;
    }
  }
  if (nrrdCopy(nout, nin)) {
    biffAddf(NRRD, "%s: trouble allocating output", me);
    airMopError(mop); return 1;
  }
  in = AIR_CAST(float *, nin->data);
  out = AIR_CAST(float *, nout->data);
  kern = AIR_CAST(float *, nkern->data);
  sx = AIR_CAST(int, nin->axis[0].size);
  sy = AIR_CAST(int, nin->axis[1].size);
  sz = AIR_CAST(int, nin->axis[2].size);
  kd = AIR_CAST(int, nkern->axis[0].size);
  kr = kd/2; /* 5 -> 2 */

  /* min_i (f(x+i)- k(i)) */
  for (zz=kr; zz<sz-kr; zz++) {
    fprintf(stderr, "%d/%d\n", zz, sz-kr);
    for (yy=kr; yy<sy-kr; yy++) {
      for (xx=kr; xx<sx-kr; xx++) {
        float mind, ival, kval;
        /*
        int verb;

        verb = ((24 == xx && 24 == yy && 9 == zz) ||
                (24 == xx && 24 == yy && 10 == zz));
        */
        mind = AIR_POS_INF;
        for (kk=-kr; kk<=kr; kk++) {
          for (jj=-kr; jj<=kr; jj++) {
            for (ii=-kr; ii<=kr; ii++) {
              ival = in[xx+ii + sx*(yy+jj + sy*(zz+kk))];
              kval = kern[ii+kr + kd*(jj+kr + kd*(kk+kr))];
              mind = AIR_MIN(mind, ival - scl*kval);
            }
          }
        }
        out[xx + sx*(yy + sy*zz)] = mind;
      }
    }
  }

  airMopOkay(mop);
  return 0;
}

int
main(int argc, const char **argv) {
  const char *me;
  char *outS;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  char *err;
  Nrrd *nin, *nkern, *nout;
  float scl;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input image", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "k", "nin", airTypeOther, 1, 1, &nkern, NULL,
             "kernel", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "s", "scl", airTypeFloat, 1, 1, &scl, "1.0",
             "scaling on kernel");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output filename", NULL);

  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, morphInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( 3 == nkern->dim &&
         nkern->axis[0].size == nkern->axis[1].size &&
         nkern->axis[0].size == nkern->axis[2].size &&
         1 == nkern->axis[0].size % 2)) {
    fprintf(stderr, "%s: need cubical kernel w/ odd # sample on edge", me);
    airMopError(mop); exit(1);
  }
  if (!( 3 == nin->dim )) {
    fprintf(stderr, "%s: need 3D input", me);
    airMopError(mop); exit(1);
  }

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (morph(nout, nin, nkern, scl)) {
    airMopAdd(mop, err = biffGet(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s", me, err);
    airMopError(mop); exit(1);
  }
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGet(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving \"%s\":\n%s",
            me, outS, err);
    airMopError(mop); exit(1);
  }

  airMopOkay(mop);
  exit(0);
}
