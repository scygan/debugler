/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef ANYVALUE_H
#define ANYVALUE_H

#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>

#include <DGLNet/serializer-fwd.h>
#include <DGLCommon/gl-types.h>

// Pointers that are serialized by value must be wrapped with this class
template <typename T>
class PtrWrap {
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& m_value;
    }

    typedef int64_t pointer_store_t;    // should be platform-independent,
                                        // rather than intptr_t
    static_assert(sizeof(int64_t) >= sizeof(intptr_t),
                  "Pointer sizes larger than 8 bytes");

   public:
    PtrWrap() : m_value(0) {}

    PtrWrap(intptr_t v) : m_value(static_cast<pointer_store_t>(v)) {}

    intptr_t getVal() const { return static_cast<intptr_t>(m_value); }

   private:
    pointer_store_t m_value;
};

class GLenumWrap {
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& m_value;
    }

   public:
    GLenumWrap() : m_value(0) {}

    GLenumWrap(gl_t v) : m_value(v) {}

    gl_t get() { return m_value; }
    operator GLenum() const { return static_cast<GLenum>(m_value); }

   private:
    gl_t m_value;
};

class AnyValue {
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& m_value;
    }

   public:
    AnyValue() {};
    template <typename T>
    AnyValue(T v)
            : m_value(v) {}

    template <typename TBase>
    AnyValue(const TBase* v)
            : m_value(PtrWrap<const void*>((intptr_t)v)) {}
    template <typename TBase>
    AnyValue(TBase* v)
            : m_value(PtrWrap<void*>((intptr_t)v)) {}

    template <typename T>
    void get(T& v) const {
        v = boost::get<T>(m_value);
    }

    template <typename TBase>
    void get(const TBase*& v) const {
        v = (const TBase*)boost::get<PtrWrap<const void*> >(m_value).getVal();
    }

    template <typename TBase>
    void get(TBase*& v) const {
        v = (TBase*)boost::get<PtrWrap<void*> >(m_value).getVal();
    }

    void writeToSS(std::ostringstream& out) const;

   private:
    boost::variant<signed long long, unsigned long long, signed long,
                   unsigned long, unsigned int, signed int, unsigned short,
                   signed short, unsigned char, signed char, float, double,
                   PtrWrap<void*>, PtrWrap<const void*>, GLenumWrap> m_value;
};

#endif    // ANYVALUE_H
