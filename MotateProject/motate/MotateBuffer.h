/*
 MotateBuffer.hpp - Buffer templates for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2013 - 2016 Robert Giseburt
 Copyright (c)        2016 Alden S. Hart, Jr.

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

#include <cstring> // for size_t
//#include <utility> // for std::move
#include <functional> // for std::function

namespace Motate {
    // Implement a simple circular buffer, with a compile-time size
    template <uint16_t _size, typename base_type = char>
    struct Buffer {
        static_assert(((_size-1)&_size)==0, "Buffer size must be 2^N");

        // Internal properties!
        base_type _data[_size+1];

        volatile uint16_t _read_offset;              // The offset into the buffer of our next read
        volatile uint16_t _write_offset;             // The offset into the buffer of our next write

        Buffer() { _data[_size] = 0; };

        uint16_t _nextReadOffset() {
            return (_read_offset + 1)&(_size-1);
        };

        uint16_t _nextWriteOffset() {
            return (_write_offset + 1)&(_size-1);
        };

        constexpr int16_t size() { return _size; };

        bool isEmpty() { return _read_offset == _write_offset; }
        bool isFull() { return ((_write_offset+1)&(_size-1)) == _read_offset; }
        bool isLocked() { return false; }

        int16_t peek() {
            if (isEmpty())
                return -1;

            int16_t ret = _data[_read_offset];
            return ret;
        };

        void pop() {
            if (isEmpty())
                return; // Ignore pop on an empty buffer

            _read_offset = _nextReadOffset();
            return;
        };

        int16_t read() {
            if (isEmpty()) {
                return -1;
            }

            int16_t ret = _data[_read_offset];
            _read_offset = _nextReadOffset();

            return ret;
        };

        int16_t write(const base_type newValue) {
            if (isFull())
                return -1;
            
            _data[_write_offset] = newValue;
            _write_offset = _nextWriteOffset();

            return 1;
        };

        int16_t available() {
            if (_write_offset == _read_offset) {
                return _size;
            } else if (_write_offset < _read_offset) {
                return _size  - (_read_offset - _write_offset);
            } else {
                return (_read_offset) + (_size - _write_offset);
            }
        };
    };

    /* RXBuffer<uint16_t _size, typename owner_type, typename base_type = char>
     * Implements a simple circular buffer, with a compile-time size, and can only be written to by DMA
     * owner_type is a *pointer* type that implements these methods:
     *   const base_type* getRXTransferPosition()
     *   void setRXTransferDoneCallback(std::function<void()> &&callback)
     *   bool startRXTransfer(char *&buffer, uint16_t length)
     */
    template <uint16_t _size, typename owner_type, typename base_type = char>
    struct RXBuffer {
        static_assert(((_size-1)&_size)==0, "RXBuffer size must be 2^N");

        owner_type _owner;


        volatile uint16_t _read_offset = 0;              // The offset into the buffer of our next read
        volatile uint16_t _last_known_write_offset = 0;  // The offset into the buffer of the last known write (cached)
        volatile uint16_t _last_requested_write_offset = 0;  // The offset into the buffer of the last requested write

        volatile uint16_t _transfer_requested = 0;   // keep track of how much we have requested. Non-zero means a request is active.

        // Internal properties!
        // Some devices write in whole-word (4-byte) chunks, even though the last bytes are garbage, and past what we requested.
        // So, we add 4-bytes past what we need to allocate.
        // We add one more to keep a null-termination, for various reasons, among them easier debugging.
        base_type _data[_size+1+4];
        uint32_t _data_end_guard = 0xBEEF;

        constexpr int16_t size() { return _size; };

        RXBuffer(owner_type owner) : _owner(owner) { _data[_size] = 0; };

        void init() {
            _owner->setRXTransferDoneCallback([&]() { // use a closure
                _transfer_requested = 0;
                //_restartTransfer();
            });
        };

        uint16_t _nextReadOffset() {
            return (_read_offset + 1)&(_size-1);
        };

        bool _canBeRead(uint16_t pos) {
            if (pos == _last_known_write_offset) {
                _getWriteOffset();
                if (pos == _last_known_write_offset) {
                    //_restartTransfer();
                    return false;
                }
            }

            return true;
        };

        uint16_t _getWriteOffset() {
            base_type* pos = _owner->getRXTransferPosition();
            if (nullptr != pos) {
                _last_known_write_offset = (pos - _data) & (_size-1); // if it's one past the end, we want it to become zero
            }
            return _last_known_write_offset;
        }


        bool isLocked() { return false; } // this kind of buffer cannot be locked

        // We are receiving. It will only get *more full* as a transfer continues.

        bool isEmpty() {
            // If we weren't empty last time we checked, we aren't empty now.
            if (!_isEmptyCached()) {
                return false;
            }

            // Update the cache and check again
            _getWriteOffset();
            return _isEmptyCached();
        }

        bool isFull() {
            // If we were full last time we checked, we are still full
            if (_isFullCached()) {
                return true;
            }

            // Update the cache and check again
            _getWriteOffset();
            return _isFullCached();
        }

        // It's empty if the write position would be the same as the read.
        bool _isEmptyCached() { return _last_known_write_offset == _read_offset; }

        // It's full if the next write position would be the same as the read.
        bool _isFullCached() { return ((_last_known_write_offset+1)&(_size-1)) == _read_offset; }

        void flush() {
            // We can't stop the machinery, but we can "trow away" what we have read so far.
            _read_offset = _getWriteOffset();
        }

        int16_t peek() {
            if (isEmpty())
                return -1;

            int16_t ret = _data[_read_offset];
            return ret;
        };

        void pop() {
            if (isEmpty())
                return; // Ignore pop on an empty buffer

            _read_offset = _nextReadOffset();
            return;
        };

        void _restartTransfer() {
            if (_transfer_requested != 0) {
                return;
            }

            // We can only request contiguous chunks. Let's see what the next one is.
            _getWriteOffset(); // cache the write position

            // Bail early if the buffer is full.
            if (isFull()) {
                return;
            }

            // Some owners can handle two data blocks, others will ignore the second one,
            // so we compute both and pass them both to startRXTransfer.
            // Note that we will only have a second one if the available region "wraps"
            // past the end of the circular buffer.

            int16_t transfer_size = 0;
            int16_t transfer_size_extra = 0;
            base_type *write_pos = _data + _last_known_write_offset;
            base_type *write_pos_extra = _data;

            // Possible cases:
            // We must keep in mind we don't want to read _size bytes, but _size-1.
            // (In the "drawings", "~" is data, "." is freed, "R" is the read pos, and "W" is thw write position.
            // [0] write_offset+1 = _read_offset
            //     |~~~~~~~~~~~~~WR~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
            //     We just eliminated this, the full case.
            // [1] _read_offset > write_offset
            //     |~~~~~~~~~~~~~W...............R~~~~~~~~~~~~~~|
            //     IOW: We have read to some position in the middle, and write_offset is before it.
            //          The unread data is between _read_offset->end, then 0->write_offset.
            //          So, we transfer from write_offset to the _read_offset-1.
            // [2] _read_offset <= write_offset
            //     |.............R~~~~~~~~~~~~~~~W..............|
            //     IOW: We have read to some position in the middle, and _read_offset is in the range 0 through
            //           write_offset. So, we can transfer from write_offset to the end of the buffer, and a second
            //           region between the end and the _read_offset-1.
            // [2a] _read_offset <= write_offset && _read_offset == 0
            //     |R~~~~~~~~~~~~~~~~~~W........................|
            //     IOW: If we read to the end, we will read _size bytes, and our "full" will look like "empty".
            //          So, we read to the end of the buffer-1.

            // Additional note: SOME DMA systems transfer in 4-byte words, and so a non-4-byte transfer will
            // drop 3 NULLS (or other garbage) into the area past what we requested.
            // So, we have padded the end of the data buffer by 4 bytes in order to catch those for [2] and [2a],
            // ans will leave 4 bytes of padding for [1] and the extra on [2].

            // Case [1]
            if (_read_offset > _last_known_write_offset) {
                transfer_size = (_read_offset - _last_known_write_offset) - 4;
                if (transfer_size < 1) {
                    return;
                }
            // Case [2a]
            } else if (_read_offset == 0) {
                transfer_size = (_size - _last_known_write_offset) - 1;

            // Case [2]
            } else {
                transfer_size = (_size - _last_known_write_offset);
                transfer_size_extra = std::max(0, _read_offset - 4);
            }

            _transfer_requested = transfer_size + transfer_size_extra;

            // startRXTransfer will return false if it couldn't start the transfer.
            if (_owner->startRXTransfer(write_pos, transfer_size, write_pos_extra, transfer_size_extra)) {
                _last_requested_write_offset = (_last_known_write_offset + _transfer_requested) & (_size-1);
                return;
            }

            // If we're here, startRXTransfer loaded some data into the buffer and ran out of room.
            // Note that _getWriteOffset() must return the new position
            _transfer_requested = 0;
        };

        int16_t read() {
            if (isEmpty()) {
                _restartTransfer();
                return -1;
            }

            int16_t ret = _data[_read_offset];
            _read_offset = _nextReadOffset();

            return ret;
        };

        int16_t _getAvailableCached() {
            if (_read_offset == _last_known_write_offset) {
                return _size;
            } else if (_read_offset < _last_known_write_offset) {
                return _size  - (_last_known_write_offset - _read_offset);
            } else {
                return (_last_known_write_offset) + (_size - _read_offset);
            }
        };


        int16_t available() {
            _getWriteOffset(); // cache the write position
            return _getAvailableCached();
        };
    }; // RXBuffer



    /* TXBuffer<uint16_t _size, typename owner_type, typename base_type = char>
     * Implements a simple circular buffer, with a compile-time size, and can only be read from by DMA
     * owner_type is a *pointer* type that implements these methods:
     *   const base_type* getTXTransferPosition()
     *   void setTXTransferDoneCallback(std::function<void()> &&callback)
     *   bool startTXTransfer(char *&buffer, uint16_t length)
     */

    // Implement a simple circular buffer, with a compile-time size, and can only be read from by DMA
    // owner_type is a *pointer* type thet implements const base_type* getTXTransferPosition()
    template <uint16_t _size, typename owner_type, typename base_type = char>
    struct TXBuffer {
        static_assert(((_size-1)&_size)==0, "TXBuffer size must be 2^N");

        owner_type _owner;

        // Internal properties!
        base_type _data[_size+1];

        uint16_t _write_offset;             // The offset into the buffer of our next write
        uint16_t _last_known_read_offset;   // The offset into the buffer of the last known read (cached)

        uint16_t _transfer_requested = 0;   // keep track of how much we have requested. Non-zero means a request is active.

        constexpr int16_t size() { return _size; };

        TXBuffer(owner_type owner) : _owner(owner) { _data[_size] = 0; };

        void init() {
            _owner->setTXTransferDoneCallback([&]() { // use a closure
                _transfer_requested = 0;
                _restartTransfer();
            });
        }

        uint16_t _nextWriteOffset() {
            return (_write_offset + 1)&(_size-1);
        };

        bool _canBeWritten(uint16_t pos) {
            if (pos == _last_known_read_offset) {
                _getReadOffset();
                if (pos == _last_known_read_offset) {
                    _restartTransfer();
                    return false;
                }
            }
            return true;
        };

        uint16_t _getReadOffset() {
            base_type* pos = _owner->getTXTransferPosition();
            if (pos==nullptr) {
                _last_known_read_offset = 0;
            } else {
                _last_known_read_offset = (pos - _data) & (_size-1); // if it's one past the end, we want it to become zero
            }
            return _last_known_read_offset;
        }


        bool isLocked() { return false; } // this kind of buffer cannot be locked

        // We are transmitting. It will only get *more empty* as a transfer continues.

        bool isEmpty() {
            // If we were empty last time we checked, we are still empty.
            if (_isEmptyCached()) {
                return true;
            }

            // Update the cache and check again
            _getReadOffset();
            return _isEmptyCached();
        }

        bool isFull() {
            // If we were not full last time we checked, we are still not full.
            if (!_isFullCached()) {
                return false;
            }

            // Update the cache and check again
            _getReadOffset();
            return _isFullCached();
        }

        // It's empty if the write position would be the same as the read.
        bool _isEmptyCached() { return _write_offset == _last_known_read_offset; }

        // It's full if the next write position would be the same as the read.
        bool _isFullCached() { return _nextWriteOffset() == _last_known_read_offset; }

        void flush() {
            _restartTransfer();
        }

        void _restartTransfer() {
            if ((_transfer_requested == 0) && !isEmpty()) {
                // We can only request contiguous chunks. Let's see what the next one is.
                _getReadOffset(); // cache the read position

                int16_t transfer_size = 0;
                base_type *_read_pos = _data + _last_known_read_offset;

                // Possible cases:
                // [0] _read_pos == _write_pos
                //     The buffer is empty. We already eliminated that case.
                // [1] _read_pos > _write_pos
                //     IOW: We read to some position in the middle, and _write_pos is before it
                //          The unread data is between read->end, then 0->write.
                //          So, we transfer from _read_pos to the end of the buffer.
                // [2] _read_pos < _write_pos
                //     IOW: We read to some position in the middle, and _read_pos is in the range 0 through _write_pos.
                //          So, we can transfer from _read_pos to _write_pos.

                // Case [1]
                if (_last_known_read_offset > _write_offset) {
                    transfer_size = _size - _last_known_read_offset;

                // Case [2]
                } else {
                    transfer_size = _write_offset - _last_known_read_offset;
                }

                // We set _transfer_requested BEFORE startRXTransfer, in case an interrupt fires before we exit startRXTransfer
                //   (which should only happen if it started succesfully).
                // startRXTransfer will return false if it couldn't start the transfer.
                _transfer_requested = transfer_size;
                if (!_owner->startTXTransfer(_read_pos, transfer_size)) {
                    _transfer_requested = 0;
                }
            }
        };

        // BLOCKING write
        int16_t write(const char *buffer, size_t write_size) {
            uint16_t to_write = write_size;
            const char *src = buffer;
            while (to_write--) {
                if (isFull()) {
                    _restartTransfer();

                    // Wait until something has been read out
                    while (isFull()) {
                        ;
                    }
                }
                _data[_write_offset] = *src;

                src++;
                _write_offset = _nextWriteOffset();
            }

            _restartTransfer();

            return write_size;
        };

        // non-blocking write
        int16_t write_nb(const char *buffer, size_t write_size) {
            if (isFull()) {
                _restartTransfer();
                return -1;
            }

            uint16_t written = 0;
            uint16_t to_write = write_size;
            const char *src = buffer;
            while (to_write-- && !isFull()) {
                _data[_write_offset] = *src;

                src++;
                written++;
                _write_offset = _nextWriteOffset();
            }

            if (isFull()) {
                _restartTransfer();
            }

            return written;
        };


        int16_t _getAvailableCached() {
            if (_write_offset == _last_known_read_offset) {
                return _size;
            } else if (_write_offset < _last_known_read_offset) {
                return _size  - (_last_known_read_offset - _write_offset);
            } else {
                return (_last_known_read_offset) + (_size - _write_offset);
            }
        };
        
        
        int16_t available() {
            _getReadOffset(); // cache the write position
            return _getAvailableCached();
        };
    }; // TXBuffer
} // namespace Motate

#endif /* end of include guard: MOTATEBUFFER_H_ONCE */
