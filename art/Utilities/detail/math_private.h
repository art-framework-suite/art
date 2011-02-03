#ifndef art_Utilities_detail_math_private_h
#define art_Utilities_detail_math_private_h

// abridged from GNU libc 2.6.1 - in detail from
//   math/math_private.h
//   sysdeps/ieee754/ldbl-96/math_ldbl.h

// part of this file:
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include <sys/types.h>

namespace art {
  namespace math_private {

    // A union which permits us to convert between a float and a 32 bit int.
    typedef union
    {
      float value;
      u_int32_t word;
    } ieee_float_shape_type;

    // A union which permits us to convert between a double and two 32 bit ints.
    typedef union
    {
      double value;
      struct
      {
        u_int32_t lsw;
        u_int32_t msw;
      } parts;
    } ieee_double_shape_type;

    // A union which permits us to convert between a long double and three 32 bit ints.
    typedef union
    {
      long double value;
      struct
      {
        u_int32_t lsw;
        u_int32_t msw;
        int sign_exponent:16;
        unsigned int empty:16;
     // unsigned int empty1:32;   // maybe needed for 128-bit long double ? (x86-64 and/or -m128bit-long-double)
      } parts;
    } ieee_long_double_shape_type;

    /* Get a 32 bit int from a float.  */
    #define GET_FLOAT_WORD(i,d)                                     \
    do {                                                            \
      art::math_private::ieee_float_shape_type gf_u;                \
      gf_u.value = (d);                                             \
      (i) = gf_u.word;                                              \
    } while (0)

    /* Get two 32 bit ints from a double.  */
    #define EXTRACT_WORDS(ix0,ix1,d)                                \
    do {                                                            \
      art::math_private::ieee_double_shape_type ew_u;               \
      ew_u.value = (d);                                             \
      (ix0) = ew_u.parts.msw;                                       \
      (ix1) = ew_u.parts.lsw;                                       \
    } while (0)

    /* Get three 32 bit ints from a long double.  */
    #define GET_LDOUBLE_WORDS(exp,ix0,ix1,d)                        \
    do {                                                            \
        art::math_private::ieee_long_double_shape_type ew_u;        \
        ew_u.value = (d);                                           \
        (exp) = ew_u.parts.sign_exponent;                           \
        (ix0) = ew_u.parts.msw;                                     \
        (ix1) = ew_u.parts.lsw;                                     \
    } while (0)

  }  // math_private
}  // art

// ======================================================================

#endif /* art_Utilities_detail_math_private_h */

// Local Variables:
// mode: c++
// End:
