/*
  MotateStaticAssert.h - Library for the Arduino-compatible Motate system
  http://tinkerin.gs/

  Copyright (c) 2012 Robert Giseburt

	This file is part of the Motate Library.

	The Motate Library is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	The Motate Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with the Motate Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MOTATESTATICASSERT_H_ONCE
#define MOTATESTATICASSERT_H_ONCE

// Borrowed technique from http://www.boost.org/libs/static_assert . Thank you Boost!
// Since we know the compiler we'll be using to a certain degree, we can simplify a lot.

// The following allows us to test some things at compile time.
// The resulting error message is less than ideal, however.

namespace Motate {
	template <bool x> struct STATIC_ASSERTION_FAILURE;
	template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
	template<int x> struct static_assert_test{};
}

#define MOTATE_JOIN_HACK( X, Y ) MOTATE_DO_JOIN( X, Y )
#define MOTATE_DO_JOIN( X, Y ) MOTATE_DO_JOIN2(X,Y)
#define MOTATE_DO_JOIN2( X, Y ) X##Y

#define MOTATE_STATIC_ASSERT( B ) \
typedef ::Motate::static_assert_test< \
	sizeof(::Motate::STATIC_ASSERTION_FAILURE< (bool)( B ) >)> \
	MOTATE_JOIN_HACK(motate_static_assert_typedef_, __LINE__);

#endif /* end of include guard: MOTATESTATICASSERT_H_ONCE */
