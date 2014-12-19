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

#include "gl-statequery-db.h"

namespace stateQueryDB {

const struct StateDescr {
    char* name;
    GetterID getter;
    gl_t param;
    TableID tableId;
} g_StateDescr[] = {
#define DEFINE_STATE(name, profile, getter, table, count ) {#name, GetterID::getter, name, TableID::##table},
#define DEFINE_TABLE(name, descr)
#   include "gl-statequery-db.inl"
#undef DEFINE_TABLE
#undef DEFINE_STATE
};

const char* g_TableNames[] = {
#define DEFINE_STATE(name, profile, getter, table, count ) 
#define DEFINE_TABLE(name, descr) descr,
#include "gl-statequery-db.inl"
#undef DEFINE_TABLE
#undef DEFINE_STATE
};
   

void DataBase::DescribeState(StateID id, const char*& retName, TableID& retTableID) {
    if (id > StateID::STATE_ID_LAST) {
        throw std::runtime_error("Unknown state ID in DescribeState");
    }
    retName = g_StateDescr[(int)id].name;
    retTableID = g_StateDescr[(int)id].tableId;
}

void DataBase::GetStateParams(StateID id, GetterID& retGetterID, gl_t& retGetterParam) {
    if (id > StateID::STATE_ID_LAST) {
        throw std::runtime_error("Unknown state ID in DescribeState");
    }
    retGetterID = g_StateDescr[(int)id].getter;
    retGetterParam = g_StateDescr[(int)id].param;
}

void DataBase::DescribeTable(TableID id, const char*& retName) {
    if (id > TableID::TABLE_ID_LAST) {
        throw std::runtime_error("Unknown table ID in DescribeState");
    }
    retName = g_TableNames[(int)id];
}

};