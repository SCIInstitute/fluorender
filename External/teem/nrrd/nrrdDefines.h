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

#ifndef NRRD_DEFINES_HAS_BEEN_INCLUDED
#define NRRD_DEFINES_HAS_BEEN_INCLUDED

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/* feel free to set these to higher values and recompile */
#define NRRD_DIM_MAX 16            /* Max array dimension (nrrd->dim) */
#define NRRD_SPACE_DIM_MAX 8       /* Max dimension of "space" around array
                                      (nrrd->spaceDim) */

#define NRRD_EXT_NRRD   ".nrrd"
#define NRRD_EXT_NHDR   ".nhdr"
#define NRRD_EXT_PGM    ".pgm"
#define NRRD_EXT_PPM    ".ppm"
#define NRRD_EXT_PNG    ".png"
#define NRRD_EXT_VTK    ".vtk"
#define NRRD_EXT_TEXT   ".txt"
#define NRRD_EXT_EPS    ".eps"

/* HEY: should this be renamed -> MAXNUM ? Would be more consistent
   with other Teem pound-define names */
#define NRRD_KERNEL_PARMS_NUM 8    /* max # arguments to a kernel-
                                      this is weird: it isn't the max
                                      of any of the NrrdKernels
                                      defined by the nrrd library
                                      (that is more like 3), but is
                                      the max number of parms of any
                                      NrrdKernel used by anyone using
                                      Teem, such as in gage.
                                      Enforcing one global max
                                      simplifies implementation. */

/*
** For the 64-bit integer types (not standard except in C99), we used
** to try to use the names for the _MIN and _MAX values which are used
** in C99 (as well as gcc) such as LLONG_MAX, or those used on SGI
** such as LONGLONG_MAX.  However, since the tests (in nrrdSanity)
** were re-written to detect overflow based on manipulation of
** specific values, we might as well also define the _MIN and _MAX in
** terms of explicit values (which agree with those defined by C99).
*/

#define NRRD_LLONG_MAX AIR_LLONG(9223372036854775807)
#define NRRD_LLONG_MIN (-NRRD_LLONG_MAX-AIR_LLONG(1))
#define NRRD_ULLONG_MAX AIR_ULLONG(18446744073709551615)

/*
** Chances are, you shouldn't mess with these
*/

/* ---- BEGIN non-NrrdIO */
/* suffix string that indicates percentile-based min/max */
#define NRRD_MINMAX_PERC_SUFF "%"
/* ---- END non-NrrdIO */
#define NRRD_COMMENT_CHAR '#'
#define NRRD_FILENAME_INCR 32
#define NRRD_COMMENT_INCR 16
#define NRRD_KEYVALUE_INCR 32
#define NRRD_LIST_FLAG "LIST"
#define NRRD_PNM_COMMENT "# NRRD>"    /* this is designed to be robust against
                                         the mungling that xv does, but no
                                         promises for any other image
                                         programs */

#define NRRD_PNG_FIELD_KEY "NRRD"     /* this is the key used for getting nrrd
                                         fields into/out of png comments */
#define NRRD_PNG_COMMENT_KEY "NRRD#"  /* this is the key used for getting nrrd
                                         comments into/out of png comments */

#define NRRD_UNKNOWN  "???"           /* how to represent something unknown in
                                         a field of the nrrd header, when it
                                         being unknown is not an error */
#define NRRD_NONE "none"              /* like NRRD_UNKNOWN, but with an air
                                         of certainty */

#ifdef __cplusplus
}
#endif

#endif /* NRRD_DEFINES_HAS_BEEN_INCLUDED */
