/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#ifndef _GL_STATE_QUERY_DB_H
#define _GL_STATE_QUERY_DB_H

#include "gl-types.h"

namespace stateQueryDB {


enum class StateID {
#define DEFINE_STATE(name, profile, getter, table, count ) name##_StateID,
#define DEFINE_TABLE(name, descr)
    #include "gl-statequery-db.inl"
#undef DEFINE_TABLE
#undef DEFINE_STATE
    STATE_ID_LAST
};


enum class TableID {
#define DEFINE_STATE(name, profile, getter, table, count ) 
#define DEFINE_TABLE(name, descr) name,
    #include "gl-statequery-db.inl"
#undef DEFINE_TABLE
#undef DEFINE_STATE
    TABLE_ID_LAST
};

enum class ContextProfile {
    CORE
};


enum class GetterID {
    GETTER_GetIntegerv
};

class DataBase {
public:
    
    static void DescribeState(StateID id, const char*& retName, TableID& retTableID);

    static void GetStateParams(StateID id, GetterID& retGetterID, gl_t& retGetterParam);

    static void DescribeTable(TableID id, const char*& retName);

};

};

#endif
