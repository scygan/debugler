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

#include "dglproject_runapp.h"

#include <DGLCommon/os.h>
#include <DGLCommon/def.h>

#include <QFileInfo>
#include <QDir>

#include <QFileDialog>

#ifndef _WIN32
#include <wordexp.h>
#else
#include <wtypes.h>
#include <shellapi.h>
#endif

#include <stdexcept>
#include <sstream>


DGLRunAppProject::DGLRunAppProject(const std::string& executable,
                                   const std::string& path, const std::wstring& args, int skipProcesses,
                                   bool eglMode)
        : m_process(nullptr),
          m_executable(executable),
          m_path(path),
          m_args(args),
          m_skipProcesses(skipProcesses),
          m_EglMode(eglMode) {}

DGLRunAppProject::DGLRunAppProject() : m_process(nullptr), m_skipProcesses(0) {}

DGLRunAppProject::~DGLRunAppProject() { stopDebugging(); }

const std::string& DGLRunAppProject::getExecutable() const {
    return m_executable;
}

const std::string& DGLRunAppProject::getPath() const { return m_path; }

const std::wstring& DGLRunAppProject::getCommandLineArgs() const {
    return m_args;
}

bool DGLRunAppProject::isEglMode() const {
    return m_EglMode;
}

int DGLRunAppProject::getSkipProcessesCount() const {
    return m_skipProcesses;
}

void DGLRunAppProject::startDebugging() {
    // randomize connection port
    int port = rand() % (0xffff - 1024) + 1024;

    m_process = new DGLDebugeeQTProcess(port, m_EglMode);

    m_process->setParent(this);

    CONNASSERT(m_process, SIGNAL(processReady()), this,
               SLOT(processReadyHandler()));
    CONNASSERT(m_process, SIGNAL(processError(std::string)), this,
               SLOT(processErrorHandler(std::string)));
    CONNASSERT(m_process, SIGNAL(processFinished(int)), this,
               SLOT(processExitHandler(int)));
    CONNASSERT(m_process, SIGNAL(processCrashed()), this,
               SLOT(processCrashHandler()));

    m_process->run(getExecutable(), getPath(), getCommandLineArgVector(), getSkipProcessesCount());
}

void DGLRunAppProject::stopDebugging() {
    if (m_process) {
        m_process->exit(false);
        m_process->requestDelete();
        m_process = NULL;
    }
}

bool DGLRunAppProject::shouldTerminateOnStop() {
    return true;
}

void DGLRunAppProject::processReadyHandler() {

    std::ostringstream portStr;
    portStr << m_process->getPort();

    emit debugStarted("127.0.0.1", portStr.str());
}

void DGLRunAppProject::processCrashHandler() {
    emit debugError(tr("Process crashed"), tr("Loader process has crashed"));
}

void DGLRunAppProject::processErrorHandler(std::string err) {
    emit debugError(tr("Fatal Error"), QString::fromStdString(err));
}

void DGLRunAppProject::processExitHandler(int code) {
    emit debugExit(tr("Process has exited with code ") + QString::number(code) +
                   ".");
}

std::vector<std::string> DGLRunAppProject::getCommandLineArgVector() {
    std::vector<std::string> ret;

// some magic here. We have arguments from user, but we must pre-parse them and
// split into and array.
// due to laziness some weird system-specific functions are used here.
#ifdef _WIN32
    int numArgs;
    if (m_args.length()) {
        LPWSTR* strings =
                CommandLineToArgvW(m_args.c_str(), &numArgs);
        if (!strings) {
            if (int osError = Os::getLastosError()) {
                throw std::runtime_error("Program arguments: " +
                                         Os::translateOsError(osError));
            } else {
                throw std::runtime_error("Program arguments: unknown error");
            }
        }

        if (numArgs > 0) {
            ret.resize(static_cast<size_t>(numArgs));
            for (size_t i = 0; i < ret.size(); i++) {
                ret[i] = QString::fromWCharArray(strings[i]).toStdString();
            }
        }
        LocalFree(strings);
    }
#else
    wordexp_t wordExp;


    std::string argsUtf8 = "";

    if (m_args.size()) {
        std::vector<char> charBuffer(MB_CUR_MAX * m_args.size(), 0);
        wcstombs(&charBuffer[0], &m_args[0], charBuffer.size());
        argsUtf8 = &charBuffer[0];
    }

    int status = wordexp(argsUtf8.c_str(), &wordExp, 0);
    switch (status) {
        case WRDE_BADCHAR:
            throw std::runtime_error(
                    "Program arguments: Illegal occurrence of newline or one "
                    "of |, "
                    "&, ;, <, >, (, ), {, }.");
        case WRDE_BADVAL:
            DGL_ASSERT(!"no  WRDE_UNDEF was set, but got WRDE_BADVAL");
            throw std::runtime_error(
                    "Program arguments: An undefined shell variable was "
                    "referenced");
        case WRDE_CMDSUB:
            DGL_ASSERT(!" WRDE_NOCMD flag not set, but got WRDE_CMDSUB");
            throw std::runtime_error(
                    "Program arguments: Command substitution occurred");
        case WRDE_NOSPACE:
            throw std::runtime_error("Program arguments: Out of memory.");
        case WRDE_SYNTAX:
            throw std::runtime_error("Program arguments: syntax error");
        case 0:
            break;
        default:
            throw std::runtime_error("Program arguments: unknown error");
    }

    ret.resize(wordExp.we_wordc);
    for (size_t i = 0; i < ret.size(); i++) {
        ret[i] = wordExp.we_wordv[i];
    }

    wordfree(&wordExp);

#endif
    return ret;
}

DGLRunAppProjectFactory::DGLRunAppProjectFactory() {
    m_ui.setupUi(&m_gui);

#ifndef _WIN32
    m_ui.radioButton_ModeWGLGLX->setText("GLX");
#endif

    CONNASSERT(m_ui.lineEdit_Executable, SIGNAL(editingFinished()), this,
               SLOT(updatePath()));
    CONNASSERT(m_ui.toolButton_Exec, SIGNAL(clicked()), this,
               SLOT(browseExecutable()));
    CONNASSERT(m_ui.toolButton_Dir, SIGNAL(clicked()), this,
               SLOT(browseDirectory()));
    m_ui.lineEdit_Executable->setFocus();
}

void DGLRunAppProjectFactory::updatePath() {
    QFileInfo info(m_ui.lineEdit_Executable->text());
    m_ui.lineEdit_Path->setText(QDir::toNativeSeparators(info.dir().path()));
    ;
}

void DGLRunAppProjectFactory::browseExecutable() {
    QFileInfo info(m_ui.lineEdit_Executable->text());

    QString res = QFileDialog::getOpenFileName(
            &m_gui, tr("Choose a executable to run"), info.absoluteFilePath(),
            tr("Executables " DGL_PLATFORM_EXECSUFFIX));

    if (!res.isNull()) {
        m_ui.lineEdit_Executable->setText(QDir::toNativeSeparators(res));
    }
    updatePath();
}

void DGLRunAppProjectFactory::browseDirectory() {
    QString res = QFileDialog::getExistingDirectory(&m_gui, "Choose directory",
                                                    m_ui.lineEdit_Path->text());
    if (!res.isNull()) {
        m_ui.lineEdit_Path->setText(res);
    }
}

std::shared_ptr<DGLProject> DGLRunAppProjectFactory::createProject() {
    QString message;
    if (!valid(message)) {
        throw std::runtime_error(message.toStdString());
    } else {
        return std::make_shared<DGLRunAppProject>(
                m_ui.lineEdit_Executable->text().toStdString(),
                m_ui.lineEdit_Path->text().toStdString(),
                m_ui.lineEdit_CommandLineArgs->text().toStdWString(),
                m_ui.spinBox_processSkip->value(),
                m_ui.radioButton_ModeEGL->isChecked() &&
                        !m_ui.radioButton_ModeWGLGLX->isChecked());
    }
}

bool DGLRunAppProjectFactory::valid(QString& message) {
    if (!m_ui.lineEdit_Executable->text().length()) {
        message = tr("Executable not set.");
        return false;
    }
    return true;
}

bool DGLRunAppProjectFactory::loadPropertiesFromProject(
        const DGLProject* project) {
    const DGLRunAppProject* runProject =
            dynamic_cast<const DGLRunAppProject*>(project);
    if (!runProject) {
        return false;
    } else {
        m_ui.lineEdit_Executable->setText(
                QString::fromStdString(runProject->getExecutable()));
        m_ui.lineEdit_CommandLineArgs->setText(
                QString::fromStdWString(runProject->getCommandLineArgs()));
        m_ui.lineEdit_Path->setText(
                QString::fromStdString(runProject->getPath()));

        if (runProject->isEglMode()) {
            m_ui.radioButton_ModeWGLGLX->setChecked(false);
            m_ui.radioButton_ModeEGL->setChecked(true);
        } else {
            m_ui.radioButton_ModeWGLGLX->setChecked(true);
            m_ui.radioButton_ModeEGL->setChecked(false);
        }

        m_ui.spinBox_processSkip->setValue(runProject->getSkipProcessesCount());
    }
    return true;
}

QString DGLRunAppProjectFactory::getName() {
    return tr("Run local Application");
}

QWidget* DGLRunAppProjectFactory::getGUI() { return &m_gui; }
