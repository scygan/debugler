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


#ifndef DGLSYNTAXHIGHLIGHT_H
#define DGLSYNTAXHIGHLIGHT_H

#include "dglqtgui.h"
#include <QSyntaxHighlighter>
#include <stack>
#include <set>
#include <vector>

class DGLHLData;
class DGLHLContext;

class DGLSyntaxHighlighterGLSL : public QSyntaxHighlighter {
    Q_OBJECT

public:
    DGLSyntaxHighlighterGLSL(QTextDocument *parent = 0);

    class HLState: public std::stack<const DGLHLContext*, std::vector<const DGLHLContext*>> {
    public:
        bool operator<(const HLState& other ) const;
        HLState(const DGLHLData* data); 
        const DGLHLContext* getContext();
    private:
        const DGLHLContext* operator[](size_t i) const;
    };

protected:
    void highlightBlock(const QString &text);

private:
    static boost::shared_ptr<DGLHLData> s_data;

    std::vector<const HLState*> m_hlStateByIdx;
    std::map<HLState, int> m_hlStateMap;
};

#endif //DGLSYNTAXHIGHLIGHT_H
