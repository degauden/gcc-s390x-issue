/**
 *  @file Real.h
 *
 *  @brief Floating point comparison implementations
 *
 *  @author Hubert Degaudenzi
 *  @date Jun 13, 2013
 *
 * @copyright 2012-2020 Euclid Science Ground Segment
 *
 * This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *  @details Due to the finite representation of the real numbers in the computing
 *    architecture, the comparison between 2 floating point numbers needs to be
 *    done carefully. In details, even if the representation bit-wise of 2
 *    numbers is different, the real numbers they represent might be the same.
 *
 *    In essence, this is equivalent to compare the 2 numbers \f$x\f$ and \f$y\f$ with a
 *    relative tolerance number \f$\epsilon\f$:
 *    \f[
 *    |x-y| \leq \epsilon |x+y|
 *    \f]
 *
 */

/**
 * @addtogroup ElementsKernel ElementsKernel
 * @{
 */

// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Authors: wan@google.com (Zhanyong Wan), eefacm@gmail.com (Sean Mcafee)
//
// The Google C++ Testing Framework (Google Test)
#ifndef ELEMENTSKERNEL_ELEMENTSKERNEL_REAL_H_
#define ELEMENTSKERNEL_ELEMENTSKERNEL_REAL_H_

#include <cmath>        // for round
#include <limits>       // for numeric_limits
#include <type_traits>  // for is_floating_point

#define ELEMENTS_EXPORT extern
#define ELEMENTS_API
#define ELEMENTS_UNUSED

using std::numeric_limits;

namespace Elements {

/// Double precision float default maximum unit in the last place
constexpr std::size_t DBL_DEFAULT_MAX_ULPS{10};

// For testing purposes only. Rather use the isEqual functions for real
// life comparison
/// Double precision float default test tolerance
ELEMENTS_API extern const double DBL_DEFAULT_TEST_TOLERANCE;

template <std::size_t size>
class ELEMENTS_API TypeWithSize {
public:
  // This prevents the user from using TypeWithSize<N> with incorrect
  // values of N.
  using UInt = void;
};

// The specialisation for size 8.
template <>
class ELEMENTS_API TypeWithSize<8> {
public:
  using Int  = long long;           // NOLINT
  using UInt = unsigned long long;  // NOLINT
};

template <typename RawType>
constexpr std::size_t defaultMaxUlps() {
  return DBL_DEFAULT_MAX_ULPS;
}

template <>
constexpr std::size_t defaultMaxUlps<double>() {
  return DBL_DEFAULT_MAX_ULPS;
}

// This template class represents an IEEE floating-point number
// (either single-precision or double-precision, depending on the
// template parameters).
//
// The purpose of this class is to do more sophisticated number
// comparison.  (Due to round-off error, etc, it's very unlikely that
// two floating-points will be equal exactly.  Hence a naive
// comparison by the == operation often doesn't work.)
//
// Format of IEEE floating-point:
//
//   The most-significant bit being the leftmost, an IEEE
//   floating-point looks like
//
//     sign_bit exponent_bits fraction_bits
//
//   Here, sign_bit is a single bit that designates the sign of the
//   number.
//
//   For float, there are 8 exponent bits and 23 fraction bits.
//
//   For double, there are 11 exponent bits and 52 fraction bits.
//
//   More details can be found at
//   http://en.wikipedia.org/wiki/IEEE_floating-point_standard.
//
// Template parameter:
//
//   RawType: the raw floating-point type (either float or double)
template <typename RawType>
class ELEMENTS_API FloatingPoint {
public:
  // Defines the unsigned integer type that has the same size as the
  // floating point number.
  using Bits = typename TypeWithSize<sizeof(RawType)>::UInt;

  // Constants.

  // # of bits in a number.
  static const std::size_t s_bitcount = 8 * sizeof(RawType);

  // # of fraction bits in a number.
  static const std::size_t s_fraction_bitcount = std::numeric_limits<RawType>::digits - 1;

  // # of exponent bits in a number.
  static const std::size_t s_exponent_bitcount = s_bitcount - 1 - s_fraction_bitcount;

  // The mask for the sign bit.
  static const Bits s_sign_bitmask = static_cast<Bits>(1) << (s_bitcount - 1);

  // The mask for the fraction bits.
  static const Bits s_fraction_bitmask = ~static_cast<Bits>(0) >> (s_exponent_bitcount + 1);

  // The mask for the exponent bits.
  static const Bits s_exponent_bitmask = ~(s_sign_bitmask | s_fraction_bitmask);

  // How many ULP's (Units in the Last Place) we want to tolerate when
  // comparing two numbers.  The larger the value, the more error we
  // allow.  A 0 value means that two numbers must be exactly the same
  // to be considered equal.
  //
  // The maximum error of a single floating-point operation is 0.5
  // units in the last place.  On Intel CPU's, all floating-point
  // calculations are done with 80-bit precision, while double has 64
  // bits.  Therefore, 4 should be enough for ordinary use.
  //
  // See the following article for more details on ULP:
  // http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm.
  static const std::size_t m_max_ulps = defaultMaxUlps<RawType>();

  // Constructs a FloatingPoint from a raw floating-point number.
  //
  // On an Intel CPU, passing a non-normalised NAN (Not a Number)
  // around may change its bits, although the new value is guaranteed
  // to be also a NAN.  Therefore, don't expect this constructor to
  // preserve the bits in x when x is a NAN.
  explicit FloatingPoint(const RawType& x) {
    m_u.m_value = x;
  }

  // Returns the exponent bits of this number.
  Bits exponentBits() const {
    return s_exponent_bitmask & m_u.m_bits;
  }

  // Returns the fraction bits of this number.
  Bits fractionBits() const {
    return s_fraction_bitmask & m_u.m_bits;
  }

  // Converts an integer from the sign-and-magnitude representation to
  // the biased representation.  More precisely, let N be 2 to the
  // power of (kBitCount - 1), an integer x is represented by the
  // unsigned number x + N.
  //
  // For instance,
  //
  //   -N + 1 (the most negative number representable using
  //          sign-and-magnitude) is represented by 1;
  //   0      is represented by N; and
  //   N - 1  (the biggest number representable using
  //          sign-and-magnitude) is represented by 2N - 1.
  //
  // Read http://en.wikipedia.org/wiki/Signed_number_representations
  // for more details on signed number representations.
  static Bits signAndMagnitudeToBiased(const Bits& sam) {
    if (s_sign_bitmask & sam) {
      // sam represents a negative number.
      return ~sam + 1;
    } else {
      // sam represents a positive number.
      return s_sign_bitmask | sam;
    }
  }

  // Given two numbers in the sign-and-magnitude representation,
  // returns the distance between them as an unsigned number.
  static Bits distanceBetweenSignAndMagnitudeNumbers(const Bits& sam1, const Bits& sam2) {
    const Bits biased1 = signAndMagnitudeToBiased(sam1);
    const Bits biased2 = signAndMagnitudeToBiased(sam2);
    return (biased1 >= biased2) ? (biased1 - biased2) : (biased2 - biased1);
  }

private:
  // The data type used to store the actual floating-point number.
  union FloatingPointUnion {
    RawType m_value;  // The raw floating-point number.
    Bits    m_bits;   // The bits that represent the number.
  };

  FloatingPointUnion m_u;
};

template <typename RawType, std::size_t max_ulps = defaultMaxUlps<RawType>()>
bool isEqual(const RawType& left, const RawType& right) {

  bool is_equal{false};

  using Bits  = typename TypeWithSize<sizeof(RawType)>::UInt;
  Bits l_bits = *reinterpret_cast<const Bits*>(&left);
  Bits r_bits = *reinterpret_cast<const Bits*>(&right);
  is_equal    = (FloatingPoint<RawType>::distanceBetweenSignAndMagnitudeNumbers(l_bits, r_bits) <= max_ulps);

  return is_equal;
}

template <std::size_t max_ulps>
inline bool isEqual(const double& left, const double& right) {
  return (isEqual<double, max_ulps>(left, right));
}

}  // namespace Elements

#endif  // ELEMENTSKERNEL_ELEMENTSKERNEL_REAL_H_

/**@}*/
