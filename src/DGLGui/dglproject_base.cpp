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

#include "dglproject_base.h"
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/export.hpp>


#include "dglproject_runapp.h"
#include "dglproject_attach.h"
#include "dglproject_android.h"

BOOST_CLASS_EXPORT(DGLRunAppProject)
BOOST_CLASS_EXPORT(DGLAttachProject)
BOOST_CLASS_EXPORT(DGLAndroidProject)
BOOST_CLASS_EXPORT(DGLProject)

void DGLProject::saveToStream(std::ostream& oStream) const {
    
    boost::archive::xml_oarchive archive(oStream);

    archive << BOOST_SERIALIZATION_NVP(this);

}

std::shared_ptr<DGLProject> DGLProject::createFromStream(std::istream& iStream) {

    DGLProject* ret;

    boost::archive::xml_iarchive archive(iStream);

    archive >> BOOST_SERIALIZATION_NVP(ret);

    return std::shared_ptr<DGLProject>(ret);
}

bool DGLProject::operator== (const DGLProject& rhs) {
    std::ostringstream thisProjectStream, rhsProjectStream;
    try {
        saveToStream(thisProjectStream);
        rhs.saveToStream(rhsProjectStream);

        return thisProjectStream.str() == rhsProjectStream.str();
    } catch (...) {
        return false;
    }
}
