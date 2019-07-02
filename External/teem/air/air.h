/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2010 Thomas Schultz
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

#ifndef AIR_HAS_BEEN_INCLUDED
#define AIR_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <float.h>
/* ---- BEGIN non-NrrdIO */
#include <stddef.h>      /* for ptrdiff_t */
/* ---- END non-NrrdIO */

/*
******** TEEM_VERSION
**
** TEEM_VERSION is a single (decimal) number that will always increase
** monotically, and the _MAJOR, _MINOR, _PATCH are also numbers that
** can be used to implement pre-processor logic about specifc
** versions.  The TEEM_VERSION_STRING is used in the (existing) char
** *airTeemVersion (added in version 1.9.0).  Yes, keeping these in
** sync is currently a manual operation.
**
** NOTE: Significant API changes (aside from API additions) should NOT
** occur with changes in patch level, only with major or minor version
** changes.
**
** NOTE: ../../CMakeLists.txt's Teem_VERSION variables must be in sync
*/
#define TEEM_VERSION_MAJOR       1   /* must be 1 digit */
#define TEEM_VERSION_MINOR      11   /* 1 or 2 digits */
#define TEEM_VERSION_PATCH      00   /* 1 or 2 digits */
#define TEEM_VERSION         11100   /* must be 5 digits, to facilitate
                                        easy numerical comparison */
#define TEEM_VERSION_STRING "1.11.0" /* cannot be so easily compared */

/* NrrdIO-hack-000 */

#ifdef __cplusplus
extern "C" {
#endif

/* NrrdIO-hack-001 */
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(air_EXPORTS) || defined(teem_EXPORTS)
#    define AIR_EXPORT extern __declspec(dllexport)
#  else
#    define AIR_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define AIR_EXPORT extern
#endif

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
typedef signed __int64 airLLong;
typedef unsigned __int64 airULLong;
#  define AIR_LLONG_FMT "%I64d"
#  define AIR_ULLONG_FMT "%I64u"
#  define AIR_LLONG(x) x##i64
#  define AIR_ULLONG(x) x##ui64
#else
typedef signed long long airLLong;
typedef unsigned long long airULLong;
#  define AIR_LLONG_FMT "%lld"
#  define AIR_ULLONG_FMT "%llu"
#  define AIR_LLONG(x) x##ll
#  define AIR_ULLONG(x) x##ull
#endif

/* ---- BEGIN non-NrrdIO */
/* This is annoying, thanks to windows */
#define AIR_PI 3.14159265358979323846
#define AIR_E  2.71828182845904523536
/* ---- END non-NrrdIO */

/*
** These serve as conservative estimates on how large various strings
** might end up being.  It would be theoretically better to completely
** avoid the use of fixed-size buffers, but in many contexts the
** implementation complexity of handling them reliably is distracts
** from more urgent implementation goals.  In the mean time, these can
** be used safely as long as the lengths are used consistently.
**
** The possibly unfortunate convention that has become established in
** Teem is code using these tends to NOT add the "+1", to explicitly
** indicate the space for 0-termination, and instead assumes it is
** part of the numbers below, even though this is at the cost of
** confusion about how the maximal strlen() will be less than each of
** these numbers. This will be addressed in Teem 2.0.
*/
#define AIR_STRLEN_SMALL (128+1) /* has to be big enough to hold:
                                  - printed value of size_t and ptrdiff_t,
                                  - line of text that should contain file
                                    format "magic"
                                  */
#define AIR_STRLEN_MED   (256+1)
#define AIR_STRLEN_LARGE (512+1)
#define AIR_STRLEN_HUGE (1024+1) /* has to be big enough to hold
                                    a biff error message (one line of it) */

/*
******** airPtrPtrUnion
**
** union of addresses of pointers to various types, to deal with strict
** aliasing warnings, especially with the first argument to airArrayNew().
** Unfortunately this can't meet the needs of all such cases because some
** libraries need to manage addresses of arrays of other kinds of
** library-specific objects (about which air is ignorant).
*/
typedef union {
  unsigned char **uc;
  signed char **sc;
  char **c;
  char ***cp;
  unsigned short **us;
  short **s;
  unsigned int **ui;
  int **i;
  float **f;
  double **d;
  void **v;
} airPtrPtrUnion;

/*
******** airEnum struct
**
** The airEnum provides the basic mechanism of mapping from a
** string to an int enum value, and back.
*/
typedef struct {
  const char *name;
               /* what are these things? */
  unsigned int M;
               /* str[0]: string for the unknown/invalid value;
                * str[1] .. str[M]: canonical strings for the enum values;
                * "val" NULL: unknown/invalid = 0;
                *             valid values are 1 .. M
                * "val" non-NULL: unknown/invalid = val[0];
                *                 valid are val[1].. val[M]
                */
  const char **str;
               /* see above */
  const int *val;
               /* see above */
  const char **desc;
               /* desc[i] is a short description of the enum values represented
                  by str[i] (thereby starting with the unknown value), to be
                  used to by things like hest */
  const char **strEqv;
               /* If non-NULL, all the variations in strings recognized in
                  mapping from string to value (the values in valEqv).
                  This **MUST** be terminated by a zero-length string ("") so
                  as to signify the end of the list.  This should *not*
                  contain the string for unknown/invalid.
                  If "strEqv" is NULL, then mapping from string to value is
                  done only by traversing "str", and "valEqv" is ignored. */
  const int *valEqv;
               /* If strEqv non-NULL, valEqv holds the values corresponding
                  to the strings in strEqv, with one integer for each
                  non-zero-length string in strEqv: strEqv[i] is a valid
                  string representation for value valEqv[i]. This should *not*
                  contain the value for unknown/invalid.
                  This "valEqv" is ignored if "strEqv" is NULL. */
  int sense;   /* require case matching on strings */
} airEnum;
AIR_EXPORT int airEnumUnknown(const airEnum *enm);
AIR_EXPORT int airEnumValCheck(const airEnum *enm, int val);
AIR_EXPORT const char *airEnumStr(const airEnum *enm, int val);
AIR_EXPORT const char *airEnumDesc(const airEnum *enm, int val);
AIR_EXPORT int airEnumVal(const airEnum *enm, const char *str);
AIR_EXPORT char *airEnumFmtDesc(const airEnum *enm, int val, int canon,
                                const char *fmt);
AIR_EXPORT void airEnumPrint(FILE *file, const airEnum *enm);
/* ---- BEGIN non-NrrdIO */
AIR_EXPORT int airEnumCheck(char err[AIR_STRLEN_LARGE], const airEnum *enm);
/* ---- END non-NrrdIO */

/*
******** airEndian enum
**
** for identifying how a file was written to disk, for those encodings
** where the raw data on disk is dependent on the endianness of the
** architecture.
*/
enum {
  airEndianUnknown,         /* 0: nobody knows */
  airEndianLittle = 1234,   /* 1234: Intel and friends */
  airEndianBig = 4321,      /* 4321: the rest */
  airEndianLast
};
/* endianAir.c */
AIR_EXPORT const airEnum *const airEndian;
AIR_EXPORT int airMyEndian(void);

/* array.c: poor-man's dynamically resizable arrays */
typedef struct {
  void *data,         /* where the data is */
    **dataP;          /* (possibly NULL) address of user's data variable,
                         kept in sync with internal "data" variable */
  unsigned int len,   /* length of array: # units for which there is
                         considered to be data (which is <= total # units
                         allocated).  The # bytes which contain data is
                         len*unit.  Always updated (unlike "*lenP") */
    *lenP,            /* (possibly NULL) address of user's length variable,
                         kept in sync with internal "len" variable */
    incr,             /* the granularity of the changes in amount of space
                         allocated: when the length reaches a multiple of
                         "incr", then the array is resized */
    size;             /* array is allocated to have "size" increments, or,
                         size*incr elements, or,
                         size*incr*unit bytes */
  size_t unit;        /* the size in bytes of one element in the array */
  int noReallocWhenSmaller;  /* as it says */

  /* the following are all callbacks useful for maintaining either an array
     of pointers (allocCB and freeCB) or array of structs (initCB and
     doneCB).  allocCB or initCB is called when the array length increases,
     and freeCB or doneCB when it decreases.  Any of them can be NULL if no
     such activity is desired.  allocCB sets values in the array (as in
     storing the return from malloc(); freeCB is called on values in the
     array (as in calling free()), and the values are cast to void*.  allocCB
     and freeCB don't care about the value of "unit" (though perhaps they
     should).  initCB and doneCB are called on the _addresses_ of elements in
     the array.  allocCB and initCB are called for the elements in ascending
     order in the array, and freeCB and doneCB are called in descending
     order.  allocCB and initCB are mutually exclusive- they can't both be
     non-NULL. Same goes for freeCB and doneCB */
  void *(*allocCB)(void);  /* values of new elements set to return of this */
  void *(*freeCB)(void *); /* called on the values of invalidated elements */
  void (*initCB)(void *);  /* called on addresses of new elements */
  void (*doneCB)(void *);  /* called on addresses of invalidated elements */

} airArray;
AIR_EXPORT airArray *airArrayNew(void **dataP, unsigned int *lenP, size_t unit,
                                 unsigned int incr);
AIR_EXPORT void airArrayStructCB(airArray *a, void (*initCB)(void *),
                                 void (*doneCB)(void *));
AIR_EXPORT void airArrayPointerCB(airArray *a, void *(*allocCB)(void),
                                  void *(*freeCB)(void *));
AIR_EXPORT void airArrayLenSet(airArray *a, unsigned int newlen);
/* ---- BEGIN non-NrrdIO */
AIR_EXPORT void airArrayLenPreSet(airArray *a, unsigned int newlen);
/* ---- END non-NrrdIO */
AIR_EXPORT unsigned int airArrayLenIncr(airArray *a, int delta);
AIR_EXPORT airArray *airArrayNix(airArray *a);
AIR_EXPORT airArray *airArrayNuke(airArray *a);

/* ---- BEGIN non-NrrdIO */

/* heap.c: a (mostly) standard binary min-heap, built on top of airArray.
 * Additional non-standard functionality includes storing additional
 * data and addressing elements that are not at the top of the heap */

typedef struct {
  airArray *key_a; /* where the keys are */
  airArray *data_a; /* where the data is */
  airArray *idx_a; /* indices */
  airArray *invidx_a; /* inverse indices, to access arbitrary elements */
  double *key;
  void *data;
  unsigned int *idx;
  unsigned int *invidx;
} airHeap;

AIR_EXPORT airHeap *airHeapNew(size_t dataUnit, unsigned int incr);
AIR_EXPORT airHeap *airHeapFromArray(const airArray *key, const airArray *data);
AIR_EXPORT airHeap *airHeapNix(airHeap *h);

AIR_EXPORT unsigned int airHeapLength(const airHeap *h);
AIR_EXPORT unsigned int airHeapInsert(airHeap *h, double key, const void *data);
AIR_EXPORT unsigned int airHeapMerge(airHeap *first, const airHeap *second);
AIR_EXPORT double airHeapFrontPeek(const airHeap *h, void *data);
AIR_EXPORT double airHeapFrontPop(airHeap *h, void *data);
AIR_EXPORT int airHeapFrontUpdate(airHeap *h, double newKey,
                                  const void *newData);

AIR_EXPORT int airHeapFind(const airHeap *h, unsigned int *ai,
                           const void *data);
AIR_EXPORT int airHeapRemove(airHeap *h, unsigned int ai);
AIR_EXPORT int airHeapUpdate(airHeap *h, unsigned int ai,
                             double newKey, const void *newData);

/* threadAir.c: simplistic wrapper functions for multi-threading  */
/*
********  airThreadCapable
**
** if non-zero: we have some kind of multi-threading available, either
** via pthreads, or via Windows stuff
*/
AIR_EXPORT const int airThreadCapable;

/*
******** airThreadNoopWarning
**
** When multi-threading is not available, and hence constructs like
** mutexes are not available, the operations on them will be
** no-ops. When this variable is non-zero, we fprintf(stderr) a
** warning to this effect when those constructs are used
*/
AIR_EXPORT int airThreadNoopWarning;

/* opaque typedefs for OS-specific stuff */
typedef struct _airThread airThread;
typedef struct _airThreadMutex airThreadMutex;
typedef struct _airThreadCond airThreadCond;
typedef struct {
  unsigned int numUsers, numDone;
  airThreadMutex *doneMutex;
  airThreadCond *doneCond;
} airThreadBarrier;

AIR_EXPORT airThread *airThreadNew(void);
AIR_EXPORT int airThreadStart(airThread *thread,
                              void *(*threadBody)(void *), void *arg);
AIR_EXPORT int airThreadJoin(airThread *thread, void **retP);
AIR_EXPORT airThread *airThreadNix(airThread *thread);

AIR_EXPORT airThreadMutex *airThreadMutexNew(void);
AIR_EXPORT int airThreadMutexLock(airThreadMutex *mutex);
AIR_EXPORT int airThreadMutexUnlock(airThreadMutex *mutex);
AIR_EXPORT airThreadMutex *airThreadMutexNix(airThreadMutex *mutex);

AIR_EXPORT airThreadCond *airThreadCondNew(void);
AIR_EXPORT int airThreadCondWait(airThreadCond *cond, airThreadMutex *mutex);
AIR_EXPORT int airThreadCondSignal(airThreadCond *cond);
AIR_EXPORT int airThreadCondBroadcast(airThreadCond *cond);
AIR_EXPORT airThreadCond *airThreadCondNix(airThreadCond *cond);

AIR_EXPORT airThreadBarrier *airThreadBarrierNew(unsigned numUsers);
AIR_EXPORT int airThreadBarrierWait(airThreadBarrier *barrier);
AIR_EXPORT airThreadBarrier *airThreadBarrierNix(airThreadBarrier *barrier);

/* ---- END non-NrrdIO */

/*
******** airFP enum
**
** the different kinds of floating point number afforded by IEEE 754,
** and the values returned by airFPClass_f().
**
** The values probably won't agree with those in #include's like
** ieee.h, ieeefp.h, fp_class.h.  This is because IEEE 754 hasn't
** defined standard values for these, so everyone does it differently.
**
** This enum uses underscores (against Teem convention) to help
** legibility while also conforming to the spirit of the somewhat
** standard naming conventions
*/
enum {
  airFP_Unknown,               /*  0: nobody knows */
  airFP_SNAN,                  /*  1: signalling NaN */
  airFP_QNAN,                  /*  2: quiet NaN */
  airFP_POS_INF,               /*  3: positive infinity */
  airFP_NEG_INF,               /*  4: negative infinity */
  airFP_POS_NORM,              /*  5: positive normalized non-zero */
  airFP_NEG_NORM,              /*  6: negative normalized non-zero */
  airFP_POS_DENORM,            /*  7: positive denormalized non-zero */
  airFP_NEG_DENORM,            /*  8: negative denormalized non-zero */
  airFP_POS_ZERO,              /*  9: +0.0, positive zero */
  airFP_NEG_ZERO,              /* 10: -0.0, negative zero */
  airFP_Last                   /* after the last valid one */
};
/* 754.c: IEEE-754 related stuff values */
typedef union {
  unsigned int i;
  float f;
} airFloat;
typedef union {
  airULLong i;
  double d;
} airDouble;
AIR_EXPORT const int airMyQNaNHiBit;
AIR_EXPORT float airFPPartsToVal_f(unsigned int sign,
                                   unsigned int expo,
                                   unsigned int mant);
AIR_EXPORT void airFPValToParts_f(unsigned int *signP,
                                  unsigned int *expoP,
                                  unsigned int *mantP, float v);
AIR_EXPORT double airFPPartsToVal_d(unsigned int sign,
                                    unsigned int expo,
                                    unsigned int mant0,
                                    unsigned int mant1);
AIR_EXPORT void airFPValToParts_d(unsigned int *signP,
                                  unsigned int *expoP,
                                  unsigned int *mant0P,
                                  unsigned int *mant1P,
                                  double v);
AIR_EXPORT float airFPGen_f(int cls);
AIR_EXPORT double airFPGen_d(int cls);
AIR_EXPORT int airFPClass_f(float val);
AIR_EXPORT int airFPClass_d(double val);
AIR_EXPORT void airFPFprintf_f(FILE *file, float val);
AIR_EXPORT void airFPFprintf_d(FILE *file, double val);
AIR_EXPORT const airFloat airFloatQNaN;
AIR_EXPORT const airFloat airFloatSNaN;
AIR_EXPORT const airFloat airFloatPosInf;
AIR_EXPORT const airFloat airFloatNegInf;
AIR_EXPORT float airNaN(void);
AIR_EXPORT int airIsNaN(double d);
AIR_EXPORT int airIsInf_f(float f);
AIR_EXPORT int airIsInf_d(double d);
AIR_EXPORT int airExists(double d);

/* ---- BEGIN non-NrrdIO */

#define AIR_RANDMT_N 624
typedef struct {
  /* These need to be at least 32 bits */
  unsigned int state[AIR_RANDMT_N], /* internal state */
    *pNext,                         /* next value to get from state */
    left;                           /* number of values left before
                                       reload needed */
} airRandMTState;
/* randMT.c */
AIR_EXPORT airRandMTState *airRandMTStateGlobal;
AIR_EXPORT void airRandMTStateGlobalInit(void);
AIR_EXPORT airRandMTState *airRandMTStateNew(unsigned int seed);
AIR_EXPORT airRandMTState *airRandMTStateNix(airRandMTState *state);
AIR_EXPORT void airSrandMT_r(airRandMTState *state, unsigned int seed);
AIR_EXPORT double airDrandMT_r(airRandMTState *state);       /* [0,1] */
AIR_EXPORT unsigned int airUIrandMT_r(airRandMTState *state);
AIR_EXPORT double airDrandMT53_r(airRandMTState *state);     /* [0,1) */
AIR_EXPORT unsigned int airRandInt(unsigned int N);
AIR_EXPORT unsigned int airRandInt_r(airRandMTState *state, unsigned int N);
AIR_EXPORT void airSrandMT(unsigned int seed);
AIR_EXPORT double airDrandMT(void);
AIR_EXPORT int airRandMTSanity(void);

/* ---- END non-NrrdIO */

/*
******** airType
**
** Different types which air cares about.
** Currently only used in the command-line parsing, but perhaps will
** be used elsewhere in air later
*/
enum {
  airTypeUnknown,   /*  0 */
  airTypeBool,      /*  1 */
  airTypeInt,       /*  2 */
  airTypeUInt,      /*  3 */
  airTypeLongInt,   /*  4 */
  airTypeULongInt,  /*  5 */
  airTypeSize_t,    /*  6 */
  airTypeFloat,     /*  7 */
  airTypeDouble,    /*  8 */
  airTypeChar,      /*  9 */
  airTypeString,    /* 10 */
  airTypeEnum,      /* 11 */
  airTypeOther,     /* 12 */
  airTypeLast
};
#define AIR_TYPE_MAX   12
/* parseAir.c */
AIR_EXPORT double airAtod(const char *str);
AIR_EXPORT int airSingleSscanf(const char *str, const char *fmt, void *ptr);
AIR_EXPORT const airEnum *const airBool;
AIR_EXPORT unsigned int airParseStrB(int *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrI(int *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrUI(unsigned int *out, const char *s,
                                      const char *ct, unsigned int n,
                                      ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrZ(size_t *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrF(float *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrD(double *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrC(char *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrS(char **out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* REQ'D even if n>1: int greedy */);
AIR_EXPORT unsigned int airParseStrE(int *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* REQUIRED: airEnum *e */);
AIR_EXPORT unsigned int (*airParseStr[AIR_TYPE_MAX+1])(void *, const char *,
                                                       const char *,
                                                       unsigned int, ...);

/* string.c */
AIR_EXPORT char *airStrdup(const char *s);
AIR_EXPORT size_t airStrlen(const char *s);
/* ---- BEGIN non-NrrdIO */
AIR_EXPORT int airStrcmp(const char *s1, const char *s2);
/* ---- END non-NrrdIO */
AIR_EXPORT int airStrtokQuoting;
AIR_EXPORT char *airStrtok(char *s, const char *ct, char **last);
AIR_EXPORT unsigned int airStrntok(const char *s, const char *ct);
AIR_EXPORT char *airStrtrans(char *s, char from, char to);
AIR_EXPORT char *airStrcpy(char *dst, size_t dstSize, const char *src);
AIR_EXPORT int airEndsWith(const char *s, const char *suff);
AIR_EXPORT char *airUnescape(char *s);
AIR_EXPORT char *airOneLinify(char *s);
AIR_EXPORT char *airToLower(char *str);
AIR_EXPORT char *airToUpper(char *str);
AIR_EXPORT unsigned int airOneLine(FILE *file, char *line, unsigned int size);

/* sane.c */
/*
******** airInsane enum
**
** reasons for why airSanity() failed (specifically, the possible
** return values for airSanity()
*/
enum {
  airInsane_not,           /*  0: actually, all sanity checks passed */
  airInsane_endian,        /*  1: airMyEndian is wrong */
  airInsane_pInfExists,    /*  2: AIR_EXISTS(positive infinity) was true */
  airInsane_nInfExists,    /*  3: AIR_EXISTS(negative infinity) was true */
  airInsane_NaNExists,     /*  4: AIR_EXISTS(NaN) was true */
  airInsane_FltDblFPClass, /*  5: double -> float assignment messed up the
                               airFPClass_f() of the value */
  airInsane_QNaNHiBit,     /*  6: airMyQNaNHiBit is wrong */
  airInsane_AIR_NAN,       /*  7: airFPClass_f(AIR_QNAN) wrong
                                  (no longer checking on problematic SNAN) */
  airInsane_dio,           /*  8: airMyDio set to something invalid */
  airInsane_UCSize,        /*  9: unsigned char isn't 8 bits */
  airInsane_FISize,        /* 10: sizeof(float), sizeof(int) not 4 */
  airInsane_DLSize         /* 11: sizeof(double), sizeof(airLLong) not 8 */
};
#define AIR_INSANE_MAX        11
AIR_EXPORT const char *airInsaneErr(int insane);
AIR_EXPORT int airSanity(void);

/* miscAir.c */
AIR_EXPORT const char *airTeemVersion;
AIR_EXPORT const char *airTeemReleaseDate;
AIR_EXPORT void *airNull(void);
AIR_EXPORT void *airSetNull(void **ptrP);
AIR_EXPORT void *airFree(void *ptr);
AIR_EXPORT FILE *airFopen(const char *name, FILE *std, const char *mode);
AIR_EXPORT FILE *airFclose(FILE *file);
AIR_EXPORT int airSinglePrintf(FILE *file, char *str, const char *fmt, ...);
AIR_EXPORT char *airSprintSize_t(char str[AIR_STRLEN_SMALL], size_t val);
/* ---- BEGIN non-NrrdIO */
AIR_EXPORT char *airSprintVecSize_t(char *str, const size_t *vec,
                                    unsigned int len);
AIR_EXPORT char *airPrettySprintSize_t(char str[AIR_STRLEN_SMALL], size_t v);
AIR_EXPORT char *airSprintPtrdiff_t(char str[AIR_STRLEN_SMALL], ptrdiff_t v);
AIR_EXPORT const int airPresent;
AIR_EXPORT FILE *airStderr(void);
AIR_EXPORT FILE *airStdout(void);
AIR_EXPORT FILE *airStdin(void);
AIR_EXPORT unsigned int airIndex(double min, double val, double max,
                                 unsigned int N);
AIR_EXPORT unsigned int airIndexClamp(double min, double val, double max,
                                      unsigned int N);
AIR_EXPORT airULLong airIndexULL(double min, double val, double max,
                                 airULLong N);
AIR_EXPORT airULLong airIndexClampULL(double min, double val, double max,
                                      airULLong N);
AIR_EXPORT char *airDoneStr(double start, double here, double end, char *str);
AIR_EXPORT double airTime(void);
AIR_EXPORT const char airTypeStr[AIR_TYPE_MAX+1][AIR_STRLEN_SMALL];
AIR_EXPORT const size_t airTypeSize[AIR_TYPE_MAX+1];
AIR_EXPORT void airEqvAdd(airArray *eqvArr, unsigned int j, unsigned int k);
AIR_EXPORT unsigned int airEqvMap(airArray *eqvArr,
                                  unsigned int *map, unsigned int len);
AIR_EXPORT unsigned int airEqvSettle(unsigned int *map, unsigned int len);


/* math.c */
AIR_EXPORT double airFastExp(double val);
AIR_EXPORT double airExp(double val);
AIR_EXPORT void airNormalRand(double *z1, double *z2);
AIR_EXPORT void airNormalRand_r(double *z1, double *z2,
                                airRandMTState *state);
AIR_EXPORT void airShuffle(unsigned int *buff, unsigned int N, int perm);
AIR_EXPORT void airShuffle_r(airRandMTState *state,
                             unsigned int *buff, unsigned int N,
                             int perm);
AIR_EXPORT double airCbrt(double);
AIR_EXPORT double airMode3(double a, double b, double c);
AIR_EXPORT double airMode3_d(const double v[3]);
AIR_EXPORT double airSgnPow(double, double);
AIR_EXPORT double airFlippedSgnPow(double, double);
AIR_EXPORT double airIntPow(double v, int p);
AIR_EXPORT int airSgn(double);
AIR_EXPORT int airLog2(size_t n);
AIR_EXPORT double airErfc(double x);
AIR_EXPORT double airErf(double x);
AIR_EXPORT double airGaussian(double x, double mean, double stdv);
AIR_EXPORT double airBesselI0(double x);
AIR_EXPORT double airBesselI1(double x);
AIR_EXPORT double airBesselI0ExpScaled(double x);
AIR_EXPORT double airBesselI1ExpScaled(double x);
AIR_EXPORT double airLogBesselI0(double x);
AIR_EXPORT double airLogRician(double mes, double tru, double sig);
AIR_EXPORT double airRician(double mes, double tru, double sig);
AIR_EXPORT double airBesselI1By0(double x);
AIR_EXPORT double airBesselIn(int n, double x);
AIR_EXPORT double airBesselInExpScaled(int n, double x);
AIR_EXPORT double airVanDerCorput(unsigned int indx, unsigned int base);
AIR_EXPORT void airHalton(double *out, unsigned int indx,
                          const unsigned int *base, unsigned int num);
#define AIR_PRIME_NUM 1000
AIR_EXPORT const unsigned int airPrimeList[AIR_PRIME_NUM];
AIR_EXPORT unsigned int airCRC32(const unsigned char *data, size_t len,
                                 size_t unit, int swap);
/* ---- END non-NrrdIO */

/* dio.c */
/*
******** airNoDio enum
**
** reasons for why direct I/O won't be used with a particular
** file/pointer combination
*/
enum {
  airNoDio_okay,    /*  0: actually, you CAN do direct I/O */
  airNoDio_arch,    /*  1: Teem thinks this architecture can't do it */
  airNoDio_format,  /*  2: Teem thinks given data file format can't use it */
  airNoDio_std,     /*  3: DIO isn't possible for std{in|out|err} */
  airNoDio_fd,      /*  4: couldn't get underlying file descriptor */
  airNoDio_dioinfo, /*  5: calling fcntl() to get direct I/O info failed */
  airNoDio_small,   /*  6: requested size is too small */
  airNoDio_size,    /*  7: requested size not a multiple of d_miniosz */
  airNoDio_ptr,     /*  8: pointer not multiple of d_mem */
  airNoDio_fpos,    /*  9: current file position not multiple of d_miniosz */
  airNoDio_setfl,   /* 10: fcntl(fd, SETFL, FDIRECT) failed */
  airNoDio_test,    /* 11: couldn't memalign() even a small bit of memory */
  airNoDio_disable  /* 12: someone disabled it with airDisableDio */
};
#define AIR_NODIO_MAX  12
AIR_EXPORT const char *airNoDioErr(int noDio);
AIR_EXPORT const int airMyDio;
AIR_EXPORT int airDisableDio;
AIR_EXPORT void airDioInfo(int *align, int *min, int *max, int fd);
AIR_EXPORT int airDioTest(int fd, const void *ptr, size_t size);
AIR_EXPORT void *airDioMalloc(size_t size, int fd);
AIR_EXPORT size_t airDioRead(int fd, void *ptr, size_t size);
AIR_EXPORT size_t airDioWrite(int fd, const void *ptr, size_t size);

/* mop.c: clean-up utilities */
enum {
  airMopNever,
  airMopOnError,
  airMopOnOkay,
  airMopAlways
};
typedef void *(*airMopper)(void *);
typedef struct {
  void *ptr;         /* the thing to be processed */
  airMopper mop;     /* the function to which does the processing */
  int when;          /* from the airMopWhen enum */
} airMop;
AIR_EXPORT airArray *airMopNew(void);
AIR_EXPORT int airMopAdd(airArray *arr, void *ptr, airMopper mop, int when);
AIR_EXPORT void airMopSub(airArray *arr, void *ptr, airMopper mop);
AIR_EXPORT void airMopMem(airArray *arr, void *_ptrP, int when);
AIR_EXPORT void airMopUnMem(airArray *arr, void *_ptrP);
AIR_EXPORT void airMopPrint(airArray *arr, const void *_str, int when);
AIR_EXPORT void airMopDone(airArray *arr, int error);
AIR_EXPORT void airMopError(airArray *arr);
AIR_EXPORT void airMopOkay(airArray *arr);
AIR_EXPORT void airMopDebug(airArray *arr);
/* ---- BEGIN non-NrrdIO */
AIR_EXPORT void airMopSingleDone(airArray *arr, void *ptr, int error);
AIR_EXPORT void airMopSingleError(airArray *arr, void *ptr);
AIR_EXPORT void airMopSingleOkay(airArray *arr, void *ptr);
/* ---- END non-NrrdIO */


/*******     the interminable sea of defines and macros     *******/

#define AIR_TRUE 1
#define AIR_FALSE 0
#define AIR_WHITESPACE " \t\n\r\v\f"       /* K+R pg. 157 */

/*
******** AIR_UNUSED
**
** one way of reconciling "warning: unused parameter" with
** C's "error: parameter name omitted"
*/
#define AIR_UNUSED(x) (void)(x)

/*
******** AIR_CAST, AIR_UINT, AIR_INT
**
** just casts, but with the added ability to grep for them more easily,
** since casts should probably always be revisited and reconsidered.
*/
#define AIR_CAST(t, v) ((t)(v))
#define AIR_UINT(x) AIR_CAST(unsigned int, x)
#define AIR_INT(x) AIR_CAST(int, x)

/*
******** AIR_VOIDP, AIR_CVOIDP
**
** explicit casting to "void *" (and "const void *") from non-void* pointers
** is strictly speaking needed for the %p format specifier in printf-like
** functions; this is a slightly more convenient form
*/
#define AIR_VOIDP(x) AIR_CAST(void *, x)
#define AIR_CVOIDP(x) AIR_CAST(const void *, x)

/*
******** AIR_MALLOC, AIR_CALLOC
**
** slightly simpler wrapper around cast and malloc/calloc
**
** HEY note that "T" is not guarded by parentheses in its first usage,
** as arguments in Teem macros normally are
*/
#define AIR_MALLOC(N, T) (T*)(malloc((N)*sizeof(T)))
#define AIR_CALLOC(N, T) (T*)(calloc((N), sizeof(T)))

/*
******** AIR_ENDIAN, AIR_QNANHIBIT, AIR_DIO
**
** These reflect particulars of hardware which we're running on. The
** difference from the things starting with TEEM_ is that the TEEM_
** values are for passing architecture-specific to compilation of source
** files, and thes AIR_ variables are for advertising that information
** to anyone linking against air (or Teem) and including air.h.
*/
#define AIR_ENDIAN (airMyEndian())
#define AIR_QNANHIBIT (airMyQNaNHiBit)
#define AIR_DIO (airMyDio)

/*
******** AIR_NAN, AIR_QNAN, AIR_SNAN, AIR_POS_INF, AIR_NEG_INF
**
** its nice to have these values available without the cost of a
** function call.
**
** NOTE: AIR_POS_INF and AIR_NEG_INF correspond to the _unique_
** bit-patterns which signify positive and negative infinity.  With
** the NaNs, however, they are only one of many possible
** representations.
*/
#define AIR_NAN  (airFloatQNaN.f)
#define AIR_QNAN (airFloatQNaN.f)
#define AIR_SNAN (airFloatSNaN.f)
#define AIR_POS_INF (airFloatPosInf.f)
#define AIR_NEG_INF (airFloatNegInf.f)

/*
******** AIR_EXISTS
**
** is non-zero (true) only for values which are not NaN or +/-infinity
**
** You'd think that (x == x) might work, but no no no, some optimizing
** compilers (e.g. SGI's cc) say "well of course they're equal, for all
** possible values".  Bastards!
**
** One of the benefits of IEEE 754 floating point numbers is that
** gradual underflow means that x = y <==> x - y = 0 for any (positive
** or negative) normalized or denormalized float.  Otherwise this
** macro could not be valid; some floating point conventions say that
** a zero-valued exponent means zero, regardless of the mantissa.
**
** However, there MAY be problems on machines which use extended
** (80-bit) floating point registers, such as Intel chips- where the
** same initial value 1) directly read from the register, versus 2)
** saved to memory and loaded back, may end up being different.  I
** have yet to produce this behavior, or convince myself it can't happen.
**
** The reason to #define AIR_EXISTS as airExists is that on some
** optimizing compilers, the !((x) - (x)) doesn't work.  This has been
** the case on Windows and 64-bit irix6 (64 bit) with -Ofast.  If
** airSanity fails because a special value "exists", then use the
** first version of AIR_EXISTS.
**
** There is a performance consequence of using airExists(x), in that it
** is a function call, although (HEY) we should facilitate inline'ing it
** for compilers that know how to.
**
** gcc 4.5.3 -std=c89, at least on cygwin, has problems with
** the type of "!((x) - (x))" when used with bit-wise xor ^, saying
** "invalid operands to binary ^ (have ‘int’ and ‘int’)" but these
** problems oddly went away with the explicit cast to int.
*/
#if defined(_WIN32) || defined(__ECC) /* NrrdIO-hack-002 */
#define AIR_EXISTS(x) (airExists(x))
#else
#define AIR_EXISTS(x) (AIR_CAST(int, !((x) - (x))))
#endif

/* ---- BEGIN non-NrrdIO */

/*
******** AIR_EXISTS_F(x)
**
** This is another way to check for non-specialness (not NaN, not
** +inf, not -inf) of a _float_, by making sure the exponent field
** isn't all ones.
**
** Unlike !((x) - (x)) or airExists(x), the argument to this macro
** MUST MUST MUST be a float, and the float must be of the standard
** 32-bit size, which must also be the size of an int.  The reason for
** this constraint is that macros are not functions, so there is no
** implicit cast or conversion to a single type.  Casting the address
** of the macro arg to an int* only works when the arg has the same
** size as an int.
**
** No cross-platform comparative timings have been done to compare the
** speed of !((x) - (x)) versus airExists() versus AIR_EXISTS_F()
**
** This macro is endian-safe.
*/
#define AIR_EXISTS_F(x) ((*(unsigned int*)&(x) & 0x7f800000) != 0x7f800000)

/*
******** AIR_EXISTS_D(x)
**
** like AIR_EXISTS_F(), but the argument here MUST be a double
*/
#define AIR_EXISTS_D(x) (                               \
  (*(airULLong*)&(x) & AIR_ULLONG(0x7ff0000000000000))  \
    != AIR_ULLONG(0x7ff0000000000000))

/*
******** AIR_ISNAN_F(x)
**
** detects if a float is NaN by looking at the bits, without relying on
** any of its arithmetic properties.  As with AIR_EXISTS_F(), this only
** works when the argument really is a float, and when floats are 4-bytes
*/
#define AIR_ISNAN_F(x) (((*(unsigned int*)&(x) & 0x7f800000)==0x7f800000) && \
                         (*(unsigned int*)&(x) & 0x007fffff))

/* ---- END non-NrrdIO */

/*
******** AIR_MAX(a,b), AIR_MIN(a,b), AIR_ABS(a)
**
** the usual
*/
#define AIR_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AIR_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AIR_ABS(a) ((a) > 0.0f ? (a) : -(a))

/*
******** AIR_COMPARE(a,b)
**
** the sort of compare that qsort() wants for ascending sort
*/
#define AIR_COMPARE(a,b) ((a) < (b)     \
                          ? -1          \
                          : ((a) > (b) \
                             ? 1        \
                             : 0))

/*
******** AIR_IN_OP(a,b,c), AIR_IN_CL(a,b,c)
**
** is true if the middle argument is in the open/closed interval
** defined by the first and third arguments
**
** AIR_IN_OP is new name for old AIR_BETWEEN
** AIR_IN_CL is new name for old AIR_INSIDE
*/
#define AIR_IN_OP(a,b,c) ((a) < (b) && (b) < (c))     /* closed interval */
#define AIR_IN_CL(a,b,c) ((a) <= (b) && (b) <= (c))   /* open interval */

/*
******** AIR_CLAMP(a,b,c)
**
** returns the middle argument, after being clamped to the closed
** interval defined by the first and third arguments
*/
#define AIR_CLAMP(a,b,c) ((b) < (a)        \
                           ? (a)           \
                           : ((b) > (c)    \
                              ? (c)        \
                              : (b)))

/*
******** AIR_MOD(i, N)
**
** returns that integer in [0, N-1] which is i plus a multiple of N. It
** may be unfortunate that the expression (i)%(N) appears three times;
** this should be inlined.  Or perhaps the compiler's optimizations
** (common sub-expression elimination) will save us.
**
** Note: integer divisions are not very fast on some modern chips;
** don't go silly using this one.
*/
#define AIR_MOD(i, N) ((i)%(N) >= 0 ? (i)%(N) : N + (i)%(N))

/*
******** AIR_LERP(w, a, b)
**
** returns a when w=0, and b when w=1, and linearly varies in between
*/
#define AIR_LERP(w, a, b) ((w)*((b) - (a)) + (a))

/*
******** AIR_AFFINE(i,x,I,o,O)
**
** given intervals [i,I], [o,O] and a value x which may or may not be
** inside [i,I], return the value y such that y stands in the same
** relationship to [o,O] that x does with [i,I].  Or:
**
**    y - o         x - i
**   -------   =   -------
**    O - o         I - i
**
** It is the callers responsibility to make sure I-i and O-o are
** both non-zero.  Strictly speaking, real problems arise only when
** when I-i is zero: division by zero generates either NaN or infinity
**
** NOTE that "x" is evaluated only once (which makes this more useful),
** as is "I" and "O" (usually not so important); "i" and "o" are each
** evaluated twice
*/
#define AIR_AFFINE(i,x,I,o,O) ( \
((double)(O)-(o))*((double)(x)-(i)) / ((double)(I)-(i)) + (o))

/*
******** AIR_DELTA(i,x,I,o,O)
**
** given intervals [i,I] and [o,O], calculates the number y such that
** a change of x within [i,I] is proportional to a change of y within
** [o,O].  Or:
**
**      y             x
**   -------   =   -------
**    O - o         I - i
**
** It is the callers responsibility to make sure I-i and O-o are
** both non-zero
**
** NOTE that all arguments are evaluated only once
*/
#define AIR_DELTA(i,x,I,o,O) ( \
((double)(O)-(o))*((double)(x)) / ((double)(I)-(i)) )

/*
******** AIR_ROUNDUP, AIR_ROUNDDOWN
**
** rounds integers up or down; just wrappers around floor and ceil
*/
#define AIR_ROUNDUP(x)   ((int)(floor((x)+0.5)))
#define AIR_ROUNDDOWN(x) ((int)(ceil((x)-0.5)))
#define AIR_ROUNDUP_UI(x)   ((unsigned int)(floor((x)+0.5)))
#define AIR_ROUNDDOWN_UI(x) ((unsigned int)(ceil((x)-0.5)))

#ifdef __cplusplus
}
#endif

#endif /* AIR_HAS_BEEN_INCLUDED */
