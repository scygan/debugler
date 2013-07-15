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


#ifndef DGLSHADERVIEW_H
#define DGLSHADERVIEW_H

#include "dgltabbedview.h"

class DGLShaderView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLShaderView(QWidget* parrent, DglController* controller);

    public slots:
        void showShader(opaque_id_t, gl_t, gl_t);

private:
        virtual DGLTabbedViewItem* createTab(const dglnet::ContextObjectName& id);
        virtual QString getTabName(gl_t id, gl_t target) override;
};

#endif //DGLSHADERVIEW_H