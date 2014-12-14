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

#ifndef _SHARED_PTR_CONVERTER_H
#define _SHARED_PTR_CONVERTER_H
/*
//http://stackoverflow.com/questions/12314967/cohabitation-of-boostshared-ptr-and-stdshared-ptr
template<typename T>
boost::shared_ptr<T> convert_shared_ptr(std::shared_ptr<T>& ptr)
{
    return boost::shared_ptr<T>(ptr.get(), [ptr](T*) mutable {ptr.reset();});
}

template<typename T>
std::shared_ptr<T> convert_shared_ptr(boost::shared_ptr<T>& ptr)
{
    return std::shared_ptr<T>(ptr.get(), [ptr](T*) mutable {ptr.reset();});
}

template<typename T>
boost::shared_ptr<T> convert_shared_ptr_nref(std::shared_ptr<T> ptr)
{
    return boost::shared_ptr<T>(ptr.get(), [ptr](T*) mutable {ptr.reset();});
}

template<typename T>
std::shared_ptr<T> convert_shared_ptr_nref(boost::shared_ptr<T> ptr)
{
    return std::shared_ptr<T>(ptr.get(), [ptr](T*) mutable {ptr.reset();});
}

*/
//-----
template<typename T>
std::shared_ptr<T> convert_shared_ptr(std::shared_ptr<T>& ptr)
{
    return ptr;
}
template<typename T>
std::shared_ptr<T> convert_shared_ptr_nref(std::shared_ptr<T> ptr)
{
    return ptr;
}


#endif
