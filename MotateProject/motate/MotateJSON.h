/*
 MotateJSON.h - Library for the Motate system
 http://github.com/synthetos/motate/

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

#include <type_traits>
#include <utility>
#include <cstdint>
#include <cstdio>

#include "MotateUtilities.h"

namespace Motate {
    namespace JSON {

        // Import some functions into this namespace and save typing
        using Motate::Private::c_strlen;
        using Motate::Private::c_itoa_len;
        using Motate::atof;
        using Motate::Private::str_buf;
        using Motate::Private::c_strreverse;


        //---- COMPILE-TIME and RUN-TIME HASHING ----//

        namespace internal
        {
            // Credit to https://gist.github.com/filsinger/1255697 for inspiration
            template <typename S> struct fnv_internal;
            template <typename S> struct fnv1a;

            template <> struct fnv_internal<uint32_t>
            {
                constexpr static uint32_t default_offset_basis = 0x811C9DC5;
                constexpr static uint32_t prime                = 0x01000193;
            };

            template <> struct fnv1a<uint32_t> : public fnv_internal<uint32_t>
            {
                constexpr static inline uint32_t hash(char const*const aString, const uint32_t val = default_offset_basis)
                {
                    return (aString[0] == '\0') ? val : hash( &aString[1], ( val ^ uint32_t(aString[0]) ) * prime);
                }
// Unused
//                constexpr static inline uint32_t hash(char const*const aString, const size_t aStrlen, const uint32_t val = default_offset_basis)
//                {
//                    return (aStrlen == 0) ? val : hash( aString + 1, aStrlen - 1, ( val ^ uint32_t(aString[0]) ) * prime);
//                }
            };
        } // namespace internal

        typedef uint32_t hash_t;
        constexpr hash_t hash(const char * const aString) {
            return internal::fnv1a<uint32_t>::hash(aString);
        }


        //---- CONFIGURATION ITEMS ----//

        // Workaround some odd bug in gcc where it won't inline this unless it's close...
        namespace internal {
            constexpr int strlen(const char *p, const int count_ = 0)
            {
                return !p
                ? 0
                : (
                   (*p == 0)
                   ? count_
                   : strlen(p+1, count_+1)
                   );
            }
        }

        struct binderBase_t  {
            // Normal contructor
            constexpr binderBase_t() = default;

            // Copy constructor -- prevent copying
            constexpr binderBase_t(const binderBase_t&) = delete;

            // Move constructor -- we want this
            constexpr binderBase_t(binderBase_t&& other) = default;

            // Return a string verison
            virtual const char* get_str() const { return ""; };

            // Writing out to a buffer
            // We force this one to be defined
            virtual bool write(str_buf &buf, bool verbose = 0) const = 0;

            virtual bool write_json_prefix(str_buf &buf, bool verbose = 0) const { return true; };
            virtual bool write_json_suffix(str_buf &buf, bool verbose = 0) const { return true; };

            virtual bool write_json_in_between(str_buf &buf, bool verbose = 0) const {
                return buf.copy(",");
            };

            virtual bool write_json(str_buf &buf, bool verbose = 0) const {
                bool success_ =
                this->write_json_prefix(buf, verbose)
                && this->write(buf, verbose)
                && this->write_json_suffix(buf, verbose);

                return success_;
            };

            virtual void set(float) const {};
            virtual void set(bool) const {};


            virtual const binderBase_t *find(const char* s) const { return find(hash(s)); };
            virtual const binderBase_t *find(const hash_t h) const { return nullptr; };
            virtual const binderBase_t *find(int) const { return nullptr; };
        };



        // binderType_t is a proper *binder* that links a token to another variable.
        template <typename valueType>
        struct binderType_t : binderBase_t {
            const char *_token;
            const int _token_len;

            const char *_description;
            const int _description_len;

            const hash_t _hash;

            typedef valueType _type;
            _type &_value;

            constexpr binderType_t(const char *token, valueType& value, const char *description) :
                _token{token}, _token_len{internal::strlen(token)},
                _description{description}, _description_len{internal::strlen(description)},
                _hash{hash(token)},
                _value{value}
            {};

            constexpr binderType_t(const binderType_t&) = delete;
            constexpr binderType_t(binderType_t&&) = default;

            const char* get_str() const override { return ""; };

            // We're seeing a -Wfloat-equal failure here, even theough we assign, not compare.
            // So, we'll disable that check real quick.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
            // Best attempt at setting the value from a float...
            void set(float f) const override { _value = f; };
#pragma GCC diagnostic pop

            void set(bool f) const override { _value = (bool)f; };

            virtual bool write_json_prefix(str_buf &buf, bool verbose = 0) const {
                // pre-check if we'll fail: (TEMPORARILY DISABLED)
                // if (verbose) {
                //     if (!buf.has_available(token_string_len_+3)) {
                //         // we don't have enough room for '"TOKEN":' and a \0
                //         return false;
                //     }
                // } else {
                //     if (!buf.has_available(token_string_len_+1)) {
                //         // we don't have enough room for 'TOKEN:' and a \0
                //         return false;
                //     }
                // }

                if (verbose) {
                    return buf.copy_multi("\"", _token, "\":");
                } else {
                    return buf.copy_multi(_token, ":");
                }

                return true;
            };
        };


        // binderOwner_t (as opposed to a binderType_t) is a binder that links a token to value it *owns*.
        // binderOwner_t is used for non-leaf nodes in the tree, and is mainly used to own binderList_t<> objects.
        template <typename valueType>
        struct binderOwner_t : binderBase_t {
            const char *_token;
            const int _token_len;

            const char *_description;
            const int _description_len;

            const hash_t _hash;

            typedef valueType _type;
            valueType _value;

            constexpr binderOwner_t(const char *token, valueType&& value, const char *description) :
                _token{token}, _token_len{internal::strlen(token)},
                _description{description}, _description_len{internal::strlen(description)},
                _hash{hash(token)},
                _value{std::move(value)}
            {};

            constexpr binderOwner_t(const binderOwner_t&) = delete;
            constexpr binderOwner_t(binderOwner_t&&) = default;

//            constexpr binderOwner_t(binderOwner_t&& other)
//                : binderBase_t{std::move(other)},
//                  _token_c{other._token_c},
//                  _hash{other._hash},
//                  _description{other._description},
//                  _value{std::move(other._value)}
//            {};

            bool write(str_buf &buf, bool verbose) const override {
                return _value.write(buf, verbose);
            };

            virtual bool write_json_prefix(str_buf &buf, bool verbose = 0) const {
                // pre-check if we'll fail: (TEMPORARILY DISABLED)
                // if (verbose) {
                //     if (!buf.has_available(token_string_len_+3)) {
                //         // we don't have enough room for '"TOKEN":' and a \0
                //         return false;
                //     }
                // } else {
                //     if (!buf.has_available(token_string_len_+1)) {
                //         // we don't have enough room for 'TOKEN:' and a \0
                //         return false;
                //     }
                // }

                if (verbose) {
                    return buf.copy_multi("\"", _token, "\":");
                } else {
                    return buf.copy_multi(_token, ":");
                }

                return true;
            };


            // Pass find calls into the value...
            // Note this will trigger SFINAE -- valueType MUST be able to find(hash_t) and find(int)
            const binderBase_t *find(const hash_t h) const override
            {
                return _value.find(h);
            };

            // REPEAT: Note this will trigger SFINAE -- valueType MUST be able to find(hash_t) and find(int)
            const binderBase_t *find(int i) const override
            {
                return _value.find(i);
            };
        };

        // Array binder -- experimental
        // template <typename T, int N, class print_t = binderWriter_t<T>>
        // struct binderArrayType_t : binderBase_t, print_t {
        //     T (&value_)[N];
        //
        //     template<class T2, class... printSubTs>
        //     binderArrayType_t(T2 token, T (&value)[N], printSubTs... f) : binderBase_t{token}, print_t{f...}, value_{value} {} ;
        //
        //     const char* get_str() const override { return ""; };
        //     // Best attempt at setting the value from a float...
        //     void set_a(int i, float v) const override {
        //         value_[i] = v;
        //     };
        //
        //     virtual void print_json_prefix(bool verbose = 0) const {
        //         if (token_is_string_ && token_.c_ != nullptr) {
        //             verbose
        //             ? printf("\"%s\":", token_.c_)
        //             : printf("%s:", token_.c_);
        //         } else if (!token_is_string_) {
        //             verbose
        //             ? printf("[%d]\":", token_.i_)
        //             : printf("[%d]:", token_.i_);
        //         }
        //     };
        //     virtual void print_json_suffix(bool verbose = 0) const {
        //     };
        //
        //     void print(bool verbose) const override {
        //         if (token_is_string_ && token_.c_ != nullptr) {
        //             verbose
        //             ? printf("\":")
        //             : printf(":");
        //         }
        //         printf("[");
        //         for (int i = 0; i < N; i++) {
        //             print_t::print_(value_[i], verbose);
        //             if (i < N-1) {
        //                 printf(",");
        //             }
        //         }
        //         printf("]");
        //     };
        // };


//        template <typename valueType, typename parentType>
//        struct binderWriter_t : parentType {}; // stub
//

        template <typename valueType, typename parentType>
        struct binderWriter_t : parentType {
            // Say exclusively that we want to use _value, token_len, description_len from the parent
            using parentType::_value;

//            constexpr binderWriter_t(const char *token, valueType& value, const char *description)
//            : parentType{token, value, description}
//            {};

            constexpr binderWriter_t(const char *token, typename parentType::_type& value, const char *description) :
                parentType{token, value, description}
            {};

            constexpr binderWriter_t(const binderWriter_t&) = delete;
            constexpr binderWriter_t(binderWriter_t&&) = default;

            bool write(str_buf &buf, bool verbose) const override {
                return buf.copy(static_cast<valueType>(_value));
            };
        };

        template <typename parentType>
        struct binderWriter_t<float, parentType> : parentType {
            // Say exclusively that we want to use _value, token_len, description_len from the parent
            using parentType::_value;

            const int _precision = 4;

            constexpr binderWriter_t(const char *token, typename parentType::_type& value, const char *description, int precision) :
                parentType{token, value, description},
                _precision{precision}
            {};


            constexpr binderWriter_t(const binderWriter_t&) = delete;
            constexpr binderWriter_t(binderWriter_t&&) = default;

            bool write(str_buf &buf, bool verbose) const override {
                return buf.copy(static_cast<float>(_value), _precision);
            };
        };

//        template <>
//        struct binderWriter_t<float> {
//            const int precision_ = 4;
//
//            constexpr binderWriter_t(int precision) : precision_{precision} {};
//
//            bool write_(str_buf &buf, float value_, bool verbose = 0) const {
//                return buf.copy(value_, precision_);
//            };
//        };

        // template <class print_t>
        // struct binderType_t<float&, print_t> : binderBase_t, print_t {
        //     float &value_;
        //
        //     template<class T, class... printSubTs>
        //     constexpr binderType_t(T& token, float& value, printSubTs&... f) : binderBase_t{token}, value_{value}, print_t{f...} {};
        //
        //     const char* get_str() const override { return ""; };
        //     void set(float f) const override { value_ = f; };
        //
        //     bool write(str_buf &buf, bool verbose) const override { return print_t::write_(buf, value_, verbose); };
        // };


#if 0
        // We don't need const char * objects right now...
        template <>
        struct binderWriter_t<const char *> {
            constexpr binderWriter_t() {};

            // const char* get_str() const { return value_; };
            bool write_(str_buf &buf, const char * value_, bool verbose = 0) const {
                if (verbose) {
                    return buf.copy_multi("\"", value_,  "\"");
                } else {
                    return buf.copy(value_);
                }
            };
        };

        template <class print_t, uint16_t token_len, uint16_t description_len>
        struct binderType_t<const char *, print_t, token_len, description_len> : binderBase_t, print_t {
            const char* &value_;

            template<class... printSubTs>
            constexpr binderType_t(const char token[token_len] token, const char* &value, const char description[description_len], printSubTs... f)
                : binderBase_t{token, description},
                  value_{value},
                  print_t{f...}
            {};

            const char* get_str() const override { return value_; };

            bool write(str_buf &buf, bool verbose) const override { return print_t::write_(buf, value_, verbose); };
        };
#endif
#if 0
        template <>
        struct binderWriter_t<bool> {
            constexpr binderWriter_t() {};

            bool write_(str_buf &buf, bool value_, bool verbose = 0) const {
                if (verbose) {
                    return buf.copy_multi((value_ ? "true" : "false"));
                }
                return buf.copy_multi((value_ ? "t" : "f"));
            };
        };

        template <class print_t, uint16_t token_len, uint16_t description_len>
        struct binderType_t<bool&, print_t, token_len, description_len> : binderBase_t, print_t {
            bool &value_;

            template<class... printSubTs>
            constexpr binderType_t(const char token[token_len], bool& value, const char description[description_len], printSubTs... f)
                : binderBase_t{token, description},
                  value_{value},
                  print_t{f...}
            {};

            const char* get_str() const override { return value_ ? "true" : "false"; };
            void set(float f) const override { value_ = (bool)f; };

            bool write(str_buf &buf, bool verbose) const override { return print_t::write_(buf, value_, verbose); };
        };
#endif

        //----
        // binderList_t is the **value** type of a container of sub elements.
        // It **has** to be wrapped in a binderOwner_t<> to be used in a list.

        template <class... extraTypes>
        struct binderList_t {
            constexpr binderList_t(extraTypes&&... extras, bool is_array = false) {};

            constexpr binderList_t(const binderList_t&) = delete;
            constexpr binderList_t(binderList_t&& other)  = default;

            static const int rest_size_ = sizeof...(extraTypes);
            constexpr binderBase_t *find(const hash_t) const { return nullptr; }
            constexpr binderBase_t *find(int) const { return nullptr; }
            constexpr bool write(str_buf &buf, bool verbose) const { return true; };
        };

        template <class valueType, class... extraTypes>
        struct binderList_t<valueType, extraTypes...> : binderList_t<extraTypes...> {

            constexpr binderList_t(valueType&& value, extraTypes&&... extras, bool is_array) :
                binderList_t<extraTypes...>{std::move(extras)..., is_array},
                _value{std::move(value)},
                _array{is_array}
            {};

            constexpr binderList_t(const binderList_t&) = delete;
//            constexpr binderList_t(binderList_t&&) = default;

            constexpr binderList_t(binderList_t&& other) :
                binderList_t<extraTypes...>{std::move(other)},
                _value{std::move(other._value)},
                _array{other._array}
            {};

            static const int rest_size_ = sizeof...(extraTypes);

            typedef binderList_t<valueType, extraTypes...> this_t;
            typedef binderList_t<extraTypes...> parent_t;
            typedef valueType type;
            valueType _value;
            bool _array = false;

            // Workaround some odd bug in gcc where it won't inline this unless it's close...
//            constexpr int internal_strlen(const char *p, const int count_ = 0) const
//            {
//                return !p
//                ? 0
//                : (
//                   (*p == 0)
//                   ? count_
//                   : internal_strlen(p+1, count_+1)
//                );
//            }
//
//            constexpr bool matches(const char* s) const
//            {
//                return streq(value_.token_.c_, s, internal_strlen(s));
//            }

            constexpr bool matches(const hash_t h) const
            {
                return (_value._hash == h);
            }

            constexpr const binderBase_t *find(const char* s) const
            {
                return find(hash(s));
            };

            constexpr const binderBase_t *find(const hash_t h) const
            {
                return (matches(h) ? &_value : parent_t::find(h));
            };

            constexpr const binderBase_t *find(const int i) const
            {
                return (i == 0 ? &_value : parent_t::find(i-1));
            };

            bool write(str_buf &buf, bool verbose = 0) const {
                bool success_ = true;
                // auto p_ = buf.get_pos();
                if (!_array) {
                    success_ = _value.write_json_prefix(buf, verbose);
                }
                success_ = success_ && _value.write(buf, verbose);
                if (success_ && rest_size_ > 0) {
                    // auto p2_ = buf.get_pos();
                    bool success2_ = _value.write_json_in_between(buf, verbose);
                    success2_ = success2_ && parent_t::write(buf, verbose);
                    // if (!success2_) {
                    //     buf.set_pos(p2_);
                    // }
                }
                success_ && _value.write_json_suffix(buf, verbose);
                return success_;
            };

            constexpr bool isArray() const {
                return _array;
            };
        };


        template <class... subTypes>
        struct binderOwner_t<binderList_t<subTypes...>> : binderBase_t {
            const char *_token;
            const int _token_len;

            const char *_description;
            const int _description_len;

            const hash_t _hash;

            typedef binderList_t<subTypes...> _type;
            binderList_t<subTypes...> _value;
            bool _array = false;

            constexpr binderOwner_t(const char *token, _type&& value, const char *description) :
                _token{token}, _token_len{internal::strlen(token)},
                _description{description}, _description_len{internal::strlen(description)},
                _hash{hash(token)},
                _value{std::move(value)},
                _array{value.isArray()}
            {};

            constexpr binderOwner_t(const binderOwner_t&) = delete;
            constexpr binderOwner_t(binderOwner_t&&) = default;

//            constexpr binderOwner_t(binderOwner_t&& other)
//                : binderBase_t{std::move(other)},
//                  _token_c{other._token_c},
//                  _hash{other._hash},
//                  _description{other._description},
//                  _value{std::move(other._value)},
//                  _array{other._array}
//            {};

            const binderBase_t *find(const char* s) const override
            {
                return _value.find(s);
            };

            const binderBase_t *find(int i) const override
            {
                return _value.find(i);
            };

            // So this gets tricky: If we are printing a part of an array, we want the results to look like:
            //    "token_[1]":... or token[1]:..., where ... is JUST the value, and will likely NOT be wrapped in []

            bool write_json_prefix(str_buf &buf, bool verbose = 0) const override
            {
                if (_array) {
                    if (verbose) {
                        return buf.copy_multi("\"", _token);
                    } else {
                        return buf.copy(_token);
                    }
                } else {
                    if (verbose) {
                        return buf.copy_multi("\"", _token, "\":{");
                    } else {
                        return buf.copy_multi(_token, ":{");
                    }
                }

                // If we get here, all is okay.
                return true;
            };

            bool write(str_buf &buf, bool verbose) const override
            {
                bool success_ = true;
                success_ = success_ && _value.write(buf, verbose);
                return success_;
            };

            bool write_json_suffix(str_buf &buf, bool verbose = 0) const override
            {
                if (!_array && _token != nullptr) {
                    return buf.copy("}");
                }
                return true;
            };
        };

        // Experiment:
        // template <class S, class... Ss>
        // auto find(S *v, const char* s, Ss... s2)  -> decltype( find(v->find(s), s2...) ) {
        //     return find(v->find(s), s2...);
        // };
        //
        // template <class S>
        // auto find(S *v, const char* s) -> decltype( v->find(s) ) {
        //     return v->find(s);
        // };

        //----


        // Example: lengthUnits -- may need updated
        //    extern bool inches;
        //
        //    struct lengthUnits_t {
        //        float &mm_;
        //        lengthUnits_t(float &v) : mm_(v) {};
        //        operator float() { return inches ? mm_ / 2.54 : mm_; };
        //        float operator=(const float v) { return (mm_ = (inches ? v * 2.54 : v)); };
        //    };
        //
        //
        //    template <>
        //    struct binderWriter_t<lengthUnits_t> {
        //        const char *f_;
        //
        //        constexpr binderWriter_t(const char *f) : f_{f} {};
        //
        //        bool write_(str_buf &buf, float value_, bool verbose = 0) const {
        //            // printf("\"");
        //            // printf(f_, value_);
        //            // printf(inches ? "in" : "mm");
        //            // printf("\"");
        //            return true;
        //        };
        //        // void set(float f) const { value_ = f; };
        //    };

        // template <class print_t>
        // struct binderType_t<lengthUnits_t, print_t> : binderBase_t, print_t {
        //     lengthUnits_t &value_;
        //
        //     template<class... printSubTs>
        //     binderType_t(const char *token, lengthUnits_t& value, printSubTs... f) : binderBase_t{token}, print_t{f...}, value_{value} {} ;
        //     const char* get_str() const { return ""; };
        //
        //     void print(bool verbose) const { print_t::print_(value_, verbose); };
        // };
        //


        //----


    // Example: writer that multiplies the output value by a specific amount
    // Better: Use a property getter/setter
    //    struct multipliedWriter_t {
    //        const char *f_;
    //        const float multiplier_;
    //
    //        constexpr multipliedWriter_t(const char *f, double multiplier) : f_{f}, multiplier_{(float)multiplier} {};
    //
    //        template<typename T>
    //        bool write_(str_buf &buf, T& value_, bool verbose = 0) const {
    //            // printf("\"");
    //            // printf(f_, ((double)value_) * multiplier_);
    //            // printf(inches ? "in" : "mm");
    //            // printf("\"");
    //            return true;
    //        };
    //        // void set(float f) const { value_ = f; };
    //    };





        //----


        // bindt_ is a shorthand used by the bind factories
        template <typename valueType>
        using bindt_ = binderWriter_t<valueType, binderType_t<valueType> >;



        template <typename valueType, typename... extraParamTypes>
        constexpr bindt_<valueType>
        bind(const char *token, valueType& valueRef, const char *description, extraParamTypes... extraParams) {
            return {token, valueRef, description, extraParams...};
        }


        template <typename subBinderType, typename... extraParamTypes>
        constexpr binderOwner_t<subBinderType>
        bind(const char *token, subBinderType&& subBinder, const char *description, extraParamTypes... extraParams) {
            return {token, std::move(subBinder), description, extraParams...};
        }

        // Experiment: Array-type binder
        // template <typename Token_t, typename T, int N, typename... Ts>
        // binderArrayType_t<T, N, binderWriter_t<T>> bind(Token_t token, T(& t)[N], Ts&... ts) {
        //     return {token, t, ts...};
        // }


        // bind a JSON object, with the contents being other binders...
        // First, we make a private helper for the list
        namespace internal {
            template <class... subBinderTypes>
            constexpr binderList_t<subBinderTypes...>
            bind_object(subBinderTypes&&... subBinders)  {
                return {std::move(subBinders)..., /*isArray:*/false};
            };

            template <class... subBinderTypes>
            constexpr binderList_t<subBinderTypes...>
            bind_array(subBinderTypes&&... subBinders)  {
                return {std::move(subBinders)..., /*isArray:*/true};
            };
        }

        template <class... subBinderTypes>
        constexpr binderOwner_t<binderList_t<subBinderTypes...>>
        bind_object(const char *token, const char *description, subBinderTypes&&... subBinders)  {
            return bind(token, internal::bind_object(std::move(subBinders)...), description);
        };

//        // bind a JSON array, with the contents being other binders (with no names)...
//        template <uint16_t token_len, uint16_t description_len, class... subBinderTypes>
//        constexpr binderOwner_t<binderList_t<subBinderTypes...>, token_len, description_len>
//        bind_array(const char (&token)[token_len], const char (&description)[description_len], subBinderTypes&&... subBinders)  {
//            return bind(static_cast<const char (&)[token_len]>(token), Private::bind_array(std::move(subBinders)...), static_cast<const char (&)[description_len]>(description));
//        };


        // bind_no_name is to bind an object with an empty name, such as in an array
        template <typename valueType, typename... extraParamTypes>
        constexpr bindt_<valueType>
        bind_no_name(valueType& valueRef, const char *description, extraParamTypes... extraParams) {
            return {nullptr, valueRef, description, extraParams...};
        }

        template <typename subBinderType, typename... extraParamTypes>
        constexpr binderOwner_t<subBinderType>
        bind_no_name(subBinderType&& subBinder, const char *description, extraParamTypes... extraParams) {
            return {"", std::move(subBinder), description, extraParams...};
        }

        // parent is a bind_no_name(bind_object())
        template <typename... subBinderTypes>
        constexpr binderOwner_t<binderList_t<subBinderTypes...>>
        parent(const char *description, subBinderTypes&&... subBinders) {
            return bind_no_name(internal::bind_object(std::move(subBinders)...), description);
        }

//        template <typename T, typename... Ts>
//        constexpr bindt_<T> bind_no_name(T&& t, Ts&&... ts) {
//            return {nullptr, t, ts...};
//        }

        // Experiment: Array-type binder (no-name version)
        // template <typename T, int N, typename... Ts>
        // binderArrayType_t<T, N, binderWriter_t<T>> bind_no_name(T(& t)[N], Ts&... ts) {
        //     return {nullptr, t, ts...};
        // }

        template <typename writerType, typename valueType>
        using bind_type_ = binderWriter_t<writerType, binderType_t<valueType>>;

        // Experiment: Array-type binder factory
        // template <typename Token_t, typename T, int N, typename P, typename... Ts>
        // binderArrayType_t<T, N, P> cfgap_(Token_t& token, T(& t)[N], Ts&... ts) {
        //     return {token, t, ts...};
        // }

        // Binders that allow you to specify the type of the writer explicitly
        template <typename writerType, typename valueType, typename... extraTypes>
        constexpr bind_type_<writerType, valueType>
        bind_typed(const char *token, valueType& t, const char *description, extraTypes... extras) {
            return {token, t, description, extras...};
        }


        //----


        /*
        {G5:{u:[0,1,2]}} ->
        {
          {name:"G5", value:null, type:parent, depth:0},
          {name:"u", value:null, type:array, depth:1},
          {name:null, value:0, type:number, depth:2},
          {name:null, value:1, type:number, depth:2},
          {name:null, value:2, type:number, depth:2},
          {name:null, value:null, type:null, depth:0}
        }

        */

        struct instruction_t {
            union search_t {
                int i_;
                search_t(int i) : i_(i) {};

                char* c_;
                search_t(char * c) : c_(c) {};
            } search_; // reserve a pointer to the data itself

            enum {
                kIsToken,
                kIsIndex
            } index_type_;

            enum {
                kEmpty,
                kNull,
                kNumber,
                kString,
                kBool,
                kArray,
                kParent,   // Parent menas there are children opcodes
                kContainer // vs. a Container, where the payload is a string
            } type_ = kEmpty;

            union value_t {
                float f_;
                value_t(float f) : f_(f) {};

                bool b_;
                value_t(bool b) : b_(b) {};

                char* c_;
                value_t(char * c) : c_(c) {};
            } value_; // reserve a pointer to the data itself

            int depth_ = 0;

            void clear() {
                search_.c_ = nullptr;
                index_type_ = kIsToken;
                type_ = kEmpty;
                value_.c_ = nullptr; // set c_ to ensure all zeros
                depth_ = 0;
            };

            void setToken(char *t) {
                search_.c_ = t;
                index_type_ = kIsToken;
            };

            char* getToken() const {
                return index_type_ == kIsToken
                 ? search_.c_
                 : nullptr;
            };

            void setIndex(int i) {
                search_.i_ = i;
                index_type_ = kIsIndex;
            };

            int getIndex() const {
                return index_type_ == kIsIndex
                    ? search_.i_
                    : -1;
            };

            const binderBase_t *findIn(const binderBase_t *t_) const {
                return index_type_ == kIsToken
                    ? t_->find(search_.c_)
                    : t_->find(search_.i_);
            };

            void setFloat(float in) {
                value_.f_ = in;
                type_ = kNumber;
            };

            bool getIsNumber() const { return type_ == kNumber; }
            int getFloat(float &out) const {
                if (type_ == kNumber) {
                    out = value_.f_;
                    return 0;
                }
                return 1;
            };

            void setBool(bool in) {
                value_.b_ = in;
                type_ = kBool;
            };

            bool getIsBool() const { return type_ == kBool; }
            int getBool(bool &out) const {
                if (type_ == kBool) {
                    out = value_.b_;
                    return 0;
                }
                return 1;
            };

            void setStr(char* in) {
                value_.c_ = in;
                type_ = kString;
            };

            bool getIsStr() const { return type_ == kString; }
            int getStr(char* &out) const {
                if (type_ == kString) {
                    out = value_.c_;
                    return 0;
                }
                return -1;
            };

            bool getIsEmpty() const { return type_ == kEmpty; }

            void setIsContainer() { type_ = kContainer; }
            bool getIsContainer() const { return type_ == kContainer; }
            void setIsParent() { type_ = kParent; }
            bool getIsParent() const { return type_ == kParent; }
            void setIsArray() { type_ = kArray; }
            bool getIsArray() const { return type_ == kArray; }
            void setIsNull() { type_ = kNull; }
            bool getIsNull() const { return type_ == kNull; }
            void setDepth(int d) { depth_ = d; }
            int  getDepth() const { return depth_; }

            instruction_t() : search_{nullptr}, index_type_{kIsToken}, type_{kEmpty}, value_{nullptr}, depth_{0} {};
            instruction_t(char *token) : search_{token}, type_{kNull}, value_{nullptr}, depth_{0} {};
            instruction_t(char *token, float f) : search_{token}, type_{kNumber}, value_{f} {};
            instruction_t(char *token, char* c) : search_{token}, type_{kString}, value_{c} {};
            instruction_t(char *token, bool b) : search_{token}, type_{kBool}, value_{b} {};

            instruction_t(int index) : search_{index}, type_{kNull}, value_{nullptr} {};
        };


        //-----

        extern bool relaxed_json;

        template<int max_stack_depth = 25>
        struct instruction_stack_t {
            int stack_ptr = -1;
            int in_array = false;
            bool is_written_ = false;

            struct ins_stackitem_t {
                const binderBase_t *t_ = nullptr;
                bool is_array_ = false;
                int index_ = -1;
                bool is_written_ = false;
            } op_stack[max_stack_depth];

            void push(const binderBase_t *t, bool isArray) {
                stack_ptr++;
                op_stack[stack_ptr].t_ = t;
                op_stack[stack_ptr].is_array_ = isArray;
                op_stack[stack_ptr].index_ = -1;
                op_stack[stack_ptr].is_written_ = false;

                in_array = isArray;
            };


            const binderBase_t *current() {
                return op_stack[stack_ptr].t_;
            };

            bool exec(instruction_t& op, int depth_offset = 0) {
                setDepth_(op.getDepth() + depth_offset);

                const binderBase_t *tok = op.findIn(op_stack[stack_ptr].t_);
                if (op.getIsParent()) {
                    push(tok, /*isArray*/ false);
                    return false;
                }

                if (op.getIsArray()) {
                    push(tok, /*isArray*/ true);
                    return false;
                }

                if (op.getIsNull()) {
                    return true;
                }

                if (op.getIsNumber()) {
                    float tmp = 0;
                    if (!op.getFloat(tmp)) {
                        tok->set(tmp);
                        op.setIsNull();
                        return true;
                    }
                    return false;
                }

                return false;
            };


            bool setDepth_(int depth, str_buf *buf = nullptr) {
                if (stack_ptr == depth) {
                    (op_stack[stack_ptr].index_)++;
                }

                while (stack_ptr > depth) {
                    if (buf != nullptr && op_stack[stack_ptr].is_written_ && !op_stack[stack_ptr].is_array_) {
                        bool success_ = op_stack[stack_ptr].t_->write_json_suffix(*buf, !relaxed_json);
                        if (!success_) {
                            // UNROLL from here!!
                            return false;
                        }
                    }

                    // pop the top off the stack, and reset it
                    op_stack[stack_ptr].t_ = nullptr;
                    op_stack[stack_ptr].is_array_ = false;
                    op_stack[stack_ptr].index_ = -1;
                    op_stack[stack_ptr].is_written_ = false;

                    --stack_ptr;
                }

                if (depth > 0) {
                    if (buf != nullptr && op_stack[stack_ptr].is_written_ && !op_stack[stack_ptr].is_array_) {
                        bool success_ = op_stack[stack_ptr].t_->write_json_suffix(*buf, !relaxed_json);
                        if (!success_) {
                            // UNROLL from here!!
                            return false;
                        }
                    }
                    op_stack[stack_ptr].is_written_ = false;
                }

                if (stack_ptr > 0) {
                    in_array = op_stack[stack_ptr].is_array_;
                } else {
                    in_array = false;
                }
                return true;
            };

            bool finish_writing(str_buf &buf, int depth_offset = 0, bool close = false) {
                bool success_ = setDepth_(depth_offset, &buf);

                if (success_ && is_written_ && close) {
                    success_ = buf.copy("}");
                }

                return success_;
            };

            bool write(instruction_t& op, str_buf &buf, int depth_offset = 0) {
                bool success_ = true;

                success_ = setDepth_(op.getDepth() + depth_offset, &buf);
                if (!success_) {
                    return false;
                }

                const binderBase_t *tok = op.findIn(op_stack[stack_ptr].t_);

                if (tok == nullptr) {
                    // tok is missing!

                }

                if (op.getIsParent()) {
                    push(tok, /*isArray*/ false);
                    return true;
                }

                if (op.getIsArray()) {
                    push(tok, /*isArray*/ true);
                    return true;
                }

                if (op.getIsNull()) {
    //                bool success_ = true;

                    if (op_stack[stack_ptr].is_written_) {
                            success_ = buf.copy(",");
                    } else
                    {
                        if (!is_written_) {
                            success_ = success_ && buf.copy("{");
                        }
                        bool last_level_was_written = is_written_;
                        for (int written_depth = 0; written_depth <= stack_ptr; written_depth++) {

                            if (!op_stack[written_depth].is_written_) {
                                if (last_level_was_written) {
                                    success_ = success_ && buf.copy(",");
                                }

                                // We use buf_len-written_-(stack_ptr*2) to reserve enough room to close all of the stack
                                success_ = success_ && op_stack[written_depth].t_->write_json_prefix(buf, !relaxed_json);

                                // if (op_stack[written_depth].is_array_) {
                                //     op_stack[written_depth].is_written_ = false;
                                // } else {
                                    op_stack[written_depth].is_written_ = true;
                                    is_written_ = true;
                                // }
                                last_level_was_written = false;
                            } else {
                                last_level_was_written = true;
                            }
                        }
                    }

                    if (tok == nullptr) {
                        // tok is missing!
                        if (op.getToken() != nullptr) {
                            if (!relaxed_json) {
                                success_ = success_ && buf.copy_multi(op.getToken(), ":null");
                            } else {
                                success_ = success_ && buf.copy_multi("\"", op.getToken(), "\":null");
                            }
                        }
                    } else {
                        success_ = success_ && tok->write_json(buf, !relaxed_json);
                    }
                    if (!success_) {
                        return false;
                    }

                }

                return true;
            };
        };


        //-----


        template<int size_>
        struct instruction_list_t {
            instruction_t ins_[size_];
            static const int kSize_ = size_;
            int pos_ = 0;

//            constexpr instruction_list_t() {;}; //<-- this appears to be the default, and causes arm-none-eabi-gcc to squak about inlining failure
            instruction_list_t() {;};

            instruction_t& getCurrent() {
                return ins_[pos_];
            };

            bool next() {
                pos_++;
                if (pos_ == size_) {
                    return false;
                } else {
                    return true;
                }
            };

            void reset() {
                while (pos_ != 0) {
                    ins_[pos_--].clear();
                }
            };

            bool exec(const binderBase_t *b_) {
                instruction_stack_t<> op_stack;

                op_stack.push(b_, /*isArray*/ false);
                bool would_print_ = false;

                for (int i = 0; i<size_ /*&& !o[i].getIsEmpty()*/; ++i) {
                    would_print_ = op_stack.exec(ins_[i], /*print:*/ false) | would_print_;
                }

                // op_stack.finish();

                return would_print_;
            };

            template<int tail_size_=0>
            int write(const binderBase_t *b_, char *buffer, int buf_len, const binderBase_t *r_ = nullptr, const binderBase_t *f_ = nullptr, instruction_list_t<tail_size_> *tail_ = nullptr) {
                instruction_stack_t<> op_stack;
                int depth_offset = 0;
                if (r_) {
                    op_stack.push(r_, /*isArray*/ false);
                    depth_offset = 1;
                }
                op_stack.push(b_, /*isArray*/ false);

                str_buf buf{buffer, buf_len-1};

                bool success_ = true;
                for (int i = 0; (i<size_) && success_; ++i) {
                    success_ = op_stack.write(ins_[i], buf, depth_offset);
                }

                if (f_ && tail_ && success_) {
                    success_ = op_stack.finish_writing(buf, -1, /*close=*/false);

                    op_stack.push(f_, /*isArray*/ false);
                    for (int i = 0; (i<tail_->kSize_) && success_; ++i) {
                        success_ = op_stack.write(tail_->ins_[i], buf, 1);
                    }
                }

                success_ = op_stack.finish_writing(buf, -1, /*close=*/true);

                return success_ ? buf.get_written() : 0;
            };
        };



        //-----


        constexpr char *skip_whitespace(char *p_) {
            return (
                    p_[0] == ' '
                    || p_[0] == '\t'
                    || p_[0] == '\r'
                    || p_[0] == '\n'
                    ) ? skip_whitespace(p_+1) : p_;
        }

        // Point to the first character AFTER '"', and it will search until it find an
        // un-esacped '"', and will remove the escapes as it goes (using offset_, which
        // will be 0 or negative).
        /*constexpr*/ char *find_end_of_str_(char *p_, int offset_ = 0);
        constexpr char *find_end_of_name_bare_(char *p_) {
            return
                (
                 ((p_[0] >= ('a')) && (p_[0] <= ('z'))) // lowercase letters
                 || ((p_[0] >= ('0')) && (p_[0] <= ('9'))) // numbers
                 || (p_[0] == '_') // underscore
                 )
                ? find_end_of_name_bare_(p_+1) // continue the search
                : (p_[0] >= ('A')) && (p_[0] <= ('Z')) // uppercase letters
                    ? p_[0] = 'a' + ('A' - p_[0]), // make it lowercase, and then
                      find_end_of_name_bare_(p_+1) // continue the search
                    : p_;
        }

        /*constexpr*/ char *find_end_of_name(char *p_);

        //-----

        template <int s>
        bool parse_json_object_(instruction_list_t<s> &o_, char *b_, int depth_) {
            char *key_start = nullptr;
            char *p_ = b_;

            if (p_[0] != '{') {
                // FAILURE: object doesn't start with a '{'
                return false;
            }

            do {
                // Method of operation:
                // If we parse something in this loop, continue back tot he top of the loop.
                // If we hit the bottom of the loop, we have not found anything and will error to avoid infinite loops.

                if (p_[0] == '{') {
                    depth_++;

                    // o_.getCurrent().setFloat(35.0);
                    o_.getCurrent().setIsParent();

                    // DON'T p_++ here, catch it in the next if.
                    // DON'T continue here, the next if will catch this case too.
                }

                // Other than the depth_ stuff, '{' and ',' are handled the same
                if ((p_[0] == '{') || (p_[0] == ',')) {
                    p_++;
                    key_start = p_;

                    o_.next();
                    o_.getCurrent().setDepth(depth_);

                    // find (but not zero) the end of the name
                    // side-effect: will lowercase characters
                    p_=find_end_of_name(p_);
                    if (p_[0] == ':') {
                        p_[0]=0;
                        if (*key_start == '"') {
                            key_start++;
                        }
                        o_.getCurrent().setToken(key_start);
                        p_++;

                    } else {
                        // FAILURE: non ':' after a key
                        return false;
                    }

                    continue;
                }

                // n or NULL
                if (p_[0] == 'n' || p_[0] == 'N') {
                    p_++;
                    if (
                        ((p_[0] == 'u') || (p_[0] == 'U'))
                     && ((p_[1] == 'l') || (p_[1] == 'L'))
                     && ((p_[2] == 'l') || (p_[2] == 'L'))
                    ) {
                        p_ += 3;
                    }
                    o_.getCurrent().setIsNull();
                    continue;
                }

                // number -0-9.
                if (
                    ((p_[0] == '-') && ((p_[1] == '.') || ((p_[1] >= '0') && (p_[1] <= '9'))))
                    || ((p_[0] == '.') || ((p_[0] >= '0') && (p_[0] <= '9')))
                   ) {

                    // side-efffect: changes p_ to point to first non-number char
                    float v_ = atof(p_);

                    o_.getCurrent().setFloat(v_);
                    continue;
                }

                if (p_[0] == '}') {
                    depth_--;
                    if (depth_ < 0) {
                        // FAILURE: too many '}' characters
                        return false;
                    }
                    p_++;
                     continue;
                }

                // whitespace can be skipped
                if (p_[0] == ' ' || p_[0] == '\t' || p_[0] == '\t' || p_[0] == '\n') {
                    p_++;
                    continue;
                }

                // parser error!
                // TODO: handle this better
                return false;
            } while(1);

            return true;
        }

        template <int s>
        bool parse_json(instruction_list_t<s> &o_, char *b_) {
            o_.reset();
            char *p_ = skip_whitespace(b_);
            return parse_json_object_(o_, p_, /*depth_=*/-1);
        }
    }
} // namespace