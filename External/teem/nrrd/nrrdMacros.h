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

#ifndef NRRD_MACROS_HAS_BEEN_INCLUDED
#define NRRD_MACROS_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*
******** NRRD_CELL_POS, NRRD_NODE_POS, NRRD_POS
******** NRRD_CELL_IDX, NRRD_NODE_IDX, NRRD_IDX
**
** the guts of nrrdAxisPos() and nrrdAxisIdx(), for converting
** between "index space" location and "position" or "world space" location,
** given the centering, min and max "position", and number of samples.
**
** Unlike nrrdAxisPos() and nrrdAxisIdx(), this assumes that center
** is either nrrdCenterCell or nrrdCenterNode, but not nrrdCenterUnknown.
*/
/* index to position, cell centering */
#define NRRD_CELL_POS(min, max, size, idx)       \
  AIR_AFFINE(0, (idx) + 0.5, (size), (min), (max))

/* index to position, node centering */
#define NRRD_NODE_POS(min, max, size, idx)       \
  AIR_AFFINE(0, (idx), (size)-1, (min), (max))

/* index to position, either centering */
#define NRRD_POS(center, min, max, size, idx)    \
  (nrrdCenterCell == (center)                         \
   ? NRRD_CELL_POS((min), (max), (size), (idx))  \
   : NRRD_NODE_POS((min), (max), (size), (idx)))

/* position to index, cell centering */
#define NRRD_CELL_IDX(min, max, size, pos)       \
  (AIR_AFFINE((min), (pos), (max), 0, (size)) - 0.5)

/* position to index, node centering */
#define NRRD_NODE_IDX(min, max, size, pos)       \
  AIR_AFFINE((min), (pos), (max), 0, (size)-1)

/* position to index, either centering */
#define NRRD_IDX(center, min, max, size, pos)    \
  (nrrdCenterCell == (center)                         \
   ? NRRD_CELL_IDX((min), (max), (size), (pos))  \
   : NRRD_NODE_IDX((min), (max), (size), (pos)))

/*
******** NRRD_SPACING
**
** the guts of nrrdAxisSpacing(), determines the inter-sample
** spacing, given centering, min and max "position", and number of samples
**
** Unlike nrrdAxisSpacing, this assumes that center is either
** nrrdCenterCell or nrrdCenterNode, but not nrrdCenterUnknown.
*/
#define NRRD_SPACING(center, min, max, size)        \
  (nrrdCenterCell == center                         \
   ? ((max) - (min))/AIR_CAST(double, size)         \
   : ((max) - (min))/(AIR_CAST(double, (size)- 1))) \

/*
******** NRRD_COORD_UPDATE
**
** This is for doing the "carrying" associated with gradually incrementing an
** array of coordinates.  Assuming that the given coordinate array "coord" has
** been incremented by adding 1 to coord[0], this macro will propagating the
** change up to higher axes (when the coordinate has reached the size on a
** lower axis.)  In addition, the final statement of the macro prevents the
** last index from going past a valid value.
**
** Assumptions:
** -- coord[] and size[] should both be arrays of unsigned integral values,
**    presumably size_t
** -- size[i] is >= 1 for all i<dim (size 0 is invalid)
** -- dim is an unsigned int (0 is ok; result is a no-op)
** Violating these will create invalid coordinate arrays or generate
** compiler warnings about comparisons between signed and unsigned.
**
** The "ddd" variable name in this and subsequent macros is an effort to
** avoid possible symbol name shadowing.
*/
#define NRRD_COORD_UPDATE(coord, size, dim)                             \
  {                                                                     \
    unsigned int ddd;                                                   \
    for (ddd=0;                                                         \
         ddd+1 < (dim) && (coord)[ddd] >= (size)[ddd];                  \
         ddd++) {                                                       \
      (coord)[ddd] = 0;                                                 \
      (coord)[ddd+1]++;                                                 \
    }                                                                   \
    if (dim) {                                                          \
      (coord)[(dim)-1] = AIR_MIN((coord)[(dim)-1], (size)[(dim)-1]-1);  \
    }                                                                   \
  }

/*
******** NRRD_COORD_INCR
**
** increments coord[idx] (by one) and then calls NRRD_COORD_UPDATE to
** propagate this change as necessary to higher numbered axes.  Does
** nothing if idx>=dim, since that would be an invalid index into
** coord[] and size[]
*/
#define NRRD_COORD_INCR(coord, size, dim, idx)                          \
  if ((idx) < (dim)) {                                                  \
    (coord)[(idx)]++;                                                   \
    NRRD_COORD_UPDATE((coord)+(idx), (size)+(idx), (dim)-(idx));        \
  }

/*
******** NRRD_INDEX_GEN
**
** Given array coordinates "coord" and sizes "size", both of length "dim",
** this calculates the linear index represented by coord (assuming lower
** coordinates are for *faster* axes), and stores it in "I".  Has the same
** assumptions as NRRD_COORD_UPDATE.
*/
#define NRRD_INDEX_GEN(I, coord, size, dim)             \
  {                                                     \
    unsigned int ddd = (dim);                           \
    (I) = 0;                                            \
    while (ddd) {                                       \
      ddd--;                                            \
      (I) = (coord)[ddd] + (size)[ddd]*(I);             \
    }                                                   \
  }

/*
******** NRRD_COORD_GEN
**
** opposite of NRRD_INDEX_GEN: going from linear index "I" to
** coordinate array "coord".
*/
#define NRRD_COORD_GEN(coord, size, dim, I)   \
  {                                           \
    unsigned int ddd;                         \
    size_t myI = (I);                         \
    for (ddd=0; ddd<(dim); ddd++) {           \
      (coord)[ddd] = myI % (size)[ddd];       \
      myI /= (size)[ddd];                     \
    }                                         \
  }

#ifdef __cplusplus
}
#endif

#endif /* NRRD_MACROS_HAS_BEEN_INCLUDED */
