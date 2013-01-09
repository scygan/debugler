#ifndef DGLPROGRAMVIEW_H
#define DGLPROGRAMVIEW_H

#include "dgltabbedview.h"
#include "ui_dglprogramview.h"
#include <QPlainTextEdit>

class DGLProgramView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLProgramView(QWidget* parrent, DglController* controller);

    public slots:
        void showProgram(uint ctx, uint name);

private:
        virtual DGLTabbedViewItem* createTab(const ContextObjectName& id);
        virtual QString getTabName(uint id, uint target);

};

class DGLProgramViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLProgramViewItem(ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const DGLResource& res);

private:
    Ui::DGLProgramViewItem m_Ui;
    QLabel* m_Label;
    DGLResourceListener* m_Listener;
    DGLResourceManager* m_ResourceManager;
};

#endif //DGLPROGRAMVIEW_H