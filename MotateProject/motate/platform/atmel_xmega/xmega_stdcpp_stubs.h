/*
 xmega_stdcpp_stubs.h - System-level routines to spport the Atmel XMega family
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

#ifndef Motate_xmega_stdcpp_stubs_h
#define Motate_xmega_stdcpp_stubs_h

// **************************
// Briefly: avr-gcc doesn't include the stl, at all.
// And, we only need enable_if, so here it is, yanked
// graciously from type_traits of gcc 4.8.3:

namespace std {
    //#include <type_traits>
    template<bool, typename _Tp = void>
    struct enable_if
    { };

    // Partial specialization for true.
    template<typename _Tp>
    struct enable_if<true, _Tp>
    { typedef _Tp type; };

    /// Define a member typedef @c type to one of two argument types.
    template<bool _Cond, typename _Iftrue, typename _Iffalse>
    struct conditional
    { typedef _Iftrue type; };

    // Partial specialization for false.
    template<typename _Iftrue, typename _Iffalse>
    struct conditional<false, _Iftrue, _Iffalse>
    { typedef _Iffalse type; };
} // end motate::std


#endif
