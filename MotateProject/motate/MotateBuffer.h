/*
  MotateBuffer.hpp - Buffer templates for the Motate system
  http://tinkerin.gs/

  Copyright (c) 2013 Robert Giseburt

	This file is part of the Motate Library.

	This file ("the software") is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License, version 2 as published by the
	Free Software Foundation. You should have received a copy of the GNU General Public
	License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.

	As a special exception, you may use this file as part of a software library without
	restriction. Specifically, if other files instantiate templates or use macros or
	inline functions from this file, or you compile this file and link it with  other
	files to produce an executable, this file does not by itself cause the resulting
	executable to be covered by the GNU General Public License. This exception does not
	however invalidate any other reasons why the executable file might be covered by the
	GNU General Public License.

	THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
	WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
	SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
	OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MOTATEBUFFER_H_ONCE
#define MOTATEBUFFER_H_ONCE

#include <inttypes.h>
#include <utility> // for std::move
#include <initializer_list>

namespace Motate {
	// Implement a regular buffer, with a compile-time size
	template <uint16_t _size, typename base_type = char>
	struct Buffer {

		// Internal properties!
		base_type _data[_size];

		constexpr std::size_t size() { return _size; };

		// Default construcotrs will work fine.

		// begin() and end()
		base_type* begin() { return _data; };
		base_type* end() { return _data+_size; };


		// Act like a pointer
		// We're not going to offer any safety that a normal array doesn't!
		// BUT, if we wanted to, we could.
		operator base_type*() { return _data; };
		operator const base_type*() const { return _data; };
		base_type* operator*() { return _data; };
	};
} // namespace Motate

#endif /* end of include guard: MOTATEBUFFER_H_ONCE */