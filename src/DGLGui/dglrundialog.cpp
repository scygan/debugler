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


#include <QFileInfo>
#include <QDir>
#include "dglrundialog.h"
#include "dglgui.h"

#include <QFileDialog>

DGLRunDialog::DGLRunDialog() {
    m_ui.setupUi(this);

#ifndef _WIN32
    m_ui.radioButton_ModeWGLGLX->setText("GLX");
#endif
    
    CONNASSERT(connect(m_ui.lineEdit_Executable, SIGNAL(editingFinished()),this,SLOT(updatePath())));
    CONNASSERT(connect(m_ui.toolButton_Exec, SIGNAL(clicked()),this,SLOT(browseExecutable())));
    CONNASSERT(connect(m_ui.toolButton_Dir, SIGNAL(clicked()),this,SLOT(browseDirectory())));
    m_ui.lineEdit_Executable->setFocus();
}

DGLRunDialog::~DGLRunDialog() {}

std::string DGLRunDialog::getExecutable() {
    return m_ui.lineEdit_Executable->text().toStdString();
}

std::string DGLRunDialog::getCommandLineArgs() {
    return m_ui.lineEdit_CommandLineArgs->text().toStdString();
}

std::string DGLRunDialog::getPath() {
    return m_ui.lineEdit_Path->text().toStdString();
}

bool DGLRunDialog::getModeEGL() {
    return m_ui.radioButton_ModeEGL->isChecked() && !m_ui.radioButton_ModeWGLGLX->isChecked();
}

void DGLRunDialog::updatePath() {
    try {
        QFileInfo info(m_ui.lineEdit_Executable->text());
        m_ui.lineEdit_Path->setText(QDir::toNativeSeparators(info.dir().path()));;
    } catch (...) {}
}

void DGLRunDialog::browseExecutable() {
    QFileInfo info(m_ui.lineEdit_Executable->text());
    QString res = QFileDialog::getOpenFileName( this, tr( "Choose a executable to run" ), info.absoluteFilePath(), tr( "Executables (*.exe)" ) );
    if (!res.isNull()) {
        m_ui.lineEdit_Executable->setText(QDir::toNativeSeparators(res));
    }
    updatePath();
}

void DGLRunDialog::browseDirectory() {
    QString res = QFileDialog::getExistingDirectory( this, "Choose directory", m_ui.lineEdit_Path->text());
    if (!res.isNull()) {
        m_ui.lineEdit_Path->setText(res);
    }
}