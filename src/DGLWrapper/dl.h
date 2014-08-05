#ifndef _DL_H
#define _DL_H
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

#include <map> 
#include <DGLCommon/def.h>

#include <string>
#include <memory>

/** 
 * Class wrapping dynamic loader calls
 *
 * Used for dynamically loading shared objects into process
 */
class DynamicLibrary {
public:
    virtual ~DynamicLibrary() {}
    /** 
     * Get address of function from library
     */
    virtual dgl_func_ptr getFunction(const char* symbolName) const = 0;
};

/** 
 * Aggregator of all open DynamicLibraries
 * 
 * Object keeping track of all open libraries. Part of early global state.
 */
class DynamicLoader {
public:
    /** 
     * Getter for library by name. 
     * 
     * @return ptr to existing DynamicLibrary object,
     * if library is opened. New DynamicLibrary object otherwise.
     */
    DynamicLibrary* getLibrary(const char* name);
private:

    /** 
     * Map of all opened dynamic libraries
     */
    std::map<std::string, std::shared_ptr<DynamicLibrary> > m_OpenLibraries;
};
#endif