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

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*	Public header file for the virtual malloc package.
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 01/16/94.
*/

    typedef struct _vmalloc_s Vmalloc_t;

    struct _vmalloc_s {
	void **allocated;	/* pointers we have given out           */
	size_t size;	/* used entries in `allocated`          */
	size_t capacity;	/* available entries in `allocated`     */
    };

    extern Vmalloc_t *vmopen(void);
    extern int vmclose(Vmalloc_t *);
    extern int vmclear(Vmalloc_t *);

/** allocate heap memory
 *
 * @param vm region allocating from
 * @param size desired block size
 * @returns Memory fulfilling the allocation request or NULL on failure
 */
void *vmalloc(Vmalloc_t *vm, size_t size);

/** resize an area of allocated memory
 *
 * @param vm region allocation from
 * @param data old block of data
 * @param size new size
 * @returns Pointer to the newly resized area or NULL on failure
 */
void *vmresize(Vmalloc_t *vm, void *data, size_t size);

/** free heap memory
 *
 * @param vm Region the pointer was originally allocated from
 * @param data The pointer originally received from vmalloc
 */
void vmfree(Vmalloc_t *vm, void *data);

    extern char *vmstrdup(Vmalloc_t *, const char *);

#define vmnewof(v,p,t,n,x)	vmresize((v), (p), sizeof(t)*(n)+(x))
#ifdef __cplusplus
}
#endif
