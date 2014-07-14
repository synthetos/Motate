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

namespace Motate {
    // Implement a simple circular buffer, with a compile-time size
    template <uint16_t _size, typename base_type = char>
    struct Buffer {
	// Internal properties!
	base_type _data[_size];
	const base_type* const _end_pos = _data+_size; // NOTE: _end_pos is one *past* the end of the buffer.
	volatile base_type* _read_pos = _data;
	volatile base_type* _write_pos = _data;
	volatile int16_t _available = _size;
	volatile bool _write_locked = true;

	struct _mutex {
	    volatile bool &_mutex_lock;
	    _mutex(volatile bool& _lockVar) : _mutex_lock {_lockVar} {
		_mutex_lock = true;
	    };

	    ~_mutex() {
		_mutex_lock = false;
	    }
	};

	constexpr std::size_t size() { return _size; };

	// Default constructors will work fine.

	volatile base_type *nextReadPos() {
	    volatile base_type *ret = _read_pos+1;
	    if (ret == _end_pos) {
		ret = _data;
	    }
	    return ret;
	};

	volatile base_type *nextWritePos() {
	    volatile base_type *ret = _write_pos+1;
	    if (ret == _end_pos) {
		ret = _data;
	    }
	    return ret;
	};

	bool isEmpty() { return _available == _size; }
	bool isFull() { return _available == 0; }
	bool isLocked() { return _write_locked; }
//	bool isEmpty() { return _read_pos == _write_pos; }
//	bool isFull() { return nextReadPos() == _write_pos; }

	int16_t peek() {
	    if (isEmpty())
		return -1;
	    int16_t ret = *_read_pos;
	    return ret;
	};

	void pop() {
	    if (isEmpty())
		return; // Ignore pop on an empty buffer

	    _read_pos = nextReadPos();
	    _available++;
	    return;
	};

	int16_t read() {
	    if (isEmpty() || isLocked())
		return -1;

	    int16_t ret = *_read_pos;
	    _read_pos = nextReadPos();

	    _available++;
	    return ret;
	};

	int16_t write(const base_type newValue) {
	    if (isFull())
		return -1;

	    _mutex write_lock(_write_locked);

	    // as quickly as possible we'll change the availability
	    _available--;

	    *_write_pos = newValue;
	    _write_pos = nextWritePos();

	    return 1;
	};

	int16_t available() {
	    return _available;
//	    if (_read_pos <= _write_pos) {
//		return _write_pos - _read_pos;
//	    } else {
//		return (_write_pos - _data) + (_end_pos - _read_pos);
//	    }
	};
    };
} // namespace Motate

#endif /* end of include guard: MOTATEBUFFER_H_ONCE */