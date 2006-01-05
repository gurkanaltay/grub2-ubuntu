/* err.c - error handling routines */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2002,2005  Free Software Foundation, Inc.
 *
 *  GRUB is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <grub/err.h>
#include <grub/misc.h>
#include <stdarg.h>

#define GRUB_MAX_ERRMSG		256
#define GRUB_ERROR_STACK_SIZE	10

grub_err_t grub_errno;
char grub_errmsg[GRUB_MAX_ERRMSG];

static struct
{
  grub_err_t errno;
  char errmsg[GRUB_MAX_ERRMSG];
} grub_error_stack_items[GRUB_ERROR_STACK_SIZE];

static int grub_error_stack_pos;
static int grub_error_stack_assert;

grub_err_t
grub_error (grub_err_t n, const char *fmt, ...)
{
  va_list ap;
  
  grub_errno = n;

  va_start (ap, fmt);
  grub_vsprintf (grub_errmsg, fmt, ap);
  va_end (ap);

  return n;
}

void
grub_fatal (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  grub_vprintf (fmt, ap);
  va_end (ap);

  grub_stop ();
}

void
grub_error_push (void)
{
  /* Only add items to stack, if there is enough room.  */
  if (grub_error_stack_pos < GRUB_ERROR_STACK_SIZE)
    {
      /* Copy active error message to stack.  */
      grub_error_stack_items[grub_error_stack_pos].errno = grub_errno;
      grub_memcpy (grub_error_stack_items[grub_error_stack_pos].errmsg,
                   grub_errmsg,
                   sizeof (grub_errmsg));
        
      /* Advance to next error stack position.  */
      grub_error_stack_pos++;
    }
  else
    {
      /* There is no room for new error message. Discard new error message
         and mark error stack assertion flag.  */
      grub_error_stack_assert = 1;
    }

  /* Allow further operation of other components by resetting
     active errno to GRUB_ERR_NONE.  */
  grub_errno = GRUB_ERR_NONE;
}

int
grub_error_pop (void)
{
  if (grub_error_stack_pos > 0)
    {
      /* Pop error message from error stack to current active error.  */
      grub_error_stack_pos--;
      
      grub_errno = grub_error_stack_items[grub_error_stack_pos].errno;
      grub_memcpy (grub_errmsg,
                   grub_error_stack_items[grub_error_stack_pos].errmsg,
                   sizeof (grub_errmsg));
                  
      return 1;
    }
  else
    {
      /* There is no more items on error stack, reset to no error state.  */
      grub_errno = GRUB_ERR_NONE;
      
      return 0;
    }
}

void
grub_print_error (void)
{
  /* Print error messages in reverse order. First print active error message
     and then empty error stack.  */
  do
    {
      if (grub_errno != GRUB_ERR_NONE)
        grub_printf ("error: %s\n", grub_errmsg);
    } 
  while (grub_error_pop ());
  
  /* If there was an assert while using error stack, report about it.  */
  if (grub_error_stack_assert)
    {
      grub_printf ("assert: error stack overflow detected!\n");
      grub_error_stack_assert = 0;
    }
}
