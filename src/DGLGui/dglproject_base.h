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

#ifndef DGLPROJECT_H
#define DGLPROJECT_H

#include <QWidget>
#include <string>
#include <memory>

class DGLProject : public QObject {
    Q_OBJECT
signals:
    void debugStarted(std::string address, std::string port);
    void debugError(QString error, QString message);
    void debugExit(QString reason);

   public:
    virtual void startDebugging() = 0;
    virtual void stopDebugging() {}

    virtual bool shouldTerminateOnStop() = 0;

    void saveToStream(std::ostream& oStream) const;
    static std::shared_ptr<DGLProject> createFromStream(std::istream& iStream);
    bool operator== (const DGLProject& rhs);

    virtual ~DGLProject() {}

    template<class Archive>
    void serialize(Archive & /* ar */, const unsigned int /* version */) {} 
};

class DGLProjectFactory : public QObject {
    Q_OBJECT
   public:

    virtual std::shared_ptr<DGLProject> createProject() = 0;

    virtual bool valid(QString&) = 0;

    virtual bool loadPropertiesFromProject(const DGLProject*) { return false; }

    virtual QString getName() = 0;
    virtual QWidget* getGUI() = 0;
    virtual ~DGLProjectFactory() {}
};

#endif
