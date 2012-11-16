#include "dglbufferview.h"
#include "dglgui.h"
#include "QHexEdit/qhexedit.h"

class DGLBufferViewWidget: public QWidget {
public:
    DGLBufferViewWidget(uint name, QWidget* parrent):QWidget(parrent),m_BufferName(name) {
        editor = new QHexEdit(this);
        verticalLayout = new QVBoxLayout(this);
        verticalLayout->addWidget(editor);
    }
    uint getBufferName() {return m_BufferName; }

    void update(dglnet::BufferMessage msg) {
        std::string errorMsg;
        if (!msg.isOk(errorMsg)) {
//...
        } else {
            QByteArray array(&msg.m_Data[0], msg.m_Data.size());
            editor->setData(array);
        }
    }

private: 
    uint m_BufferName;
    QHexEdit* editor;
    QVBoxLayout* verticalLayout;
};

DGLBufferView::DGLBufferView(QWidget* parrent, DglController* controller):QDockWidget(tr("Buffers"), parrent),m_Controller(controller),m_TabWidget(this) {
    setObjectName("DGLBufferView");

    m_TabWidget.setTabsClosable(true);

    disable();
    setWidget(&m_TabWidget);

    //inbound
    CONNASSERT(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    CONNASSERT(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    CONNASSERT(connect(controller, SIGNAL(showBuffer(uint)), this, SLOT(showBuffer(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotBuffer(uint, dglnet::BufferMessage)), this, SLOT(gotBuffer(uint, dglnet::BufferMessage))));

    //internal
    CONNASSERT(connect(&m_TabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int))));
}

void DGLBufferView::enable() {
    m_Enabled = true;
}

void DGLBufferView::disable() {
    m_Enabled = false;
    while (m_TabWidget.count()) {
        delete m_TabWidget.widget(0);
    }
}

void DGLBufferView::showBuffer(uint name) {
    bool found = false; 
    for (int i = 0; i < m_TabWidget.count(); i++) {
        DGLBufferViewWidget* widget = dynamic_cast<DGLBufferViewWidget*>(m_TabWidget.widget(i));
        if (widget && widget->getBufferName() == name) {
            found = true;
            m_TabWidget.setCurrentIndex(m_TabWidget.indexOf(widget));
        }
    }
    if (!found) {
        m_TabWidget.addTab(new DGLBufferViewWidget(name, this), QString("Buffer ") + QString::number(name));
        m_TabWidget.setCurrentIndex(m_TabWidget.count() - 1);

    }
    m_Controller->debugQueryBuffer(name);
}

void DGLBufferView::gotBuffer(uint name, dglnet::BufferMessage msg) {
    int i; 
    DGLBufferViewWidget* widget;
    for (i = 0; i < m_TabWidget.count(); i++) {
        widget = dynamic_cast<DGLBufferViewWidget*>(m_TabWidget.widget(i));
        if (widget && widget->getBufferName() == name) {
            break;
        }
    }
    if (i != m_TabWidget.count()) {
        widget->update(msg);
    }
}

void DGLBufferView::closeTab(int idx) {
    delete m_TabWidget.widget(idx);
}
