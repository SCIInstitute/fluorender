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


#include "../air.h"

/*
**    0      1    2    3   (4)
** logrice  tru  mes  sig
*/
int
main(int argc, char *argv[]) {
  char *me;
  double tru, mes, sig;

  me = argv[0];
  if (4 != argc
      || 1 != sscanf(argv[1], "%lg", &tru)
      || 1 != sscanf(argv[2], "%lg", &mes)
      || 1 != sscanf(argv[3], "%lg", &sig)) {
    fprintf(stderr, "%s: need three doubles\n", me);
    exit(1);
  }

  printf("log(Rician(%g,%g,%g)) = %g\n",
         tru, mes, sig, airLogRician(tru, mes, sig));

  exit(0);
}
