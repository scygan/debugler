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
#include "pointers.h"

#include <DGLCommon/gl-statequery-db.h>

namespace dglState {


    template<stateQueryDB::GetterID> class Getter;

    template<> class Getter<stateQueryDB::GetterID::GETTER_GetIntegerv> {
    public:
        inline void operator()(gl_t param, std::vector<AnyValue>& ret) {
            GLint value0;
            DIRECT_CALL_CHK(glGetIntegerv)(static_cast<GLenum>(param), &value0);
            if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {
                ret.resize(1); ret[0] = value0;
            } else {
                DGL_ASSERT("GL error in GETTER_GetIntegerv");
            }
        }
    };


    //NOTE: use if (hasCapability(ContextCap::Has64BitGetters)) for glGetInteger64v!


   class QueryableStateItem {
   public:
       QueryableStateItem(stateQueryDB::StateID stateID) : m_StateID(stateID) {
           stateQueryDB::DataBase::GetStateParams(m_StateID, m_GetterID, m_GetterParam);
       }
       
       bool isValid() {
           return m_GetterID == stateQueryDB::GetterID::GETTER_GetIntegerv;
       }

       void queryTo(std::vector<AnyValue>& ret) {
           switch (m_GetterID) {
               case stateQueryDB::GetterID::GETTER_GetIntegerv:
                   Getter<stateQueryDB::GetterID::GETTER_GetIntegerv>()(m_GetterParam, ret);
               break;
               default:
                   throw std::runtime_error("QueryableStateItem: unrecognized getter\n");
           }
       }

   private:
       stateQueryDB::StateID m_StateID;
       stateQueryDB::GetterID m_GetterID;       
       gl_t m_GetterParam;
   };


   void GLStateQuery::Query(std::vector<dglnet::resource::utils::StateItem>& retQueriedItems) {

       retQueriedItems.resize((size_t)stateQueryDB::StateID::STATE_ID_LAST);

       int i = 0; 
       for (int stateID = 0; stateID < (int)stateQueryDB::StateID::STATE_ID_LAST; ++stateID) {

           QueryableStateItem item((stateQueryDB::StateID)stateID);

           if (item.isValid()) {
               item.queryTo(retQueriedItems[i].m_Values);
           }
           retQueriedItems[i].m_StateId = stateID;

           ++i;
       }

   };

};
