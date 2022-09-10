/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#include <cgraph/alloc.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/// a description of where a buffer is located
typedef enum {
  AGXBUF_ON_HEAP = 0, ///< buffer is dynamically allocated
  AGXBUF_ON_STACK     ///< buffer is statically allocated
} agxbuf_loc_t;

/* Extensible buffer:
 *  Malloc'ed memory is never released until agxbfree is called.
 */
typedef struct {
  char *buf;            ///< start of buffer
  char *ptr;            ///< next place to write
  char *eptr;           ///< end of buffer
  agxbuf_loc_t located; ///< where does the backing memory for this buffer live?
} agxbuf;

/* agxbinit:
 * Initializes new agxbuf; caller provides memory.
 * Assume if init is non-null, hint = sizeof(init[])
 */
static inline void agxbinit(agxbuf *xb, unsigned int hint, char *init) {
  if (init != NULL) {
    xb->buf = init;
    xb->located = AGXBUF_ON_STACK;
  } else {
    if (hint == 0) {
      hint = BUFSIZ;
    }
    xb->located = AGXBUF_ON_HEAP;
    xb->buf = (char *)gv_calloc(hint, sizeof(char));
  }
  xb->eptr = xb->buf + hint;
  xb->ptr = xb->buf;
  *xb->ptr = '\0';
}

/* agxbfree:
 * Free any malloced resources.
 */
static inline void agxbfree(agxbuf *xb) {
  if (xb->located == AGXBUF_ON_HEAP)
    free(xb->buf);
}

/* agxblen:
 * Return number of characters currently stored.
 */
static inline size_t agxblen(const agxbuf *xb) {
  return (size_t)(xb->ptr - xb->buf);
}

/// get the size of the backing memory of a buffer
///
/// In contrast to \p agxblen, this is the total number of usable bytes in the
/// backing store, not the total number of currently stored bytes.
///
/// \param xb Buffer to operate on
/// \return Number of usable bytes in the backing store
static inline size_t agxbsizeof(const agxbuf *xb) {
  return (size_t)(xb->eptr - xb->buf);
}

/* agxbpop:
 * Removes last character added, if any.
 */
static inline int agxbpop(agxbuf *xb) {

  if (agxblen(xb) == 0) {
    return -1;
  }

  int c = *xb->ptr--;
  return c;
}

/* agxbmore:
 * Expand buffer to hold at least ssz more bytes.
 */
static inline void agxbmore(agxbuf *xb, size_t ssz) {
  size_t cnt = 0;   // current no. of characters in buffer
  size_t size = 0;  // current buffer size
  size_t nsize = 0; // new buffer size
  char *nbuf;       // new buffer

  size = agxbsizeof(xb);
  nsize = size == 0 ? BUFSIZ : (2 * size);
  if (size + ssz > nsize)
    nsize = size + ssz;
  cnt = agxblen(xb);
  if (xb->located == AGXBUF_ON_HEAP) {
    nbuf = (char *)gv_recalloc(xb->buf, size, nsize, sizeof(char));
  } else {
    nbuf = (char *)gv_calloc(nsize, sizeof(char));
    memcpy(nbuf, xb->buf, cnt);
    xb->located = AGXBUF_ON_HEAP;
  }
  xb->buf = nbuf;
  xb->ptr = xb->buf + cnt;
  xb->eptr = xb->buf + nsize;
}

/* support for extra API misuse warnings if available */
#ifdef __GNUC__
#define PRINTF_LIKE(index, first) __attribute__((format(printf, index, first)))
#else
#define PRINTF_LIKE(index, first) /* nothing */
#endif

/* agxbprint:
 * Printf-style output to an agxbuf
 */
static inline PRINTF_LIKE(2, 3) int agxbprint(agxbuf *xb, const char *fmt,
                                              ...) {
  va_list ap;
  size_t size;
  int result;

  va_start(ap, fmt);

  // determine how many bytes we need to print
  {
    va_list ap2;
    int rc;
    va_copy(ap2, ap);
    rc = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (rc < 0) {
      va_end(ap);
      return rc;
    }
    size = (size_t)rc + 1; // account for NUL terminator
  }

  // do we need to expand the buffer?
  {
    size_t unused_space = agxbsizeof(xb) - agxblen(xb);
    if (unused_space < size) {
      size_t extra = size - unused_space;
      agxbmore(xb, extra);
    }
  }

  // we can now safely print into the buffer
  result = vsnprintf(xb->ptr, size, fmt, ap);
  assert(result == (int)(size - 1) || result < 0);
  if (result > 0) {
    xb->ptr += (size_t)result;
  }

  va_end(ap);
  return result;
}

#undef PRINTF_LIKE

/* agxbput_n:
 * Append string s of length ssz into xb
 */
static inline size_t agxbput_n(agxbuf *xb, const char *s, size_t ssz) {
  if (ssz == 0) {
    return 0;
  }
  if (ssz > agxbsizeof(xb) - agxblen(xb))
    agxbmore(xb, ssz);
  memcpy(xb->ptr, s, ssz);
  xb->ptr += ssz;
  return ssz;
}

/* agxbput:
 * Append string s into xb
 */
static inline size_t agxbput(agxbuf *xb, const char *s) {
  size_t ssz = strlen(s);

  return agxbput_n(xb, s, ssz);
}

/* agxbputc:
 * Add character to buffer.
 *  int agxbputc(agxbuf*, char)
 */
static inline int agxbputc(agxbuf *xb, char c) {
  if (agxblen(xb) >= agxbsizeof(xb)) {
    agxbmore(xb, 1);
  }
  *xb->ptr++ = c;
  return 0;
}

/* agxbuse:
 * Null-terminates buffer; resets and returns pointer to data. The buffer is
 * still associated with the agxbuf and will be overwritten on the next, e.g.,
 * agxbput. If you want to retrieve and disassociate the buffer, use agxbdisown
 * instead.
 */
static inline char *agxbuse(agxbuf *xb) {
  (void)agxbputc(xb, '\0');
  xb->ptr = xb->buf;
  return xb->ptr;
}

/* agxbstart:
 * Return pointer to beginning of buffer.
 */
static inline char *agxbstart(agxbuf *xb) { return xb->buf; }

/* agxbclear:
 * Resets pointer to data;
 */
static inline void agxbclear(agxbuf *xb) { xb->ptr = xb->buf; }

/* agxbnext:
 * Next position for writing.
 */
static inline char *agxbnext(agxbuf *xb) { return xb->ptr; }

/* agxbdisown:
 * Disassociate the backing buffer from this agxbuf and return it. The buffer is
 * NUL terminated before being returned. If the agxbuf is using stack memory,
 * this will first copy the data to a new heap buffer to then return. If you
 * want to temporarily access the string in the buffer, but have it overwritten
 * and reused the next time, e.g., agxbput is called, use agxbuse instead of
 * agxbdisown.
 */
static inline char *agxbdisown(agxbuf *xb) {
  char *buf;

  if (xb->located == AGXBUF_ON_STACK) {
    // the buffer is not dynamically allocated, so we need to copy its contents
    // to heap memory

    buf = gv_strndup(xb->buf, agxblen(xb));

  } else {
    // the buffer is already dynamically allocated, so terminate it and then
    // take it as-is
    agxbputc(xb, '\0');
    buf = xb->buf;
  }

  // reset xb to a state where it is usable
  xb->buf = xb->ptr = xb->eptr = NULL;
  xb->located = AGXBUF_ON_HEAP;

  return buf;
}
