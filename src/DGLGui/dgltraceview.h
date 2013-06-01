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


#ifndef DGLTRACEVIEW_H
#define DGLTRACEVIEW_H

#include <QDockWidget>
#include <QListWidget>
#pragma warning(push)
#pragma warning(disable:4512) // assignment operator could not be generated
#include <QtGui>
#pragma warning(pop)


#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"

class DGLTraceViewList: public QListWidget {
    Q_OBJECT
public:
    DGLTraceViewList(QWidget*);
    uint getVisibleRowCount(); 
    int getFirstVisibleElementIdx(); 

private:
    virtual void resizeEvent (QResizeEvent* e);
signals:
    void resized();
};


class DGLTraceView : public QDockWidget {
    Q_OBJECT

public:
    DGLTraceView(QWidget* parrent, DglController* controller);

signals: 
    void queryCallTrace(uint startOffset, uint endOffset);

public slots:
    void setEnabled(bool);
    void setRunning(bool);
    void breaked(CalledEntryPoint, uint);
    void gotCallTraceChunkChunk(uint, const std::vector<CalledEntryPoint>&);


    void mayNeedNewElements();

private: 
    DGLTraceViewList m_traceList;
    bool m_Enabled;
    int m_QueryUpperBound;
};

#endif // DGLTRACEVIEW_H
