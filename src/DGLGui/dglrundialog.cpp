#include <QFileInfo>
#include <QDir>
#include "dglrundialog.h"
#include "dglgui.h"

#include <QFileDialog>

DGLRunDialog::DGLRunDialog() {
    m_ui.setupUi(this);
    
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