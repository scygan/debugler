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

#ifndef DGLRUNAPP_PROJECT_H
#define DGLRUNAPP_PROJECT_H

#include <QWidget>

#include "ui_dglprojproperties_runapp.h"

#include "dglproject_base.h"
#include "dglprocess.h"

#include <boost/serialization/nvp.hpp>


class DGLRunAppProject: public DGLProject {
    Q_OBJECT
public:
    DGLRunAppProject(const std::string& executable,
        const std::string& path, const std::wstring& args, bool eglMode);
    DGLRunAppProject();
    ~DGLRunAppProject();

    const std::string& getExecutable() const;
    const std::string& getPath() const;
    const std::wstring& getCommandLineArgs() const;
    bool isEglMode();

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /* version */) {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(DGLProject);
        ar & BOOST_SERIALIZATION_NVP(m_executable);
        ar & BOOST_SERIALIZATION_NVP(m_path);
        ar & BOOST_SERIALIZATION_NVP(m_args);
        ar & BOOST_SERIALIZATION_NVP(m_EglMode);
    }

private slots:
     /**
     * Debugee process crash handler
     */
    void processCrashHandler();

    /**
     * Debugee process exit handler
     */
    void processExitHandler(int);

    /**
     * Debugee process starup error event handler
     */
    void processErrorHandler(std::string);

    /**
     * Debugee process ready event handler
     */
    void processReadyHandler();

private:
    virtual void startDebugging() override;

    virtual void stopDebugging() override;

    virtual bool shouldTerminateOnStop() override;

    std::vector<std::string> getCommandLineArgVector();


    /**
     * Current debugee process
     */
    DGLDebugeeQTProcess *m_process;

    std::string m_executable, m_path;
    std::wstring m_args;

    bool m_EglMode;
};

class DGLRunAppProjectFactory: public DGLProjectFactory {
    Q_OBJECT
public:
    DGLRunAppProjectFactory();


    private
slots:
    void updatePath();
    void browseExecutable();
    void browseDirectory();

private:

    virtual std::shared_ptr<DGLProject> createProject() override;

    virtual bool valid(QString&);
    virtual bool loadPropertiesFromProject(const DGLProject*);
    virtual QString getName() override;
    virtual QWidget* getGUI() override;
    Ui::DGLProjPropertiesRunAppClass m_ui;
    QWidget m_gui;
};

#endif
