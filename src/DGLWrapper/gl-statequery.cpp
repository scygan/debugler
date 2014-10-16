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


#include "gl-statequery.h"

namespace dglState {

namespace stateQuery {

    enum StateTableId {
#define DEFINE_TABLE(name, descr) name,
#include "gl-statequery-db.inl"
#undef DEFINE_TABLE
        TABLE_LAST
    };

    class StateTable {

    };

    class StateEntry {

    };



} //namespace stateQuery 
} //namespace dglState




namespace dglState {
namespace stateQuery {

    const StateTable g_StateTables[1] = {
        StateTable()
    };

} //namespace stateQuery 
} //namespace dglState



