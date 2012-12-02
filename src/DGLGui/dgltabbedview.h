#ifndef DGLTABBEDVIEW_H
#define DGLTABBEDVIEW_H

#include <QDockWidget>
#include <QTabWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"

class DGLTabbedViewItem: public QWidget {
public:
    DGLTabbedViewItem(uint objId, QWidget* parrent);

    uint getObjId();

private: 
    uint m_ObjId;
};

class DGLTabbedView : public QDockWidget {
    Q_OBJECT

public:
    DGLTabbedView(QWidget* parrent, DglController* controller);
    virtual ~DGLTabbedView() {}

    public slots:
        void clear();
        void enable();
        void disable();
      
    private slots:
        void closeTab(int);

protected:
    void ensureTabDisplayed(uint id, uint target = 0);
    DGLTabbedViewItem* getTab(uint id);
    void setupNames(char* title, char* objName);

    DGLResourceManager* m_ResourceManager;

    DglController* m_Controller;
private: 
    virtual DGLTabbedViewItem* createTab(uint id) = 0;
    virtual QString getTabName(uint id, uint target) = 0;
    QTabWidget m_TabWidget;    
};

#endif // DGLTABBEDVIEW_H
