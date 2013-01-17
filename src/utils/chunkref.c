/*
    Copyright (c) 2013 250bpm s.r.o.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "chunkref.h"
#include "err.h"

#include <string.h>

/*  sp_chunkref should be reinterpreted as this structure in case the first
    byte ('tag') is 0xff. */
struct sp_chunkref_chunk {
    uint8_t tag;
    struct sp_chunk *chunk;
};

/*  Compile time check whether VSM are small enough for size to fit into the
    first byte of the structure. */
typedef int sp_chunkref_vsm_size_test [SP_CHUNKREF_MAX < 255 ? 1 : -1] ;

/*  Compile time check to determine whether sp_chunkref_chunk fits
    into sp_chunkref. */
typedef int sp_chunkref_chunk_size_test [sizeof (struct sp_chunkref) >=
    sizeof (struct sp_chunkref_chunk) ? 1 : -1];

void sp_chunkref_init (struct sp_chunkref *self, size_t size)
{
    struct sp_chunkref_chunk *ch;

    if (size < SP_CHUNKREF_MAX) {
        self->ref [0] = (uint8_t) size;
        return;
    }

    ch = (struct sp_chunkref_chunk*) self;
    ch->tag = 0xff;
    ch->chunk = sp_chunk_alloc (size, 0);
    alloc_assert (ch->chunk);
}

void sp_chunkref_init_chunk (struct sp_chunkref *self, struct sp_chunk *chunk)
{
    struct sp_chunkref_chunk *ch;

    ch = (struct sp_chunkref_chunk*) self;
    ch->tag = 0xff;
    ch->chunk = chunk;
}

void sp_chunkref_term (struct sp_chunkref *self)
{
}

void sp_chunkref_mv (struct sp_chunkref *dst, struct sp_chunkref *src)
{
    memcpy (dst, src, src->ref [0] == 0xff ?
        sizeof (struct sp_chunkref_chunk) : src->ref [0] + 1);
}

void *sp_chunkref_data (struct sp_chunkref *self)
{
    return self->ref [0] == 0xff ?
        sp_chunk_data (((struct sp_chunkref_chunk*) self)->chunk) :
        &self->ref [1];
}

size_t sp_chunkref_size (struct sp_chunkref *self)
{
    return self->ref [0] == 0xff ?
        sp_chunk_size (((struct sp_chunkref_chunk*) self)->chunk) :
        self->ref [0];
}

void sp_chunkref_trim (struct sp_chunkref *self, size_t n)
{
    struct sp_chunkref_chunk *ch;

    if (self->ref [0] == 0xff) {
        ch = (struct sp_chunkref_chunk*) self;
        ch->chunk = sp_chunk_trim (ch->chunk, n);
        return;
    }

    sp_assert (self->ref [0] >= n);
    memmove (&self->ref [1], &self->ref [1 + n], self->ref [0] - n);
    self->ref [0] -= n;
}
