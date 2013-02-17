#ifndef DGLTABBEDVIEW_H
#define DGLTABBEDVIEW_H

#include <QDockWidget>
#include <QTabWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"

class DGLTabbedViewItem: public QWidget {
public:
    DGLTabbedViewItem(ContextObjectName, QWidget* parrent);

    const ContextObjectName& getObjName();

private: 
    ContextObjectName m_ObjectName;
};

class DGLTabbedView : public QDockWidget {
    Q_OBJECT

public:
    DGLTabbedView(QWidget* parrent, DglController* controller);
    virtual ~DGLTabbedView() {}

    public slots:
        void setConnected(bool);
      
    private slots:
        void closeTab(int);

protected:
    void ensureTabDisplayed(uint ctxid, uint id, uint target = 0);
    DGLTabbedViewItem* getTab(const ContextObjectName& id);
    void setupNames(const char* title, const char* objName);

    DGLResourceManager* m_ResourceManager;

    DglController* m_Controller;
private: 
    virtual DGLTabbedViewItem* createTab(const ContextObjectName& id) = 0;
    virtual QString getTabName(uint id, uint target) = 0;
    QTabWidget m_TabWidget;    
};

#endif // DGLTABBEDVIEW_H
