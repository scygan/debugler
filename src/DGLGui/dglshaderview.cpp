#include "dglshaderview.h"
#include "dglgui.h"

#include "ui_dglshaderview.h"

class DGLShaderViewItem: public DGLTabbedViewItem {
public:
    DGLShaderViewItem(uint name, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
        m_Ui.setupUi(this);
    }

    void update(const dglnet::ShaderMessage& msg) {
        std::string errorMsg;
        if (!msg.isOk(errorMsg)) {
//TODO
        } else {
            m_Ui.textEditLinker->setText(QString::fromStdString(msg.m_CompileStatus.first));
            m_Ui.textEdit->clear();
            for (size_t i = 0; i < msg.m_Sources.size(); i++) {
                m_Ui.textEdit->append(QString::fromStdString(msg.m_Sources[i]));
            }
            if (!msg.m_CompileStatus.second) {
                m_Ui.labelLinkStatus->setText(tr("Compile status: failed"));
            } else {
                m_Ui.labelLinkStatus->setText(tr("Compile status: success"));
            }
        }
    }
    
    virtual void requestUpdate(DglController* controller) {
        controller->requestShader(getObjId(), false);
    }
    Ui::DGLShaderViewItem m_Ui;
};

DGLShaderView::DGLShaderView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Shaders", "DGLShaderView");

    //inbound
    CONNASSERT(connect(controller, SIGNAL(focusShader(uint)), this, SLOT(showShader(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotShader(uint, const dglnet::ShaderMessage&)), this, SLOT(gotShader(uint, const dglnet::ShaderMessage&))));
}

void DGLShaderView::showShader(uint name) {
    update(name);
}

void DGLShaderView::gotShader(uint name, const dglnet::ShaderMessage& msg) {
    DGLTabbedViewItem* widget = getTab(name);
    if (widget) {
        dynamic_cast<DGLShaderViewItem*>(widget)->update(msg);
    }
}

DGLTabbedViewItem* DGLShaderView::createTab(uint id) {
    return new DGLShaderViewItem(id, this);
}

QString DGLShaderView::getTabName(uint id) {
    return QString("Shader ") + QString::number(id);
}