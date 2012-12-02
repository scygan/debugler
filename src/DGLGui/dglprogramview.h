#ifndef DGLPROGRAMVIEW_H
#define DGLPROGRAMVIEW_H

#include "dgltabbedview.h"
#include <QPlainTextEdit>

class DGLProgramView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLProgramView(QWidget* parrent, DglController* controller);

    public slots:
        void showProgram(uint);
        void gotProgram(uint, const dglnet::ProgramMessage&);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id, uint target);

};

#endif //DGLPROGRAMVIEW_H