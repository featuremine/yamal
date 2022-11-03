/* Decimal context module for the decNumber C Library.
   Copyright (C) 2005-2022 Free Software Foundation, Inc.
   Contributed by IBM Corporation.  Author Mike Cowlishaw.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   GCC is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/* ------------------------------------------------------------------ */
/* Decimal Context module					      */
/* ------------------------------------------------------------------ */
/* This module comprises the routines for handling arithmetic	      */
/* context structures.						      */
/* ------------------------------------------------------------------ */

#include <string.h> /* for strcmp */
#ifdef DECCHECK
#include <stdio.h> /* for printf if DECCHECK */
#endif
#include "dconfig.h"        /* for GCC definitions */
#include "decContext.h"     /* context and base types */
#include "decNumberLocal.h" /* decNumber local types, etc. */

/* compile-time endian tester [assumes sizeof(Int)>1] */
static const Int mfcone = 1;                       /* constant 1 */
static const Flag *mfctop = (const Flag *)&mfcone; /* -> top byte */
#define LITEND *mfctop /* named flag; 1=little-endian */

/* ------------------------------------------------------------------ */
/* round-for-reround digits					      */
/* ------------------------------------------------------------------ */
const uByte DECSTICKYTAB[10] = {1, 1, 2, 3, 4,
                                6, 6, 7, 8, 9}; /* used if sticky */

/* ------------------------------------------------------------------ */
/* Powers of ten (powers[n]==10**n, 0<=n<=9)			      */
/* ------------------------------------------------------------------ */
const uInt DECPOWERS[10] = {1,      10,      100,      1000,      10000,
                            100000, 1000000, 10000000, 100000000, 1000000000};

/* ------------------------------------------------------------------ */
/* decContextDefault -- initialize a context structure		      */
/*								      */
/*  context is the structure to be initialized			      */
/*  kind selects the required set of default values, one of:	      */
/*	DEC_INIT_BASE	    -- select ANSI X3-274 defaults	      */
/*	DEC_INIT_DECIMAL32  -- select IEEE 754 defaults, 32-bit       */
/*	DEC_INIT_DECIMAL64  -- select IEEE 754 defaults, 64-bit       */
/*	DEC_INIT_DECIMAL128 -- select IEEE 754 defaults, 128-bit      */
/*	For any other value a valid context is returned, but with     */
/*	Invalid_operation set in the status field.		      */
/*  returns a context structure with the appropriate initial values.  */
/* ------------------------------------------------------------------ */
decContext *decContextDefault(decContext *context, Int kind) {
  /* set defaults... */
  context->round = DEC_ROUND_HALF_UP; /* 0.5 rises */
#if DECSUBSET
  context->extended = 0; /* cleared */
#endif
  switch (kind) {
  case DEC_INIT_BASE:
    /* [use defaults] */
    break;
  case DEC_INIT_DECIMAL32:
    context->round = DEC_ROUND_HALF_EVEN; /* 0.5 to nearest even */
#if DECSUBSET
    context->extended = 1; /* set */
#endif
    break;
  case DEC_INIT_DECIMAL64:
    context->round = DEC_ROUND_HALF_EVEN; /* 0.5 to nearest even */
#if DECSUBSET
    context->extended = 1; /* set */
#endif
    break;
  case DEC_INIT_DECIMAL128:
    context->round = DEC_ROUND_HALF_EVEN; /* 0.5 to nearest even */
#if DECSUBSET
    context->extended = 1; /* set */
#endif
    break;

  default: /* invalid Kind */
    /* use defaults, and .. */
    break;
  }

  return context;
} /* decContextDefault */

/* ------------------------------------------------------------------ */
/* decContextGetRounding -- return current rounding mode	      */
/*								      */
/*  context is the context structure to be queried		      */
/*  returns the rounding mode					      */
/*								      */
/* No error is possible.					      */
/* ------------------------------------------------------------------ */
enum rounding decContextGetRounding(decContext *context) {
  return context->round;
} /* decContextGetRounding */

/* ------------------------------------------------------------------ */
/* decContextSetRounding -- set current rounding mode		      */
/*								      */
/*  context is the context structure to be updated		      */
/*  newround is the value which will replace the current mode	      */
/*  returns context						      */
/*								      */
/* No error is possible.					      */
/* ------------------------------------------------------------------ */
decContext *decContextSetRounding(decContext *context, enum rounding newround) {
  context->round = newround;
  return context;
} /* decContextSetRounding */

/* ------------------------------------------------------------------ */
/* decContextTestEndian -- test whether DECLITEND is set correctly    */
/*								      */
/*  quiet is 1 to suppress message; 0 otherwise 		      */
/*  returns 0 if DECLITEND is correct				      */
/*	    1 if DECLITEND is incorrect and should be 1 	      */
/*	   -1 if DECLITEND is incorrect and should be 0 	      */
/*								      */
/* A message is displayed if the return value is not 0 and quiet==0.  */
/*								      */
/* No error is possible.					      */
/* ------------------------------------------------------------------ */
Int decContextTestEndian(Flag quiet) {
  Int res = 0;                /* optimist */
  uInt dle = (uInt)DECLITEND; /* unsign */
  if (dle > 1)
    dle = 1; /* ensure 0 or 1 */

  if (LITEND != DECLITEND) {
    if (!quiet) {
#if DECCHECK
      const char *adj;
      if (LITEND)
        adj = "little";
      else
        adj = "big";
      printf("Warning: DECLITEND is set to %d, but this computer appears to be "
             "%s-endian\n",
             DECLITEND, adj);
#endif
    }
    res = (Int)LITEND - dle;
  }
  return res;
} /* decContextTestEndian */

/* ------------------------------------------------------------------ */
/* decContextTestSavedStatus -- test bits in saved status	      */
/*								      */
/*  oldstatus is the status word to be tested			      */
/*  mask indicates the bits to be tested (the oldstatus bits that     */
/*    correspond to each 1 bit in the mask are tested)		      */
/*  returns 1 if any of the tested bits are 1, or 0 otherwise	      */
/*								      */
/* No error is possible.					      */
/* ------------------------------------------------------------------ */
uInt decContextTestSavedStatus(uInt oldstatus, uInt mask) {
  return (oldstatus & mask) != 0;
} /* decContextTestSavedStatus */
