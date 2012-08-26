/*
  MotatePins.hpp - Library for the Arduino-compatible Motate system
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

#ifndef MOTATECIRCULARBUFFER_H_ONCE
#define MOTATECIRCULARBUFFER_H_ONCE

#include <MotateStaticAssert.h>
#include <inttypes.h>

namespace Motate {
	// Super-simple circular buffer, where old nodes are reused
	
	// It is similar to a FIFO structure, but objects are pre-allocated.
	// You "add" an object to the head by requesting the head and modifying it.
	// You "remove" an object from the tail by retrieving it, and then bumping it.
	
	// WARNING WARNING WARNING: The buffer must be sized a power of two, otherwise operations will not work
	// (2, 4, 8, 16, 32, 64)
	// You have been warned!
	template<typename T, int8_t buffer_size, typename buffer_size_type = int8_t>
	class CircularBuffer
	{
		MOTATE_STATIC_ASSERT((buffer_size & (buffer_size-1)) == 0); // Failure here means buffer_size is not 2, 4, 8, 16, 32, 64.
		MOTATE_STATIC_ASSERT(buffer_size < 128); // 64 is actually the top, but ... whatever. It works.
		
	public:
		typedef T BufDataType;
		typedef buffer_size_type BufSizeType;
		
	private:
		volatile BufSizeType head, tail;
		static const BufSizeType size = buffer_size;
		static const BufSizeType size_mask = buffer_size-1;
		BufDataType /*const */data[buffer_size]; // the buffer
	
	public:
		CircularBuffer() : head(0), tail(0) {
			for (BufSizeType i = 0; i < size; i++) {
				data[i] = BufDataType();
			}
		};
		
		inline BufDataType *getHead() {
			return &data[head];
		}
		inline BufSizeType getHeadIndex() {
			return head;
		}
		
		inline BufDataType *getTail() {
			return &data[tail];
		}
		inline BufSizeType getTailIndex() {
			return tail;
		}
		
		inline BufSizeType getNextIndex(BufSizeType from) {
			return ((from + 1) & size_mask);
		}
		
		inline BufSizeType getPreviousIndex(BufSizeType from) {
			return (((from+size) - 1) & size_mask);
		}
		
		inline BufDataType *getNextHead() {
			return &data[getNextIndex(head)];
		}
		
		inline BufDataType &operator[] (BufSizeType index) {
			 // adding size should make negative indexes < size work ok
			// int16_t offset = index < 0 ? index : ((index + size) & size_mask);
			return data[((index + size) & size_mask)];
		}
		
		// bump the head. cannot return anything useful, so it doesn't
		// WARNING: no sanity checks!
		inline void bumpHead() {
			head = getNextIndex(head);
		}

		// bump the tail. cannot return anything useful, so it doesn't
		// WARNING: no sanity checks!
		inline void bumpTail() {
			tail = getNextIndex(tail);
		}
		
		inline bool isEmpty() {
			return head == tail;
		}
		
		inline bool isFull() {
			return (getNextIndex(head) == tail);
		}
		
		inline BufSizeType getUsedCount() {
			return ((head-tail+size) & size_mask);
		}
		
		inline void clear() {
			head = 0;
			tail = 0;
		}
	};

	template<int8_t buffer_size, typename buffer_size_type>
	class CircularBuffer<char, buffer_size, buffer_size_type>
	{
		MOTATE_STATIC_ASSERT((buffer_size & (buffer_size-1)) == 0); // Failure here means buffer_size is not 2, 4, 8, 16, 32, 64.
		MOTATE_STATIC_ASSERT(buffer_size < 128); // 64 is actually the top, but ... whatever. It works.
		
	public:
		typedef char BufDataType;
		typedef buffer_size_type BufSizeType;
		
	private:
		volatile BufSizeType head, tail;
		static const BufSizeType size = buffer_size;
		static const BufSizeType size_mask = buffer_size-1;
		BufDataType data[buffer_size]; // the buffer
	
	public:
		CircularBuffer() : head(0), tail(0)/*, data()*/ {};
		
		// NO GUARDS
		void setHead(BufDataType c) {
			data[head] = c;
			head = (head+1) & size_mask;
		}
		
		// NO GUARDS
		BufDataType getTail() {
			BufDataType c = data[tail];
			tail = (tail+1) & size_mask;
			return c;
		}
		
		// NO GUARDS
		void bumpTail() {
			tail = (tail+1) & size_mask;
		}
		
		BufDataType peek() {
			return data[tail];
		}
		
		bool isEmpty() {
			return head == tail;
		}
		
		bool isFull() {
			return (((head+1) & size_mask) == tail);
		}
		
		BufSizeType getUsedCount() {
			return (((head-tail)+size) & size_mask);
		}
		
		void clear() {
			head = 0;
			tail = 0;
		}
	};

	
} // namespace Motate

#endif /* end of include guard: MOTATECIRCULARBUFFER_H_ONCE */