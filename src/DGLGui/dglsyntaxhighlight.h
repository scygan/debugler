#ifndef DGLSYNTAXHIGHLIGHT_H
#define DGLSYNTAXHIGHLIGHT_H

#include <QSyntaxHighlighter>
#include <boost/shared_ptr.hpp>
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
