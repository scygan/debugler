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

#include "dglproject_android.h"

#include <QMessageBox>

DGLAndroidProject::DGLAndroidProject(const std::string& deviceSerial, const std::string& processName, const std::string& pid)
        : m_deviceSerial(deviceSerial), m_processName(processName), m_pid(pid), m_Device(nullptr), m_Deleting(false) {}

DGLAndroidProject::~DGLAndroidProject() {
    if (m_Device) {
        m_Device->deleteLater();
    }
}

const std::string& DGLAndroidProject::getDeviceSerial() const {
    return m_deviceSerial;
}

const std::string& DGLAndroidProject::getPid() const {
    return m_pid;
}

const std::string& DGLAndroidProject::getProcessName() const {
    return m_processName;
}

void DGLAndroidProject::startDebugging() {

    m_Deleting = false;

    m_ForwardedPort = rand() % (0xffff - 1024) + 1024;

    m_Device = new DGLADBDevice(m_deviceSerial);

    CONNASSERT(m_Device, SIGNAL(failed(DGLADBDevice*, std::string)), this,
        SLOT(deviceFailed(DGLADBDevice*, std::string)));

    CONNASSERT(
        m_Device,
        SIGNAL(gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>)),
        this,
        SLOT(gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>)));

    CONNASSERT(m_Device, SIGNAL(portForwardSuccess(DGLADBDevice*)), this,
        SLOT(portForwardSuccess(DGLADBDevice*)));

    CONNASSERT(m_Device, SIGNAL(setProcessBreakPointSuccess(DGLADBDevice*)), this,
        SLOT(setProcessBreakPointSuccess(DGLADBDevice*)));

    CONNASSERT(m_Device, SIGNAL(unsetProcessBreakPointSuccess(DGLADBDevice*)), this,
        SLOT(unsetProcessBreakPointSuccess(DGLADBDevice*)));

    if (!m_pid.length()) {
        //process is not running. inject global breakpoint to break it when in starts
        m_Device->setProcessBreakpoint(m_processName);
    } else {
        //just list running processes and pick one.
        m_Device->reloadProcesses();
    }

}

void DGLAndroidProject::stopDebugging() {
    if (m_Device) {
        if (!m_pid.length()) {
            m_Deleting = true;
            m_Device->unsetProcessBreakpoint();
        } else {
            m_Device->deleteLater();
            m_Device = nullptr;
        }
    }
}

bool DGLAndroidProject::shouldTerminateOnStop() {
    return false;
}

void DGLAndroidProject::gotProcesses(DGLADBDevice* device, std::vector<DGLAdbDeviceProcess> processes) {

    if (device != m_Device) return;

    if (m_pid.length()) {

        for (size_t i = 0; i < processes.size(); i++) {
            if (processes[i].getName() == m_processName && processes[i].getPid() == m_pid) {
                m_Device->portForward(processes[i].getPortName(), m_ForwardedPort);
                return;
            }
        }
        m_Device->deleteLater();
        m_Device = NULL;
        emit debugError(tr("Cannot find process"), tr("Cannot find selected process"));

    } else {
        for (size_t i = 0; i < processes.size(); i++) {
            if (processes[i].getName() == m_processName) {

                //found matching process. forward connection to it:
                m_Device->portForward(processes[i].getPortName(), m_ForwardedPort);

                return;
            }
        }
        //todo: wait here. prevent looping.
        m_Device->reloadProcesses();
    }
}


void DGLAndroidProject::setProcessBreakPointSuccess(DGLADBDevice* device) {
    if (device == m_Device) {
        //breakpoint inserted. now look for the process:
        m_Device->reloadProcesses();
    }
}

void DGLAndroidProject::unsetProcessBreakPointSuccess(DGLADBDevice* device) {
    if (device == m_Device) {

        if (!m_Deleting) {
            //breakpoint removed. start debug.
            std::ostringstream portStr; 
            portStr << m_ForwardedPort;
            emit debugStarted("127.0.0.1", portStr.str());
        } else {
            m_Device->deleteLater();
            m_Device = nullptr;
        }
    }
}


void DGLAndroidProject::portForwardSuccess(DGLADBDevice* device) {
   if (device == m_Device) {
      m_Device->unsetProcessBreakpoint();
   }
}

void DGLAndroidProject::deviceFailed(DGLADBDevice* device, const std::string& message) {
    if (m_Deleting) {
        m_Device->deleteLater();
        m_Device = nullptr;
    } else {
        if (device == m_Device) {
            emit debugError(tr("Device Error"), QString::fromStdString(m_Device->getSerial()) + ": " +
                QString::fromStdString(message));
        }   
    }
}

DGLAndroidProjectFactory::DGLAndroidProjectFactory() {
    m_ui.setupUi(&m_gui);

    CONNASSERT(m_ui.selectDevWidget, SIGNAL(selectDevice(DGLADBDevice*)), this,
        SLOT(selectDevice(DGLADBDevice*)));
    CONNASSERT(m_ui.selectDevWidget, SIGNAL(updateWidget()), this, SLOT(updateDialog()));
    CONNASSERT(m_ui.selectDevWidget, SIGNAL(adbFailed(std::string)), this,
        SLOT(adbFailed(std::string)));

    CONNASSERT(m_ui.radioButtonAttach, SIGNAL(toggled(bool)), this,
        SLOT(radioStartupChanged(bool)));
    CONNASSERT(m_ui.radioButtonNew, SIGNAL(toggled(bool)), this,
        SLOT(radioStartupChanged(bool)));

    m_ui.checkBoxManualStart->setChecked(true);
    m_ui.checkBoxManualStart->setDisabled(true);

    radioStartupChanged(true);
}


std::shared_ptr<DGLProject> DGLAndroidProjectFactory::createProject() {
    QString message;
    if (!valid(message)) {
        throw std::runtime_error(message.toStdString());
    } else {
        if (m_Attach) {
            int idx = m_ui.comboBoxProcess->currentIndex();
            return std::make_shared<DGLAndroidProject>(m_ui.selectDevWidget->getCurrentDevice()->getSerial(), 
                m_CurrentProcesses[idx].getName(), m_CurrentProcesses[idx].getPid());
        } else {
            return std::make_shared<DGLAndroidProject>(m_ui.selectDevWidget->getCurrentDevice()->getSerial(), 
                m_ui.comboBoxPackage->lineEdit()->text().toStdString());
        }
    }
}

bool DGLAndroidProjectFactory::valid(QString& message) {
    if (!m_ui.selectDevWidget->getCurrentDevice()) {
        message =
                tr("No device selected. Please select desired device from the list. Use \"adb connect...\" to connect to networked adb devices.");
        return false;
    }

    if (m_Attach) {
        int idx = m_ui.comboBoxProcess->currentIndex();
        if (m_CurrentProcesses.size() == 0 ||
            (idx = m_ui.comboBoxProcess->currentIndex()) < 0) {
                message =
                    tr("No process selected. Please select "
                    "appropriate process running on device.");
                return false;
        }
    } else {
        if (m_ui.comboBoxPackage->lineEdit()->text().length() < 0) {
                message =
                    tr("No process name given. Please fill "
                    "appropriate process name.");
                return false;
        }
    }
    
    return true;
}

bool DGLAndroidProjectFactory::loadPropertiesFromProject(
        const DGLProject* project) {
    const DGLAndroidProject* androidProject =
            dynamic_cast<const DGLAndroidProject*>(project);
    if (!androidProject) {
        return false;
    }

    std::string pid = androidProject->getPid();

    m_Attach = (pid.length() > 0);

    m_ui.selectDevWidget->setPreselectedDevice(androidProject->getDeviceSerial());


    if (m_Attach) {
        m_PreselectedProcess = std::make_shared<DGLAdbDeviceProcess>(pid, androidProject->getProcessName());
    } else {
        m_ui.comboBoxPackage->setEditText(QString::fromStdString(androidProject->getProcessName()));
    }

    return true;
}

QString DGLAndroidProjectFactory::getName() {
    return tr("Android(R) Application");
}

QWidget* DGLAndroidProjectFactory::getGUI() { return &m_gui; }

void DGLAndroidProjectFactory::updateProcesses() {

    m_ui.label_deviceStatus->setText("ok.");

    std::sort(m_CurrentProcesses.begin(), m_CurrentProcesses.end());
    int j = 0;
    for (size_t i = 0; i < m_CurrentProcesses.size(); i++) {
        while (j < m_ui.comboBoxProcess->count() &&
            m_CurrentProcesses[i].getDescriptionStr() <
            m_ui.comboBoxProcess->itemText(j).toStdString()) {
                m_ui.comboBoxProcess->removeItem(j);
        }
        if (m_ui.comboBoxProcess->itemText(j).toStdString() !=
            m_CurrentProcesses[i].getDescriptionStr()) {
                m_ui.comboBoxProcess->insertItem(
                    j, QIcon(),
                    QString::fromStdString(m_CurrentProcesses[i].getDescriptionStr()));
        }
        j++;
    }
    while (m_ui.comboBoxProcess->count() > j) {
        m_ui.comboBoxProcess->removeItem(j);
    }


    if (m_PreselectedProcess) {
        std::vector<DGLAdbDeviceProcess>::iterator i =
            std::find(m_CurrentProcesses.begin(), m_CurrentProcesses.end(), *m_PreselectedProcess);

        if (i != m_CurrentProcesses.end()) {
            m_ui.comboBoxProcess->setCurrentIndex(i - m_CurrentProcesses.begin());
        } else {
            std::shared_ptr<DGLAdbDeviceProcess> process;
            std::swap(m_PreselectedProcess, process);
            QMessageBox::critical(&m_gui, tr("Project properties error"),
                QString::fromStdString("Process " + process->getDescriptionStr() + "not found."));
        }
        m_PreselectedProcess.reset();
    }

}

void DGLAndroidProjectFactory::updatePackages() {

    m_ui.label_deviceStatus->setText("ok.");

    QString tmpValue = m_ui.comboBoxPackage->lineEdit()->text();

    std::sort(m_CurrentPackages.begin(), m_CurrentPackages.end());
    int j = 0;
    for (size_t i = 0; i < m_CurrentPackages.size(); i++) {
        while (j < m_ui.comboBoxPackage->count() &&
            m_CurrentPackages[i] <
            m_ui.comboBoxPackage->itemText(j).toStdString()) {
                m_ui.comboBoxPackage->removeItem(j);
        }
        if (m_ui.comboBoxPackage->itemText(j).toStdString() !=
            m_CurrentPackages[i]) {
                m_ui.comboBoxPackage->insertItem(
                    j, QIcon(),
                    QString::fromStdString(m_CurrentPackages[i]));
        }
        j++;
    }
    while (m_ui.comboBoxPackage->count() > j) {
        m_ui.comboBoxPackage->removeItem(j);
    }

    m_ui.comboBoxPackage->lineEdit()->setText(tmpValue);
}




void DGLAndroidProjectFactory::selectDevice(DGLADBDevice* device) {

    m_CurrentProcesses.clear();
    m_ui.comboBoxProcess->clear();

    m_CurrentPackages.clear();
    QString tmpValue = m_ui.comboBoxPackage->lineEdit()->text();
    m_ui.comboBoxPackage->clear();
    m_ui.comboBoxPackage->lineEdit()->setText(tmpValue);

    if (!device) {
        m_ui.label_deviceStatus->setText("No device selected.");

    } else {
        m_ui.label_deviceStatus->setText(
            "not prepared. Go to Tools-> Prepare Android device.");

        CONNASSERT(
            device,
            SIGNAL(gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>)),
            this,
            SLOT(gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>)));

        CONNASSERT(
            device,
            SIGNAL(gotPackages(DGLADBDevice*, std::vector<std::string>)),
            this,
            SLOT(gotPackages(DGLADBDevice*, std::vector<std::string>)));

        CONNASSERT(device, SIGNAL(failed(DGLADBDevice*, std::string)), this,
            SLOT(deviceFailed(DGLADBDevice*, std::string)));
    }
}

void DGLAndroidProjectFactory::radioStartupChanged(bool) {
    assert(m_ui.radioButtonAttach->isChecked() != m_ui.radioButtonNew->isChecked());

    m_Attach = m_ui.radioButtonAttach->isChecked();

    m_ui.comboBoxProcess->setEnabled(m_Attach);
    m_ui.comboBoxPackage->setEnabled(!m_Attach);
    m_ui.lineEditActivity->setEnabled(!m_Attach && !m_ui.checkBoxManualStart->isChecked());

    if (!m_Attach) {
        m_ui.comboBoxProcess->clear();
    }
}

void DGLAndroidProjectFactory::updateDialog() {
    if (m_ui.selectDevWidget->getCurrentDevice()) {
        if (m_Attach) {
            m_ui.selectDevWidget->getCurrentDevice()->reloadProcesses();
        } else {
            m_ui.selectDevWidget->getCurrentDevice()->reloadPackages();
        }
    }
}

void DGLAndroidProjectFactory::adbFailed(std::string reason) {
    QMessageBox::critical(&m_gui, tr("ADB Error"),
        QString::fromStdString(reason));
}

void DGLAndroidProjectFactory::gotProcesses(
    DGLADBDevice* device, std::vector<DGLAdbDeviceProcess> processes) {

        if (m_ui.selectDevWidget->getCurrentDevice() == device) {
            m_CurrentProcesses = processes;

            updateProcesses();
        }
}

void DGLAndroidProjectFactory::gotPackages(
    DGLADBDevice* device, std::vector<std::string> packages) {

        if (m_ui.selectDevWidget->getCurrentDevice() == device) {
            m_CurrentPackages = packages;

            updatePackages();
        }
}

void DGLAndroidProjectFactory::deviceFailed(DGLADBDevice* device, std::string reason) {
    selectDevice(nullptr);

    QMessageBox::critical(&m_gui, tr("Device Error"),
        QString::fromStdString(device->getSerial()) + ": " +
        QString::fromStdString(reason));
}