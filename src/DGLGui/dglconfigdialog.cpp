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

#include "dglconfigdialog.h"

#include <QFileDialog>

DGLConfigDialog::DGLConfigDialog(const DGLConfiguration& configuration,
                                 std::string adbPath)
        : m_Configuration(configuration) {
    m_Ui.setupUi(this);

    m_Configuration.m_ForceDebugContextES =
        (m_Configuration.m_ForceDebugContextES &&
         m_Configuration.m_ForceDebugContext);

    m_Ui.checkBoxDebugContext->setChecked(m_Configuration.m_ForceDebugContext);
    m_Ui.checkBoxDebugContextES->setChecked(
        m_Configuration.m_ForceDebugContextES);

    m_Ui.lineEdit_Adb->setText(QString::fromStdString(adbPath));
}

DGLConfigDialog::~DGLConfigDialog() {}

const DGLConfiguration* DGLConfigDialog::getConfig() {
    m_Configuration.m_ForceDebugContext =
        m_Ui.checkBoxDebugContext->isChecked();
    m_Configuration.m_ForceDebugContextES =
        m_Ui.checkBoxDebugContextES->isChecked();
    return &m_Configuration;
}

void DGLConfigDialog::toggleDebugFlagRenderingContext(bool enabled) {
    m_Ui.checkBoxDebugContextES->setDisabled(!enabled);
    if (!enabled) {
        m_Ui.checkBoxDebugContextES->setChecked(enabled);
    }
}

void DGLConfigDialog::adbBrowseDialog() {
    QFileInfo info(m_Ui.lineEdit_Adb->text());
    QString res = QFileDialog::getOpenFileName(
        this, tr("Choose adb executable"), info.absoluteFilePath(),
        tr("Executables (*.exe)"));
    if (!res.isNull()) {
        m_Ui.lineEdit_Adb->setText(QDir::toNativeSeparators(res));
    }
}

QString DGLConfigDialog::getAdbPath() { return m_Ui.lineEdit_Adb->text(); }
