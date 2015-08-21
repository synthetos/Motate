/*
 MotateJSON.cpp - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2015 Robert Giseburt

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

#include "MotateJSON.h"

#pragma GCC diagnostic push
// We are getting useless warning about the constexpr not being "inlined"
#pragma GCC diagnostic ignored "-Winline"


namespace Motate {
    namespace JSON {

        constexpr float round_lookup_[] = {
                0.5,          // precision 0
                0.05,         // precision 1
                0.005,        // precision 2
                0.0005,       // precision 3
                0.00005,      // precision 4
                0.000005,     // precision 5
                0.0000005,    // precision 6
                0.00000005,   // precision 7
                0.000000005,  // precision 8
                0.0000000005, // precision 9
                0.00000000005 // precision 10
            };

        /* ignore this ---
        constexpr c_floattoa_factional_(const float v_, char *p_, const int maxlen_, const int precision_) {
            return
                (v_ < 0.0)
                  ? c_floattoa_factional_(value + round_lookup_[precision]), p_+count_, maxlen_-count_, precision_)
                  : (

                  );
        }

        constexpr c_floattoa(const float v_, char *p_, const int maxlen_, const int precision_, int count_ = 0)
        {
            return
                (count_ = c_itoa(value + round_lookup_[precision]), p_, maxlen_),
                c_floattoa_factional_(value + round_lookup_[precision]), p_+count_, maxlen_-count_, precision_)
                ;
        };
        --- end ignore */

        int c_floattoa(float in, char *buffer, int maxlen, int precision) {
            int length_ = 0;
            char *b_ = buffer;

            if (in < 0.0) {
                *b_++ = '-';
                return c_floattoa(-in, b_, maxlen-1, precision) + 1;
            }

            // float round_ = 0.5;
            in = in + round_lookup_[precision];

            int int_length_ = 0;

            int integer_part_ = (int)in;

            while (integer_part_ > 0) {
                if (length_++ > maxlen) {
                    *buffer = 0;
                    return 0;
                }
                int t_ = integer_part_ / 10;
                *b_++ = '0' + (integer_part_ - (t_*10));
                integer_part_ = t_;
                int_length_++;
            }
            if (length_ > 0) {
                c_strreverse(buffer, int_length_);
            } else {
                *b_++ = '0';
                int_length_++;
            }

            *b_++ = '.';
            length_ = int_length_+1;

            float frac_part_ = in;
            frac_part_ -= (int)frac_part_;
            while (precision-- > 0) {
                if (length_++ > maxlen) {
                    *buffer = 0;
                    return 0;
                }
                frac_part_ *= 10.0;
                // if (precision==0) {
                //     t_ += 0.5;
                // }
                *b_++ = ('0' + (int)frac_part_);
                frac_part_ -= (int)frac_part_;
            }

            // reduce extra characters
            while (*(b_-1) == '0' && length_>1) {
                *(b_--) = 0;
                length_--;
            }

            if (*(b_-1) == '.') {
                *(b_--) = 0;
                length_--;
            }

            return length_;
        }

        //---- COMPILE-TIME STRING MANIPULATION ----//

    //    constexpr int c_strcmp(const char *p, const char *q, const size_t n) {
    //        return !n
    //        ? 0
    //        : (*p != *q)
    //        ? *p - *q
    //        : (!*p)
    //        ? 0
    //        : c_strcmp(p+1, q+1, n-1);
    //    }

        /*constexpr*/ int c_streq(const char *p, const char *q, const size_t n) {
            return
            (
             (!n || !p || !q)
             ? 1
             : (
                (*p != *q)
                ? 0
                : (
                   (!*p || !*q)
                   ? 1
                   : c_streq(p+1, q+1, n-1)
                   )
                )
             );
        }

    //    constexpr int c_strlen(const char *p, const int count_/* = 0*/) {
    //        return !p
    //        ? 0
    //        : (
    //           (*p == 0)
    //           ? count_
    //           : c_strlen(p+1, count_+1)
    //           );
    //    }

        // It's assumed that the string buffer contains at lest count_ non-\0 chars
    //    /*constexpr*/ int c_strreverse(char * const t, const int count_, char hold/* = 0*/) {
    //        return count_>1 ? (hold=*t, *t=*(t+(count_-1)), *(t+(count_-1))=hold), c_strreverse(t+1, count_-2), count_ : count_;
    //    }

        // Returns the number of characters written (not counting the final \0), or 0 if it needed > maxlen.
        // NOTE: It will NOT replace the \0 at the beginning when it runs out of room!
    //    /*constexpr*/ int c_itoa_backwards_(const int value, char *p, int maxlen, const int count_/* = 0*/) {
    //        return (maxlen<1)
    //        ? 0
    //        : (
    //           (value>0 || count_==0)
    //           ? (*p = (value % 10)+'0'), // <- comma operator
    //           c_itoa_backwards_((value/10), p+1, maxlen-1, count_+1)
    //           : count_
    //           );
    //    }
    //    /*constexpr*/ int c_itoa(const int value, char *p, int maxlen, int count_/* = 0*/) {
    //        return (value<0)
    //        ? (
    //           // We have to put the '-' before the area we reverse
    //           (*p = '-'), // <- comma operator
    //           (count_ = c_itoa_backwards_(value*-1, p+1, maxlen-1, 0)),
    //           c_strreverse(p+1, count_),
    //           p[count_+1]=0,
    //           count_+1
    //           )
    //        : (
    //           (count_ = c_itoa_backwards_(value, p, maxlen, 0)),
    //           c_strreverse(p, count_),
    //           p[count_]=0,
    //           count_
    //           );
    //    }

        // Returns the number of characters needed to wrint value, NOT counting the \0
    //    constexpr int c_itoa_len(int value, int length_/* = 0*/) {
    //        return (value < 0)
    //        ? c_itoa_len(value*-1, length_+1)
    //        : (value > 0)
    //        ? c_itoa_len(value/10, length_+1)
    //        : length_+1;
    //    }

        // Copy at most max_len_ characters, returning how many was copied.
        // *(t+{return value}) = \0, guaranteed, *unless* max_len_ == 0
    //    constexpr int c_strcpy(char * const t, const char *f, int max_len_, const int count_/* = 0*/) {
    //        return max_len_ <= 0 || !t || !f
    //        ? count_
    //        : max_len_==1
    //        ? *t = 0, count_+1
    //        : ( *t = *f ), // <- comma operator
    //        ( (*f == 0)
    //         ? count_
    //         : c_strcpy(t+1, f+1, max_len_-1, count_+1) );
    //    }

        //-----


    //    constexpr char *skip_whitespace(char *p_) {
    //        return (
    //                p_[0] == ' '
    //                || p_[0] == '\t'
    //                || p_[0] == '\r'
    //                || p_[0] == '\n'
    //                ) ? skip_whitespace(p_+1) : p_;
    //    }
    //
        // Point to the first character AFTER '"', and it will search until it find an
        // un-esacped '"', and will remove the escapes as it goes (using offset_, which
        // will be 0 or negative).
        /*constexpr*/ char *find_end_of_str_(char *p_, int offset_/* = 0*/) {
            return
            (p_[0] == '\\' && p_[1] == '"')
            ? ((p_[offset_-1] = p_[1]), find_end_of_str_(p_+2, offset_-1))
            : p_[0] != '"'
            ? ((p_[offset_] = p_[0]), find_end_of_str_(p_+1, offset_))
            : ((p_[offset_]=0),p_+offset_);
        }
    //
    //    constexpr char *find_end_of_name_bare_(char *p_) {
    //        return
    //        (
    //         ((p_[0] >= ('a')) && (p_[0] <= ('z'))) // lowercase letters
    //         || ((p_[0] >= ('0')) && (p_[0] <= ('9'))) // numbers
    //         || (p_[0] == '_') // underscore
    //         )
    //        ? find_end_of_name_bare_(p_+1) // continue the search
    //        : (p_[0] >= ('A')) && (p_[0] <= ('Z')) // uppercase letters
    //        ? p_[0] = 'a' + ('A' - p_[0]), // make it lowercase, and then
    //        find_end_of_name_bare_(p_+1) // continue the search
    //        : p_;
    //    }
    //
       /*constexpr*/ char *find_end_of_name(char *p_) {
            return (p_[0] == '"') ? find_end_of_str_(p_+1) : find_end_of_name_bare_(p_);
        }
    //
    //    // Fraction part
    //    constexpr float c_atof_frac_(char *&p_, float v_, float m_) {
    //        return ((*p_ >= '0') && (*p_ <= '9'))
    //        ? (v_=((v_)+((*p_)-'0')*m_), c_atof_frac_(++p_, v_, m_/10.0))
    //        : v_;
    //    }
    //
    //    // Integer part
    //    constexpr float c_atof_int_(char *&p_, int v_) {
    //        return (*p_ == '.')
    //        ? (float)(v_) + c_atof_frac_(++p_, 0, 1.0/10.0)
    //        : (
    //           ((*p_ >= '0') && (*p_ <= '9'))
    //           ? ((v_=((*p_)-'0')+(v_*10)), c_atof_int_(++p_, v_))
    //           : v_
    //           );
    //    }
    //
    //    // Start portion
    //    constexpr float c_atof(char *&p_) {
    //        return (*p_ == '-')
    //        ? (c_atof_int_(++p_, 0) * -1.0)
    //        : (c_atof_int_(p_, 0));
    //    }

    }// namespace JSON
}// namespace Motate

#pragma GCC diagnostic pop
