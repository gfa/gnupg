/* membuf.c - A simple implementation of a dynamic buffer.
 * Copyright (C) 2001, 2003, 2009, 2011 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either
 *
 *   - the GNU Lesser General Public License as published by the Free
 *     Software Foundation; either version 3 of the License, or (at
 *     your option) any later version.
 *
 * or
 *
 *   - the GNU General Public License as published by the Free
 *     Software Foundation; either version 2 of the License, or (at
 *     your option) any later version.
 *
 * or both in parallel, as here.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdlib.h>
#include <errno.h>

#include "membuf.h"

#include "util.h"


/* A simple implementation of a dynamic buffer.  Use init_membuf() to
   create a buffer, put_membuf to append bytes and get_membuf to
   release and return the buffer.  Allocation errors are detected but
   only returned at the final get_membuf(), this helps not to clutter
   the code with out of core checks.  */

void
init_membuf (membuf_t *mb, int initiallen)
{
  mb->len = 0;
  mb->size = initiallen;
  mb->out_of_core = 0;
  mb->buf = xtrymalloc (initiallen);
  if (!mb->buf)
    mb->out_of_core = errno;
}

/* Same as init_membuf but allocates the buffer in secure memory.  */
void
init_membuf_secure (membuf_t *mb, int initiallen)
{
  mb->len = 0;
  mb->size = initiallen;
  mb->out_of_core = 0;
  mb->buf = xtrymalloc_secure (initiallen);
  if (!mb->buf)
    mb->out_of_core = errno;
}


/* Shift the the content of the membuf MB by AMOUNT bytes.  The next
   operation will then behave as if AMOUNT bytes had not been put into
   the buffer.  If AMOUNT is greater than the actual accumulated
   bytes, the membuf is basically reset to its initial state.  */
void
clear_membuf (membuf_t *mb, size_t amount)
{
  /* No need to clear if we are already out of core.  */
  if (mb->out_of_core)
    return;
  if (amount >= mb->len)
    mb->len = 0;
  else
    {
      mb->len -= amount;
      memmove (mb->buf, mb->buf+amount, mb->len);
    }
}


void
put_membuf (membuf_t *mb, const void *buf, size_t len)
{
  if (mb->out_of_core || !len)
    return;

  if (mb->len + len >= mb->size)
    {
      char *p;

      mb->size += len + 1024;
      p = xtryrealloc (mb->buf, mb->size);
      if (!p)
        {
          mb->out_of_core = errno ? errno : ENOMEM;
          /* Wipe out what we already accumulated.  This is required
             in case we are storing sensitive data here.  The membuf
             API does not provide another way to cleanup after an
             error. */
          wipememory (mb->buf, mb->len);
          return;
        }
      mb->buf = p;
    }
  memcpy (mb->buf + mb->len, buf, len);
  mb->len += len;
}


void
put_membuf_str (membuf_t *mb, const char *string)
{
  put_membuf (mb, string, strlen (string));
}


void *
get_membuf (membuf_t *mb, size_t *len)
{
  char *p;

  if (mb->out_of_core)
    {
      if (mb->buf)
        {
          wipememory (mb->buf, mb->len);
          xfree (mb->buf);
          mb->buf = NULL;
        }
      gpg_err_set_errno (mb->out_of_core);
      return NULL;
    }

  p = mb->buf;
  if (len)
    *len = mb->len;
  mb->buf = NULL;
  mb->out_of_core = ENOMEM; /* hack to make sure it won't get reused. */
  return p;
}


/* Peek at the membuf MB.  On success a pointer to the buffer is
   returned which is valid until the next operation on MB.  If LEN is
   not NULL the current LEN of the buffer is stored there.  On error
   NULL is returned and ERRNO is set.  */
const void *
peek_membuf (membuf_t *mb, size_t *len)
{
  const char *p;

  if (mb->out_of_core)
    {
      gpg_err_set_errno (mb->out_of_core);
      return NULL;
    }

  p = mb->buf;
  if (len)
    *len = mb->len;
  return p;
}