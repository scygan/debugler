#include "dglbufferview.h"
#include "dglgui.h"
#include "QHexEdit/qhexedit.h"

class DGLBufferViewItem: public DGLTabbedViewItem {
public:
    DGLBufferViewItem(uint name, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
        editor = new QHexEdit(this);
        verticalLayout = new QVBoxLayout(this);
        verticalLayout->addWidget(editor);
    }

    void update(const dglnet::BufferMessage& msg) {
        std::string errorMsg;
        if (!msg.isOk(errorMsg)) {
//TODO
        } else {
            QByteArray array(&msg.m_Data[0], msg.m_Data.size());
            editor->setData(array);
        }
    }

private: 
    QHexEdit* editor;
    QVBoxLayout* verticalLayout;
};

DGLBufferView::DGLBufferView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Vertex Buffers", "DGLBufferView");

    //inbound
    CONNASSERT(connect(controller, SIGNAL(showBuffer(uint)), this, SLOT(showBuffer(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotBuffer(uint, const dglnet::BufferMessage&)), this, SLOT(gotBuffer(uint, const dglnet::BufferMessage&))));
}

void DGLBufferView::showBuffer(uint name) {
    update(name);
}

void DGLBufferView::gotBuffer(uint name, const dglnet::BufferMessage& msg) {
    DGLTabbedViewItem* widget = getTab(name);
    if (widget) {
        dynamic_cast<DGLBufferViewItem*>(widget)->update(msg);
    }
}

DGLTabbedViewItem* DGLBufferView::createTab(uint id) {
    return new DGLBufferViewItem(id, this);
}

QString DGLBufferView::getTabName(uint id) {
    return QString("Vertex Buffer ") + QString::number(id);
}