#ifndef DGLSYNTAXHIGHLIGHT_H
#define DGLSYNTAXHIGHLIGHT_H

#include <QSyntaxHighlighter>
#include <boost/shared_ptr.hpp>
#include <stack>

class DGLHLData;
class DGLHLContext;

class DGLSyntaxHighlighterGLSL : public QSyntaxHighlighter {
    Q_OBJECT

public:
    DGLSyntaxHighlighterGLSL(QTextDocument *parent = 0);

    class HLState: public std::stack<const DGLHLContext*> {
    public:
        HLState(const DGLHLData* data); 
        const DGLHLContext* getContext();
    };

protected:
    void highlightBlock(const QString &text);

private:
    static boost::shared_ptr<DGLHLData> s_data;

    std::vector<HLState> m_hlStates;
};

#endif //DGLSYNTAXHIGHLIGHT_H