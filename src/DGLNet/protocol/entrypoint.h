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

#ifndef ENTRYPOINT_H
#define ENTRYPOINT_H

#include <DGLNet/protocol/anyvalue.h>

#include <DGLCommon/gl-entrypoints.h>

#include <boost/serialization/base_object.hpp>

#include <vector>

/**
 *  Class holding return value for entrypoint
 */
class RetValue : public AnyValue {
   public:
    RetValue() : m_isSet(false) {}

    RetValue(const RetValue& v)
            : AnyValue(*static_cast<const AnyValue*>(&v)), m_isSet(v.isSet()) {}

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<AnyValue>(*this);
        ;
        ar& m_isSet;
    }

    template <typename T>
    RetValue(T v)
            : AnyValue(v), m_isSet(true) {}

    static RetValue getVoidAlreadySet() {
        RetValue ret;
        ret.m_isSet = true;
        return ret;
    }

    bool isSet() const { return m_isSet; }

   private:
    bool m_isSet;
};

/**
 *  Class holding called entrypoint: entrypoint, parameters and return value
 */
class CalledEntryPoint {
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& m_args;
        ar& m_retVal;
        ar& m_entryp;
        ar& m_glError;
        ar& m_DebugOutput;
    }

   public:
    CalledEntryPoint() {}
    CalledEntryPoint(Entrypoint, int numArgs);
    Entrypoint getEntrypoint() const;
    void setRetVal(const RetValue& ret);
    void setError(gl_t error);
    void setDebugOutput(const std::string& message);

    const std::vector<AnyValue>& getArgs() const;
    template <typename T>
    void operator<<(const T& arg) {
        m_args[m_SavedArgsCount] = arg;
        m_SavedArgsCount++;
    }
    std::string toString() const;
    const RetValue& getRetVal() const;
    gl_t getError() const;
    const std::string& getDebugOutput() const;

   private:
    std::vector<AnyValue> m_args;
    RetValue m_retVal;
    Entrypoint m_entryp;
    gl_t m_glError;
    std::string m_DebugOutput;
    int m_SavedArgsCount;
};

#endif    // ENTRYPOINT_H
