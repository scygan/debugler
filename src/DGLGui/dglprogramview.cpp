#include "dglprogramview.h"
#include "dglgui.h"

#include "ui_dglprogramview.h"
#include "dglshaderviewitem.h"


class DGLProgramViewItem: public DGLTabbedViewItem {
public:
    DGLProgramViewItem(uint name, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
        m_Ui.setupUi(this);
    }

    void update(const dglnet::ProgramMessage& msg) {
        std::string errorMsg;
        if (!msg.isOk(errorMsg)) {
//TODO
        } else {
            m_Ui.textEditLinker->setText(QString::fromStdString(msg.mLinkStatus.first));

            for (size_t i = 0; i < msg.m_AttachedShaders.size(); i++) {
                DGLShaderViewItem* newTab = new DGLShaderViewItem(msg.m_AttachedShaders[i].first, this);
                m_Ui.tabWidget->addTab(newTab, QSting(GetShaderStageName(msg.m_AttachedShaders[i].second)
                    + QString(" Shader ") + QString::number(msg.m_AttachedShaders[i].first)));
                //CONNASSERT(connect(controller, SIGNAL(gotShader(uint, const dglnet::ShaderMessage&)), this, SLOT(gotShader(uint, const dglnet::ShaderMessage&))));
                //controller->request...
            }
            }
            
            if (!msg.mLinkStatus.second) {
                m_Ui.labelLinkStatus->setText(tr("Link status: failed"));
            } else {
                m_Ui.labelLinkStatus->setText(tr("Link status: success"));
            }
        }
    }
    
    virtual void requestUpdate(DglController* controller) {
        controller->requestProgram(getObjId(), false);
    }
    Ui::DGLProgramViewItem m_Ui;
};

DGLProgramView::DGLProgramView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Shader Programs", "DGLProgramView");

    //inbound
    CONNASSERT(connect(controller, SIGNAL(focusProgram(uint)), this, SLOT(showProgram(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotProgram(uint, const dglnet::ProgramMessage&)), this, SLOT(gotProgram(uint, const dglnet::ProgramMessage&))));
}

void DGLProgramView::showProgram(uint name) {
    update(name);
}

void DGLProgramView::gotProgram(uint name, const dglnet::ProgramMessage& msg) {
    DGLTabbedViewItem* widget = getTab(name);
    if (widget) {
        dynamic_cast<DGLProgramViewItem*>(widget)->update(msg);
    }
}

DGLTabbedViewItem* DGLProgramView::createTab(uint id) {
    return new DGLProgramViewItem(id, this);
}

QString DGLProgramView::getTabName(uint id, uint target) {
    return QString("Program Shader ") + QString::number(id);
}