/*
 MotateUtilities.cpp - Library for the Motate system
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

#include "MotateUtilities.h"

#pragma GCC diagnostic push
// We are getting useless warning about the constexpr not being "inlined"
#pragma GCC diagnostic ignored "-Winline"


namespace Motate {

    // We'll put these in the Private namespace for now, to indicate that they're private.
    namespace Private {

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
                b_--; *b_ = 0;
                length_--;
            }

            if (*(b_-1) == '.') {
                b_--; *b_ = 0;
                length_--;
            }

            return length_;
        }

    } // namespace Private


    int strlen(const char *p) {
        return Private::c_strlen(p);
    }

    int streq(const char *p, const char *q, const size_t n) {
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
               : streq(p+1, q+1, n-1)
               )
            )
         );
    }

    float atof(char *&buffer) {
        return Private::c_atof(buffer);
    }

    int strncpy(char * const t, const char *f, int max_len_) {
        return Private::c_strcpy(t, f, max_len_);
    }


} // namespace Motate

#pragma GCC diagnostic pop
