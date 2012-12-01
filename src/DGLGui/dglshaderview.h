#ifndef DGLSHADERVIEW_H
#define DGLSHADERVIEW_H

#include "dgltabbedview.h"


class DGLShaderView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLShaderView(QWidget* parrent, DglController* controller);

    public slots:
        void showShader(uint);
        void gotShader(uint, const dglnet::ShaderMessage&);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id);

};

#endif //DGLSHADERVIEW_H