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
** Until Teem has its own printf implementation, this will have to do;
** it is imperfect because these are not functionally identical.
*/
#if defined(WIN32) || defined(_WIN32)
#  define snprintf _snprintf
#endif

/*
******** airEnumUnknown
**
** return the value representing "unknown" in an enum
*/
int
airEnumUnknown(const airEnum *enm) {

  if (enm && enm->val) {
    return enm->val[0];
  } else {
    return 0;
  }
}

/*
** _airEnumIndex()
**
** given an enum "enm" and value "val", return the index into enm->str[]
** and enm->desc[] which correspond to that value.  To be safe, when
** given an invalid enum value, we return zero.
*/
static unsigned int
_airEnumIndex(const airEnum *enm, int val) {
  unsigned int ii, ret;

  ret = 0;
  if (enm->val) {
    for (ii=1; ii<=enm->M; ii++) {
      if (val == enm->val[ii]) {
        ret = ii;
        break;
      }
    }
  } else {
    unsigned int uval;
    uval = AIR_UINT(val);
    ret = (0 <= val && uval <= enm->M) ? uval : 0;
  }
  return ret;
}

/*
** returns non-zero if there is an error: given "val" is *not*
** a valid value of the airEnum "enm"
*/
int
airEnumValCheck(const airEnum *enm, int val) {

  return (0 == _airEnumIndex(enm, val));
}

const char *
airEnumStr(const airEnum *enm, int val) {
  unsigned int idx;

  idx = _airEnumIndex(enm, val);
  return enm->str[idx];
}

const char *
airEnumDesc(const airEnum *enm, int val) {
  unsigned int idx;

  idx = _airEnumIndex(enm, val);
  return enm->desc[idx];
}

int
airEnumVal(const airEnum *enm, const char *str) {
  char *strCpy, test[AIR_STRLEN_SMALL];
  unsigned int ii;

  if (!str) {
    return airEnumUnknown(enm);
  }

  strCpy = airStrdup(str);
  if (!enm->sense) {
    airToLower(strCpy);
  }

  if (enm->strEqv) {
    /* want strlen and not airStrlen here because the strEqv array
       should be terminated by a non-null empty string */
    for (ii=0; strlen(enm->strEqv[ii]); ii++) {
      airStrcpy(test, AIR_STRLEN_SMALL, enm->strEqv[ii]);
      if (!enm->sense) {
        airToLower(test);
      }
      if (!strcmp(test, strCpy)) {
        free(strCpy);
        return enm->valEqv[ii];
      }
    }
  } else {
    /* enm->strEqv NULL */
    for (ii=1; ii<=enm->M; ii++) {
      airStrcpy(test, AIR_STRLEN_SMALL, enm->str[ii]);
      if (!enm->sense) {
        airToLower(test);
      }
      if (!strcmp(test, strCpy)) {
        free(strCpy);
        return enm->val ? enm->val[ii] : (int)ii; /* HEY scrutinize cast */
      }
    }
  }

  /* else we never matched a string */
  free(strCpy);
  return airEnumUnknown(enm);
}

/*
******** airEnumFmtDesc()
**
** Formats a description line for one element "val" of airEnum "enm",
** and puts the result in a NEWLY ALLOCATED string which is the return
** of this function.  The formatting is done via sprintf(), as governed
** by "fmt", which should contain to "%s" conversion sequences, the
** first for the string version "val", and the second for the
** description If "canon", then the canonical string representation
** will be used (the one in enm->str[]), otherwise the shortest string
** representation will be used (which differs from the canonical one
** when there is a strEqv[]/valEqv[] pair defining a shorter string)
*/
char *
airEnumFmtDesc(const airEnum *enm, int val, int canon, const char *fmt) {
  const char *desc;
  char *buff, ident[AIR_STRLEN_SMALL];
  const char *_ident;
  int i;
  size_t len;

  if (!(enm && enm->desc && fmt)) {
    return airStrdup("(airEnumDesc: invalid args)");
  }
  if (airEnumValCheck(enm, val)) {
    val = airEnumUnknown(enm);
  }
  _ident = airEnumStr(enm, val);
  if (!canon && enm->strEqv) {
    len = airStrlen(_ident);
    for (i=0; airStrlen(enm->strEqv[i]); i++) {
      if (val != enm->valEqv[i]) {
        /* this isn't a string representing the value we care about */
        continue;
      }
      if (airStrlen(enm->strEqv[i]) < len) {
        /* this one is shorter */
        len = airStrlen(enm->strEqv[i]);
        _ident = enm->strEqv[i];
      }
    }
  }
  airStrcpy(ident, AIR_STRLEN_SMALL, _ident);
  if (!enm->sense) {
    airToLower(ident);
  }
  desc = enm->desc[_airEnumIndex(enm, val)];
  buff = AIR_CALLOC(airStrlen(fmt) + airStrlen(ident) +
                    airStrlen(desc) + 1, char);
  if (buff) {
    sprintf(buff, fmt, ident, desc);
  }
  return buff;
}

static void
_enumPrintVal(FILE *file, const airEnum *enm, int ii) {

  if (enm->desc) {
    fprintf(file, "desc: %s\n", enm->desc[ii]);
  }
  if (enm->strEqv) {
    unsigned int jj;
    fprintf(file, "eqv:"); fflush(file);
    jj = 0;
    while (airStrlen(enm->strEqv[jj])) {
      if (enm->valEqv[jj] == (enm->val
                              ? enm->val[ii]
                              : ii)) {
        fprintf(file, " \"%s\"", enm->strEqv[jj]);
      }
      jj++;
    }
    fprintf(file, "\n");
  }
}

void
airEnumPrint(FILE *file, const airEnum *enm) {
  int ii; /* this should arguable be unsigned int, but
             airEnum values were kept as "int", even after
             the great unsigned conversion */

  if (!(file && enm)) {
    return;
  }

  if (airStrlen(enm->name)) {
    fprintf(file, "airEnum \"%s\":\n", enm->name);
  } else {
    fprintf(file, "airEnum (NO NAME!):\n");
  }
  fprintf(file, "(%s case sensitive)\n", (enm->sense ? "yes, is" : "is not"));
  if (enm->val) {
    fprintf(file, "Values (%u valid) given explicitly\n", enm->M);
    fprintf(file, "--- (0) %d: \"%s\"\n", enm->val[0], enm->str[0]);
    for (ii=1; ii<=AIR_CAST(int, enm->M); ii++) {
      fprintf(file, "--- (%d) %d: \"%s\" == \"%s\"\n", ii,
              enm->val[ii], enm->str[ii],
              airEnumStr(enm, enm->val[ii]));
      _enumPrintVal(file, enm, ii);
    }
  } else {
    /* enm->val NULL */
    fprintf(file, "Values implicit; [1,%u] valid\n", enm->M);
    fprintf(file, "--- 0: \"%s\"\n", enm->str[0]);
    for (ii=1; ii<=AIR_CAST(int, enm->M); ii++) {
      fprintf(file, "--- %d: %s == %s\n", ii, enm->str[ii],
              airEnumStr(enm, ii));
      _enumPrintVal(file, enm, ii);
    }
  }
  return;
}

/* ---- BEGIN non-NrrdIO */
/*
******** airEnumCheck
**
** tries various things to check on the construction and internal
** consistency of an airEnum; returns 1 if there is a problem, and 0
** if all is well.  we're in air, so there's no biff, but we sprintf a
** description of the error into "err", if given
**
** The requirement that the strings have strlen <= AIR_STRLEN_SMALL-1
** is a reflection of the cheap implementation of the airEnum
** functions in this file, rather than an actual restriction on what an
** airEnum could be.
*/
int
airEnumCheck(char err[AIR_STRLEN_LARGE], const airEnum *enm) {
  static const char me[]="airEnumCheck";
  unsigned int ii, jj;
  size_t slen, ASL;

  ASL = AIR_STRLEN_LARGE;
  if (!enm) {
    if (err) {
      snprintf(err, ASL, "%s: got NULL enm", me);
    }
    return 1;
  }
  if (!enm->name) {
    if (err) {
      snprintf(err, ASL, "%s: enm->name NULL", me);
    }
    return 1;
  }
  if (0 == enm->M) {
    if (err) {
      snprintf(err, ASL, "%s(%s): enm->M zero; no values in enum",
               me, enm->name);
    }
    return 1;
  }
  for (ii=0; ii<=enm->M; ii++) {
    if (!enm->str[ii]) {
      if (err) {
        snprintf(err, ASL, "%s(%s): enm->str[%u] NULL", me, enm->name, ii);
      }
      return 1;
    }
    slen = airStrlen(enm->str[ii]);
    if (!( slen >= 1 && slen <= AIR_STRLEN_SMALL-1 )) {
      if (err) {
        char stmp[AIR_STRLEN_SMALL];
        snprintf(err, ASL, "%s(%s): strlen(enm->str[%u] \"%s\") "
                 "%s not in range [1,%u]", me,
                 enm->name, ii, enm->str[ii],
                 airSprintSize_t(stmp, slen), AIR_STRLEN_SMALL-1);
      }
      return 1;
    }
    /* make sure there are no duplicates among the strings,
       including remapping the case in case of case insensitivity */
    for (jj=ii+1; jj<=enm->M; jj++) {
      if (!strcmp(enm->str[ii], enm->str[jj])) {
        if (err) {
          snprintf(err, ASL, "%s(%s): str[%d] and [%u] both \"%s\"",
                   me, enm->name, ii, jj, enm->str[jj]);
        }
        return 1;
      }
      if (!enm->sense) {
        char bb1[AIR_STRLEN_SMALL], bb2[AIR_STRLEN_SMALL];
        strcpy(bb1, enm->str[ii]);
        airToLower(bb1);
        strcpy(bb2, enm->str[jj]);
        airToLower(bb2);
        if (!strcmp(bb1, bb2)) {
          if (err) {
            snprintf(err, ASL, "%s(%s): after case-lowering, "
                     "str[%d] and [%u] both \"%s\"",
                     me, enm->name, ii, jj, bb1);
          }
          return 1;
        }
      }
    }
  }
  if (enm->val) {
    for (ii=1; ii<=enm->M; ii++) {
      if (enm->val[0] == enm->val[ii]) {
        if (err) {
          snprintf(err, ASL, "%s(%s): val[%u] %u same as "
                   "unknown/invalid val[0] %u",
                   me, enm->name, ii, enm->val[ii], enm->val[0]);
        }
        return 1;
      }
      for (jj=ii+1; jj<=enm->M; jj++) {
        if (enm->val[jj] == enm->val[ii]) {
          if (err) {
            snprintf(err, ASL, "%s(%s): val[%u] %u same as val[%u] %u", me,
                     enm->name, ii, enm->val[ii], jj, enm->val[jj]);
          }
          return 1;
        }
      }
    }
  }
  if (enm->desc) {
    for (ii=0; ii<=enm->M; ii++) {
      if (!enm->desc[ii]) {
        if (err) {
          snprintf(err, ASL, "%s(%s): enm->desc[%u] NULL", me, enm->name, ii);
        }
        return 1;
      }
      /* we don't really care about slen, but learning it will
         hopefully produce some memory error if the array is not valid */
      slen = airStrlen(enm->desc[ii]);
      if (!( slen > 0 )) {
        if (err) {
          snprintf(err, ASL, "%s(%s): enm->desc[%u] empty", me, enm->name, ii);
        }
        return 1;
      }
    }
  }
  if (enm->strEqv) {
    /* the strEqv array is one of the easiest ways to mess up an
       airEnum definition; it deserves these tests and maybe more */
    if (!enm->valEqv) {
      if (err) {
        snprintf(err, ASL, "%s(%s): non-NULL strEqv but NULL valEqv",
                 me, enm->name);
      }
      return 1;
    }
    if (!( airStrlen(enm->strEqv[0]) )) {
      if (err) {
        snprintf(err, ASL, "%s(%s): strEqv[0] is NULL or empty",
                 me, enm->name);
      }
      return 1;
    }
    /* check length and see if any string maps to an invalid value */
    for (ii=0; (slen = strlen(enm->strEqv[ii])); ii++) {
      if (!( slen <= AIR_STRLEN_SMALL-1 )) {
        if (err) {
          char stmp[AIR_STRLEN_SMALL];
          snprintf(err, ASL, "%s(%s): strlen(enm->strEqv[%u] \"%s\") "
                   "%s not <= %u", me, enm->name, ii, enm->strEqv[ii],
                   airSprintSize_t(stmp, slen), AIR_STRLEN_SMALL-1);
        }
        return 1;
      }
      /* see if a string maps to an invalid value */
      if (airEnumValCheck(enm, enm->valEqv[ii])) {
        if (err) {
          snprintf(err, ASL, "%s(%s): valEqv[%u] %u (with strEqv[%u] \"%s\")"
                   " not valid",
                   me, enm->name, ii, enm->valEqv[ii], ii, enm->strEqv[ii]);
        }
        return 1;
      }
    }
    /* make sure eqv strings contain the canonical string */
    for (ii=1; ii<=enm->M; ii++) {
      int eval, rval;
      eval = (enm->val ? enm->val[ii] : AIR_CAST(int, ii));
      rval = airEnumVal(enm, enm->str[ii]);
      if (eval != rval) {
        if (err) {
          snprintf(err, ASL, "%s(%s): ii=%u->eval=%d->str=\"%s\"->%d != %d "
                   "(i.e. canonical string didn't map to its own value)",
                   me, enm->name, ii, eval, enm->str[ii], rval, eval);
        }
        return 1;
      }
    }
    /* make sure there are no duplicates among the strEqv,
       including remapping the case in case of case insensitivity */
    for (ii=0; strlen(enm->strEqv[ii]); ii++) {
      for (jj=ii+1; strlen(enm->strEqv[jj]); jj++) {
        if (!strcmp(enm->strEqv[ii], enm->strEqv[jj])) {
          if (err) {
            snprintf(err, ASL, "%s(%s): strEqv[%d] and [%u] both \"%s\"",
                     me, enm->name, ii, jj, enm->strEqv[jj]);
          }
          return 1;
        }
        if (!enm->sense) {
          char bb1[AIR_STRLEN_SMALL], bb2[AIR_STRLEN_SMALL];
          strcpy(bb1, enm->strEqv[ii]);
          airToLower(bb1);
          strcpy(bb2, enm->strEqv[jj]);
          airToLower(bb2);
          if (!strcmp(bb1, bb2)) {
            if (err) {
              snprintf(err, ASL, "%s(%s): after case-lowering, "
                       "strEqv[%d] and [%u] both \"%s\"",
                       me, enm->name, ii, jj, bb1);
            }
            return 1;
          }
        }
      }
    }
  }
  return 0;
}
/* ---- END non-NrrdIO */
/* this is the end */
