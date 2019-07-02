/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2010 Thomas Schultz

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
#include <string.h>

/* internal helper routines */
/* restore heap property by pulling a single node up the tree */
static void upheap(airHeap *h, unsigned int i) {
  while (i) {
    unsigned int parent = (i-1)/2;
    if (h->key[h->idx[parent]] > h->key[h->idx[i]]) { /* swap */
      unsigned int tmp = h->idx[parent];
      h->idx[parent] = h->idx[i];
      h->idx[i] = tmp;
      h->invidx[h->idx[i]] = i;
      h->invidx[h->idx[parent]] = parent;
      i = parent;
    } else
      i = 0; /* break loop */
  }
}

/* restore heap property by pushing a single node down the tree */
static void downheap (airHeap *h, unsigned int i) {
  unsigned int len = h->key_a->len;
  while (1) {
    unsigned int left = 2*i+1;
    unsigned int right = 2*i+2;
    unsigned int minidx = 0;
    double minval = 0;
    if (left>=len)
      return;
    if ((right>=len) ||
        (h->key[h->idx[left]] < h->key[h->idx[right]])) {
      minidx = left;
      minval = h->key[h->idx[left]];
    } else {
      minidx = right;
      minval = h->key[h->idx[right]];
    }
    if (h->key[h->idx[i]] > minval) { /* swap */
      unsigned int tmp = h->idx[i];
      h->idx[i] = h->idx[minidx];
      h->idx[minidx] = tmp;
      h->invidx[h->idx[i]] = i;
      h->invidx[h->idx[minidx]] = minidx;
      i = minidx;
    } else
      return;
  }
}

/* Restore heap property "from scratch" (without any prior assumptions).
 * Works in a bottom-up manner, which is more efficient than top-down */
static void heapify (airHeap *h) {
  unsigned int len = h->key_a->len;
  int maxi = len/2-1, i; /* above maxi, downheap has no effect */
  for (i=maxi; i>=0; i--) {
    downheap(h, i);
  }
}

/* Tries to increase or decrease the length of h. returns 0 upon success. */
static int heapLenIncr (airHeap *h, int delta) {
  unsigned int oldlen=h->key_a->len;
  unsigned int newlen=oldlen+delta;
  if (delta==0)
    return 0;
  airArrayLenIncr(h->key_a, delta);
  if (h->data_a!=NULL) airArrayLenIncr(h->data_a, delta);
  airArrayLenIncr(h->idx_a, delta);
  airArrayLenIncr(h->invidx_a, delta);
  if (h->key_a->len<newlen || (h->data_a!=NULL && h->data_a->len<newlen) ||
      h->idx_a->len<newlen || h->invidx_a->len<newlen) {
    /* Error. Try to undo changes and return error code. */
    if (h->key_a->len>oldlen)    airArrayLenSet(h->key_a, oldlen);
    if (h->data_a!=NULL && h->data_a->len>oldlen)
      airArrayLenSet(h->data_a, oldlen);
    if (h->idx_a->len>oldlen)    airArrayLenSet(h->idx_a, oldlen);
    if (h->invidx_a->len>oldlen) airArrayLenSet(h->invidx_a, oldlen);
    return 1;
  }
  return 0;
}

/* Creates a new heap, returning NULL upon error.  If additional data
 * is to be stored with each node, dataUnit needs to be set to the
 * number of data bytes needed per element.  incr is used for dynamic
 * memory allocation (an additional number of incr elements are
 * allocated each time the heap grows past its current capacity).
 */
airHeap *airHeapNew(size_t dataUnit, unsigned int incr) {
  airHeap *h;
  airPtrPtrUnion appu;
  h = AIR_CALLOC(1, airHeap);
  if (h==NULL) {
    return NULL;
  }

  appu.d = &h->key;
  h->key_a = airArrayNew(appu.v, NULL, sizeof(double), incr);
  if (dataUnit>0) { /* data is optional */
    h->data_a = airArrayNew(&h->data, NULL, dataUnit, incr);
  }
  appu.ui = &h->idx;
  h->idx_a = airArrayNew(appu.v, NULL, sizeof(unsigned int), incr);
  appu.ui = &h->invidx;
  h->invidx_a = airArrayNew(appu.v, NULL, sizeof(unsigned int), incr);

  if (h->key_a==NULL || (dataUnit>0 && h->data_a==NULL) || h->idx_a==NULL ||
      h->invidx_a==NULL) { /* allocation failed (partly) */
    airHeapNix(h);
    return NULL;
  }
  return h;
}

/* Same as airHeapNew, but initializes the new heap with the keys from key
 * and the optional data from data. If data is non-NULL, it needs to
 * have the same length as key, or this function will fail. The incr
 * field of the heap is copied from key (rather than data).
 */
airHeap *airHeapFromArray(const airArray *key, const airArray *data) {
  airHeap *h;
  unsigned int i;
  if (key==NULL || (data!=NULL && data->len!=key->len))
    return NULL; /* unusable input */
  h = airHeapNew((data==NULL)?0:data->unit, key->incr);
  if (heapLenIncr (h, key->len)) { /* could not allocate memory */
    airHeapNix(h);
    return NULL;
  }
  memcpy(h->key, key->data, key->len*sizeof(double));
  if (h->data_a!=NULL) memcpy(h->data, data->data, data->len*data->unit);
  for (i=0; i<key->len; i++) { /* initialize indices to identity */
    h->idx[i]=i;
    h->invidx[i]=i;
  }
  heapify(h);
  return h;
}

/* Frees all memory associated with the heap and its data */
airHeap *airHeapNix(airHeap *h) {
  if (h!=NULL) {
    airArrayNuke(h->key_a);
    if (h->data_a!=NULL) airArrayNuke(h->data_a);
    airArrayNuke(h->idx_a);
    airArrayNuke(h->invidx_a);
    free(h);
  }
  return NULL;
}

/* Returns the number of elements that are currently in heap h
 * (or 0 if h==NULL) */
unsigned int airHeapLength(const airHeap *h) {
  if (h!=NULL) {
    return h->key_a->len;
  } else
    return 0;
}

/* Inserts a key into the heap. data is copied over (deep copy) when the
 * heap was initialized to hold additional data. Otherwise, it is ignored.
 *
 * Returns the new number of elements in h.
 * Upon error, this can be the same as the old length. */
unsigned int airHeapInsert(airHeap *h, double key, const void *data) {
  unsigned int oldlen;
  if (h==NULL) return 0;
  oldlen = h->key_a->len;
  /* make space for the new element */
  if (heapLenIncr(h, 1))
    return oldlen;
  h->key[oldlen]=key;
  if (h->data_a!=NULL && data!=NULL) {
    memcpy((char*)h->data_a->data+oldlen*h->data_a->unit, data,
           h->data_a->unit);
  }
  h->idx[oldlen]=oldlen;
  h->invidx[oldlen]=oldlen;
  upheap(h,oldlen); /* restore the heap property */
  return oldlen+1;
}

/* Merges the second heap into the first. Returns the new length of first,
 * or zero upon error. */
unsigned int airHeapMerge(airHeap *first, const airHeap *second) {
  unsigned int first_len, second_len, i;
  if (first==NULL || second==NULL)
    return 0;
  if ((first->data_a==NULL) ^ (second->data_a==NULL))
    return 0; /* incompatible data */
  if (first->data_a!=NULL &&
      (first->data_a->unit!=second->data_a->unit))
    return 0; /* incompatible data */
  /* make sufficient space in first */
  first_len = first->key_a->len;
  second_len = second->key_a->len;
  if (heapLenIncr(first, second_len))
    return 0;
  /* concatenate and heapify */
  memcpy(first->key+first_len, second->key, second_len*sizeof(double));
  if (first->data_a!=NULL)
    memcpy((char*)first->data_a->data+first_len*first->data_a->unit,
           second->data_a->data,second_len*second->data_a->unit);
  for (i=0; i<second_len; i++) {
    first->idx[first_len+i] = first_len+second->idx[i];
    first->invidx[first->idx[first_len+i]]=first_len+i;
  }
  heapify(first);
  return first_len+second_len;
}

/* Returns the key of the front element in the heap and optionally copies the
 * associated data to *data. Does not modify the heap. */
double airHeapFrontPeek(const airHeap *h, void *data) {
  if (h==NULL || h->key_a->len==0)
    return 0.0;
  if (data!=NULL && h->data_a!=NULL)
    memcpy(data, (char*)h->data_a->data+h->idx[0]*h->data_a->unit,
           h->data_a->unit);
  return h->key[h->idx[0]];
}

/* Same as airHeapFrontPeek, but additionally removes the front element. */
double airHeapFrontPop(airHeap *h, void *data) {
  double retval = airHeapFrontPeek(h, data);
  if (h!=NULL && h->key_a->len>0) {
    airHeapRemove(h, h->idx[0]);
  }
  return retval;
}

/* Assigns a new key (and optionally, new data) to the front element and
 * re-sorts the heap if necessary. */
int airHeapFrontUpdate(airHeap *h, double newKey, const void *newData) {
  if (h==NULL || h->key_a->len==0)
    return 1;
  if (newData!=NULL && h->data_a!=NULL)
    memcpy((char*)h->data_a->data+h->idx[0]*h->data_a->unit, newData,
           h->data_a->unit);
  h->key[h->idx[0]]=newKey;
  downheap(h, 0);
  return 0;
}

/* The following functions provide advanced functionality and should
 * not be required in most applications. */

/* airHeapFind returns 0 if an element is found whose data is bitwise
 * equal to the given data, and sets *ai to the index of this
 * element. If more than one element matches the data, an arbitrary
 * one of them is returned.  The index can be used with airHeapRemove
 * or airHeapUpdate, but will become invalid as soon as any element is
 * removed from the heap. */
int airHeapFind(const airHeap *h, unsigned int *ai, const void *data) {
  unsigned int i;
  if (h==NULL || ai==NULL || data==NULL || h->data_a==NULL)
    return 1;
  for (i=0; i<h->key_a->len; i++) {
    if (!memcmp((char*)h->data_a->data+i*h->data_a->unit, data,
                h->data_a->unit)) {
      *ai = i;
      return 0;
    }
  }
  return 1;
}

/* Removes element ai from the heap, returns 0 upon success. */
int airHeapRemove(airHeap *h, unsigned int ai) {
  unsigned int old_invidx_ai;
  if (h==NULL || h->key_a->len<=ai)
    return 1;
  /* in the tree, replace ai with last element */
  old_invidx_ai=h->invidx[ai];
  h->idx[h->invidx[ai]]=h->idx[h->key_a->len-1];
  h->invidx[h->idx[h->key_a->len-1]]=h->invidx[ai];
  /* remove ai - copy last element over, then drop it */
  if (ai!=h->key_a->len-1) {
    h->key[ai]=h->key[h->key_a->len-1];
    if (h->data_a!=NULL) {
      memcpy((char*)h->data_a->data+ai*h->data_a->unit,
             (char*)h->data_a->data+(h->key_a->len-1)*h->data_a->unit,
             h->data_a->unit);
    }
    h->idx[h->invidx[h->key_a->len-1]]=ai;
    h->invidx[ai]=h->invidx[h->key_a->len-1];
  }
  heapLenIncr(h, -1);
  /* push moved element downheap */
  if (old_invidx_ai<h->key_a->len)
    downheap(h, old_invidx_ai);
  return 0;
}

/* Changes the key (and optional data) of the element ai, re-sorting
 * the heap if necessary. Returns 0 upon success. */
int airHeapUpdate(airHeap *h, unsigned int ai, double newKey,
                  const void *newData) {
  double oldkey;
  if (h==NULL || h->key_a->len<=ai)
    return 1;
  oldkey = h->key[ai];
  /* replace key and data */
  h->key[ai] = newKey;
  if (h->data_a!=NULL && newData!=NULL) {
    memcpy((char*)h->data_a->data+ai*h->data_a->unit,
           newData, h->data_a->unit);
  }
  if (oldkey<newKey) downheap(h, h->invidx[ai]);
  else upheap(h, h->invidx[ai]);
  return 0;
}
