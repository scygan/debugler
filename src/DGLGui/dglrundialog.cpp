#include <QFileInfo>
#include <QDir>
#include "dglrundialog.h"
#include "dglgui.h"

DGLRunDialog::DGLRunDialog() {
    m_ui.setupUi(this);
    CONNASSERT(connect(m_ui.lineEdit_Executable, SIGNAL(editingFinished()),this,SLOT(updatePath())));
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

void DGLRunDialog::updatePath() {
    try {
        QFileInfo info(m_ui.lineEdit_Executable->text());
        m_ui.lineEdit_Path->setText(QDir::toNativeSeparators(info.dir().path()));;
    } catch (...) {}
}