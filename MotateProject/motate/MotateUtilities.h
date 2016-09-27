/*
 MotateUtilities.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2015-2016 Robert Giseburt

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

#ifndef MOTATEUTILITIES_H_ONCE
#define MOTATEUTILITIES_H_ONCE

#include <type_traits>
#include <utility>
#include <stdio.h>

// NOTES: Some functions are constexpr, but the compiler refuses to inline them when
// they aren't used with compile-time constants. More investigation is needed.

// Expect the API to these string manipulation functions (including their names)
// to be highly unstable at this point!!


namespace Motate {
    // We'll put these in the Private namespace for now, to indicate that they're private.
    namespace Private {
        //---- COMPILE-TIME STRING MANIPULATION ----//

    #pragma GCC diagnostic push
        // We are getting useless warning about the constexpr not being "inlined"
    #pragma GCC diagnostic ignored "-Winline"

        constexpr int c_strlen(const char *p, const int count_ = 0) {
            return !p
            ? 0
            : (
               (*p == 0)
               ? count_
               : c_strlen(p+1, count_+1)
               );
        }

        // It's assumed that the string buffer contains at lest count_ non-\0 chars
        constexpr int c_strreverse(char * const t, const int count_, char hold = 0) {
            return count_>1
                // Note: It always returns the count_, for a consistent interface.
                ? (hold=*t, *t=*(t+(count_-1)), *(t+(count_-1))=hold), c_strreverse(t+1, count_-2), count_
                : count_;
        }

        // TODO: Make c_floattoa a constexpr, which will be difficult.
        int c_floattoa(float in, char *buffer, int maxlen, int precision);


        // Returns the number of characters written (not counting the final \0), or 0 if it needed > maxlen.
        // NOTE: It will NOT replace the \0 at the beginning when it runs out of room!
        constexpr int c_itoa_backwards_(const int value, char *p, int maxlen, const int count_ = 0) {
            return (maxlen<1)
            ? (maxlen==1)?(*p=0,0):0
            : (
               (value>0 || count_==0)
                ? (*p = (value % 10)+'0'), // <- comma operator
                  c_itoa_backwards_((value/10), p+1, maxlen-1, count_+1)
                : count_
               );
        }
        constexpr int c_itoa(const int value, char *p, int maxlen, int count_ = 0) {
            return (value<0)
            ? (
               // We have to put the '-' before the area we reverse
               (*p = '-'), // <- comma operator
               c_strreverse(p+1, c_itoa_backwards_(-value, p+1, maxlen-1, 0))+1
               )
            : (
                c_strreverse(p, c_itoa_backwards_(value, p, maxlen, 0))
               );
        }

        // Returns the number of characters needed to wrint value, NOT counting the \0
        constexpr int c_itoa_len(int value, int length_ = 0) {
            return (value < 0)
            ? c_itoa_len(-value, length_+1)
            : (value > 0)
            ? c_itoa_len(value/10, length_+1)
            : length_+1;
        }

        // Copy at most max_len_ characters, returning how many was copied.
        // *(t+{return value}) = \0, guaranteed, *unless* max_len_ == 0
        constexpr int c_strcpy(char * const t, const char *f, int max_len_, const int count_ = 0) {
            return max_len_ <= 0 || !t || !f
            ? count_
            : max_len_==1
            ? *t = 0, count_+1
            : ( *t = *f ), // <- comma operator
            ( (*f == 0)
             ? count_
             : c_strcpy(t+1, f+1, max_len_-1, count_+1) );
        }

        //*** c_strcpyx_t special INTERNAL object ***//

        // Create the generic object, which will also server as the end of the list.
        template <typename... Ts>
        struct c_strcpyx_t {
            constexpr c_strcpyx_t(){;} // <-- NOTE the lack of Ts... here.
            // That forces a specialization to handle values, and only
            // allows this version to handle the "end" of the list.

            constexpr int32_t get_length() const {
                return 0;
            };

            constexpr int32_t copy_to(char * const dest_, int32_t max_len_) const {
                return *dest_=0,0;
            };
        };

        // Allows for const char* to be appended to the output.
        template <typename... Ts>
        struct c_strcpyx_t<const char *, Ts...> : c_strcpyx_t<Ts...>  {
            typedef c_strcpyx_t<Ts...> parent_t;
            const char * const val_;
            const int32_t length_;

            constexpr c_strcpyx_t(const char *value, Ts... ts) : c_strcpyx_t<Ts...>{ts...}, val_{value}, length_{c_strlen(val_)} {;}

            constexpr int32_t get_length() const {
                return length_ + parent_t::get_length();
            };

            constexpr int32_t copy_to(char * const dest_, int32_t max_len_, int32_t written_=0) const {
                return (
                    written_=c_strcpy(dest_, val_, max_len_),
                    (max_len_ < get_length())
                      ? written_
                      : written_+parent_t::copy_to((dest_+written_), (max_len_-written_))
                );
            };
        };


        // Allows for non-const char* to be appended to the output.
        template <typename... Ts>
        struct c_strcpyx_t<char *, Ts...> : c_strcpyx_t<Ts...>  {
            typedef c_strcpyx_t<Ts...> parent_t;
            const char * const val_;
            const int32_t length_;

            constexpr c_strcpyx_t(const char *value, Ts... ts) : c_strcpyx_t<Ts...>{ts...}, val_{value}, length_{c_strlen(val_)} {;}

            constexpr int32_t get_length() const {
                return length_ + parent_t::get_length();
            };

            constexpr int32_t copy_to(char * const dest_, int32_t max_len_, int32_t written_=0) const {
                return (
                        written_=c_strcpy(dest_, val_, max_len_),
                        (max_len_ < get_length())
                        ? written_
                        : written_+parent_t::copy_to((dest_+written_), (max_len_-written_))
                        );
            };
        };

        // Allows for int or const int to be appended to the output.
        template <typename int_type, typename... Ts>
        struct c_strcpyx_t<int_type, Ts...> : c_strcpyx_t<Ts...>  {
            typedef c_strcpyx_t<Ts...> parent_t;
            const int_type val_;

            constexpr c_strcpyx_t(int_type value, Ts... ts) : c_strcpyx_t<Ts...>{ts...}, val_{value} {;}

            constexpr int32_t copy_to(char * const dest_, int32_t max_len_, int32_t written_=0) const {
                return c_itoa(val_, dest_, max_len_)+parent_t::copy_to((dest_+written_), (max_len_-written_));
            };
        };

        // Internal function to construct a temporary c_strcpyx_t
        template <typename... Ts>
        constexpr c_strcpyx_t<Ts...> create_c_strcpyx_t(Ts... ts) {
            return {ts...};
        }

        //*** END c_strcpyx_t special INTERNAL object ***//


        //
        template <typename... Ts>
        constexpr int c_strcpy_multi(char * const dest_, int max_len_, Ts... ts) {
            return create_c_strcpyx_t(ts...).copy_to(dest_, max_len_);
        }


        /** string buffer object **/
        /** Goal: Create an object that can be passed around, that will carry all that's
                  needed about the buffer (char[x] style) as one variable.
        **/

        struct str_buf {
            char* const &b_;
            const int32_t l_;
            mutable int32_t w_ = 0;
            mutable int32_t r_ = 0;

            // typedef int rollback_pos_t;

            constexpr str_buf(char* const &b, int32_t l) : b_{b}, l_{l}, w_{0} {;}

            constexpr int get_written() const { return w_; };

            // constexpr rollback_pos_t get_pos() const { return w_; };

            // void set_pos(rollback_pos_t n) { w_ = n; b_[w_] = 0; }
            // return true so we can reserve a value on successful copy
            // bool reserve(int r) { r_ += r; return true; }

            // constexpr bool has_available(int x) const { return (l_-w_)>=x; };

            /*constexpr*/ bool record_copy_(int w) const {
                return ((w_ += w), (l_>w_));
            };
            template <typename int_type>
            /*constexpr*/ bool record_copy_zerofail_(int_type w) const {
                return ((w_ += w), (w>0) && (l_>w_));
            };

            // Copy at most max_len_ characters, returning false if it failed
            constexpr bool copy(const char *f) const {
                return (l_>w_)
                    ? record_copy_(c_strcpy(b_+w_, f, l_-w_-r_))
                    : false;
            };

            // Copy at most max_len_ characters, returning false if it failed
            constexpr bool copy(float f, uint8_t precision) const {
                return (l_>w_)
                    ? record_copy_zerofail_(c_floattoa(f, b_+w_, l_-w_-r_, precision))
                    : false;
            };

            // Copy at most max_len_ characters, returning false if it failed
            template <typename int_type>
            constexpr bool copy(int_type i) const {
                return (l_>w_)
                    ? record_copy_zerofail_(c_itoa(i, b_+w_, l_-w_-r_))
                    : false;
            };

            template <typename... Ts>
            constexpr bool copy_multi(Ts... ts) const {
                return record_copy_(create_c_strcpyx_t(ts...).copy_to(b_+w_, l_-w_-r_));
            };
        };

        // constexp atof = c_atof

        // Fraction part
        constexpr float c_atof_frac_(char *&p_, float v_, float m_) {
            return ((*p_ >= '0') && (*p_ <= '9'))
            ? (v_=((v_)+((*p_)-'0')*m_), c_atof_frac_(++p_, v_, m_/10.0))
            : v_;
        }

        // Integer part
        template <typename int_type>
        constexpr float c_atof_int_(char *&p_, int_type v_) {
            return (*p_ == '.')
            ? (float)(v_) + c_atof_frac_(++p_, 0, 1.0/10.0)
            : (
               ((*p_ >= '0') && (*p_ <= '9'))
               ? ((v_=((*p_)-'0')+(v_*10)), c_atof_int_(++p_, v_))
               : v_
               );
        }

        // Start portion
        constexpr float c_atof(char *&p_) {
            return (*p_ == '-')
            ? (c_atof_int_(++p_, 0) * -1.0)
            : (c_atof_int_(p_, 0));
        }


        namespace BitManipulation {

        // **** Bit manipulation ****
            /* Counts the trailing zero bits of the given value considered as a 32-bit integer. */
        constexpr uint32_t ctz(const uint32_t u) { return __builtin_ctz(u); };
            /* Counts the leading zero bits of the given value considered as a 32-bit integer. */
        constexpr uint32_t clz(const uint32_t u) { return __builtin_clz(u); };

        inline uint32_t Rd_bits(volatile const uint32_t &value, uint32_t mask) {
            return ((value) & (mask));
        };
        inline uint32_t Wr_bits(volatile uint32_t &lvalue, uint32_t mask, uint32_t bits) {
            return ((lvalue) = ((lvalue) & ~(mask)) | ((bits  ) &  (mask)));
        };
        inline bool Tst_bits(volatile const uint32_t &value, uint32_t mask) {
            return (Rd_bits(value, mask) != 0);
        };
        inline uint32_t Clr_bits(volatile uint32_t &lvalue, uint32_t mask) {
            return ((lvalue) &= ~(mask));
        };
        inline uint32_t Set_bits(volatile uint32_t &lvalue, uint32_t mask) {
            return ((lvalue) |=  (mask));
        };
        inline uint32_t Tgl_bits(volatile uint32_t &lvalue, uint32_t mask) {
            return ((lvalue) ^=  (mask));
        };
        inline uint32_t Rd_bitfield(volatile const uint32_t &value, uint32_t mask) {
            return (Rd_bits( value, mask) >> ctz(mask));
        };
        inline uint32_t Wr_bitfield(volatile uint32_t &lvalue, uint32_t mask, uint32_t bitfield) {
            return (Wr_bits(lvalue, mask, (uint32_t)(bitfield) << ctz(mask)));
        };

    }; // namespace BitManipulation

    } // Namespace Private

    // Non-constexpr version of strlen
    int strlen(const char *p);

    int streq(const char *p, const char *q, const size_t n);

    int strncpy(char * const t, const char *f, int max_len_);

    float atof(char *&buffer);

    // These convert to local byte order from the stated order
    static inline uint32_t fromBigEndian(volatile const uint32_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//        return ((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24);
        return __builtin_bswap32 (x);
#else
        return x;
#endif
    };
    static inline uint32_t fromLittleEndian(volatile const uint32_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return x;
#else
//        return ((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24);
        return __builtin_bswap32 (x);
#endif
    };

    static inline uint16_t fromLittleEndian(volatile const uint16_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return x;
#else
        return __builtin_bswap16 (x);
#endif
    };

    // These convert to the stated order from local byte order
    static inline uint32_t toBigEndian(volatile const uint32_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//        return ((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24);
        return __builtin_bswap32 (x);
#else
        return x;
#endif
    };
    static inline uint32_t toLittleEndian(volatile const uint32_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return x;
#else
//        return ((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24);
        return __builtin_bswap32 (x);
#endif
    };

#pragma GCC diagnostic pop
} // namespace Motate

#endif //MOTATEUTILITIES_H_ONCE
