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

#include "dglrundialog.h"

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

#include <DGLCommon/os.h>

DGLRunDialog::DGLRunDialog() {
    m_ui.setupUi(this);

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

DGLRunDialog::~DGLRunDialog() {}

std::string DGLRunDialog::getExecutable() {
    return m_ui.lineEdit_Executable->text().toStdString();
}

std::vector<std::string> DGLRunDialog::getCommandLineArgs() {

    std::vector<std::string> ret;

// some magic here. We have arguments from user, but we must pre-parse them and
// split into and array.
// due to laziness some weird system-specific functions are used here.
#ifdef _WIN32
    int numArgs;
    if (!m_ui.lineEdit_CommandLineArgs->text().isEmpty()) {
        LPWSTR* strings = CommandLineToArgvW(
            m_ui.lineEdit_CommandLineArgs->text().toStdWString().c_str(),
            &numArgs);
        if (!strings) {
            if (int osError = Os::getLastosError()) {
                throw std::runtime_error("Program arguments: " +
                                         Os::translateOsError(osError));
            } else {
                throw std::runtime_error("Program arguments: unknown error");
            }
        }

        if (numArgs > 0) {
            ret.resize(numArgs);
            for (size_t i = 0; i < ret.size(); i++) {
                ret[i] = QString::fromWCharArray(strings[i]).toStdString();
            }
        }
        LocalFree(strings);
    }
#else
    wordexp_t wordExp;

    int status =
        wordexp(m_ui.lineEdit_CommandLineArgs->text().toUtf8(), &wordExp, 0);
    switch (status) {
        case WRDE_BADCHAR:
            throw std::runtime_error(
                "Program arguments: Illegal occurrence of newline or one of |, "
                "&, ;, <, >, (, ), {, }.");
        case WRDE_BADVAL:
            assert(!"no  WRDE_UNDEF was set, but got WRDE_BADVAL");
            throw std::runtime_error(
                "Program arguments: An undefined shell variable was "
                "referenced");
        case WRDE_CMDSUB:
            assert(!" WRDE_NOCMD flag not set, but got WRDE_CMDSUB");
            throw std::runtime_error(
                "Program arguments: Command substitution occurred");
        case WRDE_NOSPACE:
            wordfree(&wordExp);
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

std::string DGLRunDialog::getPath() {
    return m_ui.lineEdit_Path->text().toStdString();
}

bool DGLRunDialog::getModeEGL() {
    return m_ui.radioButton_ModeEGL->isChecked() &&
           !m_ui.radioButton_ModeWGLGLX->isChecked();
}

void DGLRunDialog::updatePath() {
    try {
        QFileInfo info(m_ui.lineEdit_Executable->text());
        m_ui.lineEdit_Path->setText(
            QDir::toNativeSeparators(info.dir().path()));
        ;
    }
    catch (...) {
    }
}

void DGLRunDialog::browseExecutable() {
    QFileInfo info(m_ui.lineEdit_Executable->text());
    QString res = QFileDialog::getOpenFileName(
        this, tr("Choose a executable to run"), info.absoluteFilePath(),
        tr("Executables (*.exe)"));
    if (!res.isNull()) {
        m_ui.lineEdit_Executable->setText(QDir::toNativeSeparators(res));
    }
    updatePath();
}

void DGLRunDialog::browseDirectory() {
    QString res = QFileDialog::getExistingDirectory(this, "Choose directory",
                                                    m_ui.lineEdit_Path->text());
    if (!res.isNull()) {
        m_ui.lineEdit_Path->setText(res);
    }
}
