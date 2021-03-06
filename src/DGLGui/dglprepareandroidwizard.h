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

#ifndef DGLPROVISIONANDROIDDIALOG_H
#define DGLPROVISIONANDROIDDIALOG_H

#include "dglqtgui.h"
#include <QWizard>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>

#include "dgladbdevice.h"

class DGLAndroidSelectDevWidget;

namespace dglPrepareAndroidWizard {

class Wizard : public QWizard {
    Q_OBJECT
   public:
    Wizard(QWidget* parent = 0);
    void accept();

    enum {
        Page_Intro,
        Page_DeviceChoice,
        Page_Run,
        Page_Conclusion
    };
};

namespace pages {
class Intro : public QWizardPage {
    Q_OBJECT
   public:
    Intro(QWidget* parent = 0);

   private:
    int nextId() const override;

    QLabel* label;
    QCheckBox* fakeAcceptBox;
    QCheckBox* acceptBox;
};

class DeviceChoice : public QWizardPage {
    Q_OBJECT
   public:
    DeviceChoice(QWidget* parent = 0);
    DGLADBDevice* device() const;
   public
slots:
    void adbFailed(std::string reason);
    void selectDevice(DGLADBDevice*);
    void queryDeviceStatusSuccess(DGLADBDevice* device);
    void reloadStatus();
signals:
   private:
    virtual bool isComplete() const override;

    void setDeviceStatus(DGLADBDevice::InstallStatus, DGLADBDevice::ABI, bool force = false);

    virtual void hideEvent(QHideEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;

    DGLAndroidSelectDevWidget* m_SelectWidget;

    Q_PROPERTY(DGLADBDevice* deviceProp READ device)

    QLabel* m_DeviceStatusLabel;
    QLabel* m_DeviceABILabel;
    QRadioButton* m_RadioButtonClean, *m_RadioButtonUpdate,
            *m_RadioButtonInstall;
    QTimer m_ReloadTimer;

    DGLADBDevice::InstallStatus m_Status;
    DGLADBDevice::ABI m_Abi;
};


class Run : public QWizardPage {
    Q_OBJECT
   public:
    Run(QWidget* parent = 0);
   public
slots:
    void failed(DGLADBDevice*, const std::string& reason);
    void log(DGLADBDevice*, const std::string& log);
    void installerDone(DGLADBDevice*);

   private:
    virtual void initializePage() override;
    virtual bool isComplete() const override;
    virtual int nextId() const override;

    DGLADBDevice* m_Device;
    QListWidget* m_LogWidget;

    /**
     * Set to true, if we can get out of this wizard page
     */
    bool m_Complete;

    /**
     * Set to true on error, so this is the last page we can show.
     */
    bool m_Final;
};

class Conclusion : public QWizardPage {
    Q_OBJECT
   public:
    Conclusion(QWidget* parent = 0);
};
}
}

Q_DECLARE_METATYPE(DGLADBDevice*)

#endif    // DGLPROVISIONANDROIDDIALOG_H
