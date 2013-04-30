#include "dglsyntaxhighlight.h"
#include<boost/make_shared.hpp>

#include <QFile>
#include <QDomDocument>
#include <QRegExp>
#include <QDebug>

#ifndef NDEBUG
#define HL_DEBUG(x) x
#else
#define HL_DEBUG(x)
#endif

#define FATAL { throw std::runtime_error("Cannot parse syntax highlighting file."); }

class DGLHLTextCharFormat {
public:
    DGLHLTextCharFormat(const QColor& color, bool italics = false, bool bold = false, bool strikeout = false) {
        if (!s_defaultFormatsInitialized) {
            initialize();
        }

        m_Format.setFontItalic(italics);
        if (bold) {
            m_Format.setFontWeight(QFont::Bold);
        }
        m_Format.setFontStrikeOut(strikeout);
        m_Format.setForeground(color);
    }
    DGLHLTextCharFormat(const QDomElement& xml) {
        if (!s_defaultFormatsInitialized) {
            initialize();
        }

        if (xml.isNull() || xml.tagName() != "itemData"  || xml.attribute("defStyleNum", "") == "") {
            FATAL;
        }

        bool bold = false, strikeout = false;
        std::string defFormat;

        for (int i = 0; i < xml.attributes().count(); i++) {
            QString name = xml.attributes().item(i).toAttr().nodeName();
            QString val = xml.attributes().item(i).toAttr().nodeValue();
            if (name == "name") {
                continue;
            } else if (name == "bold") {
                bold = (val == "1");
            } else if (name == "strikeout") {
                strikeout = (val == "1");
            } else if (name == "defStyleNum") {
                defFormat = val.toStdString();
            } else {
                FATAL;
            }
        }

        m_Format = s_defaultFormats[defFormat].getFormat();
        if (bold) {
            m_Format.setFontWeight(QFont::Bold);
        }
        m_Format.setFontStrikeOut(strikeout);
    }

    DGLHLTextCharFormat() {}

    const QTextCharFormat& getFormat() const {
        return m_Format;
    }
    static const DGLHLTextCharFormat& getDefault(std::string name) {
        return s_defaultFormats[name];
    }

private:
    void initialize() {
        s_defaultFormatsInitialized = true;
        s_defaultFormats["dsNormal"]   = DGLHLTextCharFormat(Qt::white);
        s_defaultFormats["dsKeyword"]  = DGLHLTextCharFormat(Qt::darkYellow);
        s_defaultFormats["dsDataType"] = DGLHLTextCharFormat(Qt::darkMagenta);
        s_defaultFormats["dsFloat"]    = DGLHLTextCharFormat(Qt::darkBlue);
        s_defaultFormats["dsBaseN"]    = DGLHLTextCharFormat(Qt::green);
        s_defaultFormats["dsDecVal"]   = DGLHLTextCharFormat(Qt::blue);
        s_defaultFormats["dsComment"]  = DGLHLTextCharFormat(Qt::darkCyan);
    }

    QTextCharFormat m_Format;
    static  std::map<std::string, DGLHLTextCharFormat> s_defaultFormats;
    static bool s_defaultFormatsInitialized;
    std::string m_Name;
};
std::map<std::string, DGLHLTextCharFormat> DGLHLTextCharFormat::s_defaultFormats;
bool DGLHLTextCharFormat::s_defaultFormatsInitialized = false;

class DGLHLActionBase {
public:
    virtual void doAction(DGLSyntaxHighlighterGLSL::HLState& state) const = 0;

    static DGLHLActionBase* Create(const DGLHLData* data, const QString name);
};


class DGLHLActionStay: public DGLHLActionBase {
    virtual void doAction(DGLSyntaxHighlighterGLSL::HLState& state) const;
};

class DGLHLActionPop: public DGLHLActionBase {
public:
    DGLHLActionPop::DGLHLActionPop(int counter):m_counter(counter) {}
private:
    virtual void doAction(DGLSyntaxHighlighterGLSL::HLState& state) const;
    int m_counter;
};

class DGLHLActionSetContext: public DGLHLActionBase {
public:
    DGLHLActionSetContext(const DGLHLContext* context):m_context(context) {}
private:
    virtual void doAction(DGLSyntaxHighlighterGLSL::HLState& state) const;
    const DGLHLContext* m_context;
};

class DGLHLRuleBase {
public:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) = 0;

    const DGLHLTextCharFormat* getFormat() {
        assert(m_format);
        return m_format;
    }
    const DGLHLActionBase* getAction() {
        assert(m_action.get());
        return m_action.get();
    }

    static DGLHLRuleBase* Create(const DGLHLData* data, const QDomElement& xml);

    HL_DEBUG(QString m_debugRuleName);

protected:
    DGLHLRuleBase(const DGLHLData* data, QString formatName, QString actionName, HL_DEBUG(QString debugRuleName));

private:
    boost::shared_ptr<DGLHLActionBase> m_action;
    const DGLHLTextCharFormat* m_format;
};

class DGLHLRuleDetectChar: public DGLHLRuleBase {
public: 
    DGLHLRuleDetectChar(const DGLHLData* data, char _char, QString formatName, QString actionName):DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__) + ":" + _char)),
        m_char(_char) {}
private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        matchSize = 1;
        pos = str.indexOf(m_char);
    }
    char m_char;
};

class DGLHLRuleDetect2Chars: public DGLHLRuleBase {
public: 
    DGLHLRuleDetect2Chars(const DGLHLData* data, char char1, char char2, QString formatName, QString actionName):
        DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__) + ":" + char1 + "," + char2)),m_regex(QRegExp::escape(QString(char1) + QString(char2))) {}

    typedef std::vector<std::string> keywordList_t;

private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        pos = (uint)m_regex.indexIn(str);
        matchSize = m_regex.matchedLength();
    }
    QRegExp m_regex;
};

class DGLHLRuleKeyword: public DGLHLRuleBase {
public: 
    DGLHLRuleKeyword(const DGLHLData* data, std::string _string, QString formatName, QString actionName);

    typedef std::vector<QString> keywordList_t;

private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        pos = (uint)m_regex.indexIn(str);
        matchSize = m_regex.matchedLength();
    }
    QRegExp m_regex;
};

class DGLHLRuleInt: public DGLHLRuleBase {
public: 
    DGLHLRuleInt(const DGLHLData* data, QString formatName, QString actionName):DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__))),
        m_regex("\\b[+-]?(0|[1-9][0-9]*)") {}
private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        pos = (uint)m_regex.indexIn(str);
        matchSize = m_regex.matchedLength();
    }
    QRegExp m_regex;
};

class DGLHLRuleHlCHex: public DGLHLRuleBase {
public: 
    DGLHLRuleHlCHex(const DGLHLData* data, QString formatName, QString actionName):DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__))),
        m_regex("\\b[+-]?0(x|X)[0-9]+") {}
private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        pos = (uint)m_regex.indexIn(str);
        matchSize = m_regex.matchedLength();
    }
    QRegExp m_regex;
};

class DGLHLRuleDecimal: public DGLHLRuleBase {
public: 
    DGLHLRuleDecimal(const DGLHLData* data, QString formatName, QString actionName):DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__))) {}
private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        assert(!"not implemented");
    }
};

class DGLHLRuleHlCOct: public DGLHLRuleBase {
public: 
    DGLHLRuleHlCOct(const DGLHLData* data, QString formatName, QString actionName):DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__))),
        m_regex("\\b[+-]?0[0-9]+") {}
private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        pos = (uint)m_regex.indexIn(str);
        matchSize = m_regex.matchedLength();
    }
    QRegExp m_regex;
};

class DGLHLRuleFloat: public DGLHLRuleBase {
public: 
    DGLHLRuleFloat(const DGLHLData* data, QString formatName, QString actionName):DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__))),
      m_regex("\\b[+-]?((((0|[1-9][0-9]*)\\.[0-9]*)|(\\.[0-9]+))([Ee][+-]?[0-9]+)?)|((((0|[1-9][0-9]*)\\.?[0-9]*)|(\\.[0-9]+))([Ee][+-]?[0-9]+))(f|LF)?") {}
private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        pos = (uint)m_regex.indexIn(str);
        matchSize = m_regex.matchedLength();
    }
    QRegExp m_regex;
};

class DGLHLRuleStringDetect: public DGLHLRuleBase {
public: 
    DGLHLRuleStringDetect(const DGLHLData* data, QString _string, QString formatName, QString actionName)
        :DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__) + ":" + _string)),m_regex(QRegExp::escape(_string)) {}

    typedef std::vector<std::string> keywordList_t;

private:
    virtual void tryMatch(const QString& str, uint& pos, int& matchSize) {
        pos = (uint)m_regex.indexIn(str);
        matchSize = m_regex.matchedLength();
    }
    QRegExp m_regex;
};

DGLHLRuleBase* DGLHLRuleBase::Create(const DGLHLData* data, const QDomElement& xml) {
    
    QString actionName = xml.attribute("context");
    QString formatName = xml.attribute("attribute");

    if (xml.tagName() == "Detect2Chars") {
        std::string char1 = xml.attribute("char").toStdString();
        std::string char2 = xml.attribute("char1").toStdString();
        if (char1.size() != 1 || char2.size() != 1) {
            FATAL;
        }
        return new DGLHLRuleDetect2Chars(data, char1[0], char2[0], formatName, actionName);
    } else if (xml.tagName() == "DetectChar") {
        std::string _char = xml.attribute("char").toStdString();
        if (_char.size() != 1) {
            FATAL;
        }
        return new DGLHLRuleDetectChar(data, _char[0], formatName, actionName);
    } else if (xml.tagName() == "keyword") {
        std::string _string = xml.attribute("String").toStdString();
        return new DGLHLRuleKeyword(data, _string, formatName, actionName);
    } else if (xml.tagName() == "Int") {
        return new DGLHLRuleInt(data, formatName, actionName);
    } else if (xml.tagName() == "HlCOct") {
        return new DGLHLRuleHlCOct(data, formatName, actionName);
    } else if (xml.tagName() == "HlCHex") {
        return new DGLHLRuleHlCHex(data, formatName, actionName);
    } else if (xml.tagName() == "Decimal") {
        return new DGLHLRuleDecimal(data, formatName, actionName);
    } else if (xml.tagName() == "Float") {
        return new DGLHLRuleFloat(data, formatName, actionName);
    } else if (xml.tagName() == "StringDetect") {
        QString _string = xml.attribute("String");
        return new DGLHLRuleStringDetect(data, _string, formatName, actionName);
    } else {
        FATAL;
    }
}

class DGLHLContext {
public:
    DGLHLContext(const QDomElement& xml, const DGLHLData* data);
    DGLHLContext() {}

    void link(const DGLHLData* data);

    struct HLResult {
        HLResult():format(NULL), action(NULL), pos(0), size(0) {}
        const DGLHLTextCharFormat* format;
        const DGLHLActionBase* action;
        int pos, size; 
    };

    HLResult doHighlight(const QString& str) const {
        HLResult ret; 
        int best = -1, bestMatchSize = 0, bestPos = str.size();
        for (size_t i = 0; i < m_rules.size(); i++) {
            uint pos; int matchSize;
            m_rules[i]->tryMatch(str, pos, matchSize);
            if (pos < bestPos || (pos == bestPos && matchSize > bestMatchSize)) {
                bestPos = pos;
                bestMatchSize = matchSize;
                best = i;
            }
        }
        if (best >= 0) {
            ret.size = bestMatchSize;
            ret.pos = bestPos;
            ret.format = m_rules[best]->getFormat();
            ret.action = m_rules[best]->getAction();
            HL_DEBUG(qDebug() << "Match(rule = " << m_rules[best]->m_debugRuleName << ", pos=" << ret.pos << ",size=" << ret.size << "): |" << str.mid(ret.pos, ret.size) << "|\n");
        } else {
            ret.pos = str.size();
            ret.size = 0;
            HL_DEBUG(qDebug() << "No Match: |" << str << "|\n");
        }

        return ret;
    }
    const DGLHLTextCharFormat* getDefaultFormat() const { return m_DefaultFormat; }

    const DGLHLActionBase* getLineEndAction() const { return m_LineEndAction.get(); }

    HL_DEBUG(QString m_debugName);

private:
    std::vector<boost::shared_ptr<DGLHLRuleBase> > m_rules;
    const DGLHLTextCharFormat* m_DefaultFormat;
    boost::shared_ptr<DGLHLActionBase> m_LineEndAction;
    QDomElement m_xml;
};


void DGLHLActionStay::doAction(DGLSyntaxHighlighterGLSL::HLState& state) const {
    HL_DEBUG(qDebug() << "Action: stay, current context: " << state.getContext()->m_debugName << "\n");
}

void DGLHLActionPop::doAction(DGLSyntaxHighlighterGLSL::HLState& state) const {
    for (int i = 0; i < m_counter; i++) {
        if (state.size() == 1) {
            assert(!"DGLHLActionPop::doAction - stack underflow");
            break;
        }
        state.pop();
        HL_DEBUG(qDebug() << "Action: pop, current context: " << state.getContext()->m_debugName << "\n");
    }

}

void DGLHLActionSetContext::doAction(DGLSyntaxHighlighterGLSL::HLState& state) const {
    state.push(m_context);
    HL_DEBUG(qDebug() << "Action: set, current context: " << state.getContext()->m_debugName << "\n");
}

class DGLHLData {
public:
    DGLHLData():m_case_sensitive(true) {
        const QString paths[] = {"glsl.xml", ":/res/glsl.xml"};
        boost::shared_ptr<QFile> file;
        for (size_t i = 0; i < sizeof(paths)/sizeof(paths[0]) && !file; i++) {
            file = boost::make_shared<QFile>(paths[i]);
            if (!file->open(QFile::ReadOnly)) {
                file.reset(); //not file->reset() !!
            }
        }

        if (!file) {
            FATAL;
        }

        QDomDocument doc;
        doc.setContent(file.get());
        file->close();

        QDomElement docElem = doc.documentElement();

        QDomElement hightLightingElement = docElem.firstChildElement("highlighting");
        if (hightLightingElement.isNull()) {
            FATAL;
        }

        for (QDomElement element = hightLightingElement.firstChildElement("list"); !element.isNull(); element = element.nextSiblingElement("list")) {
            DGLHLRuleKeyword::keywordList_t list; 
            for (QDomNode listItemNode = element.firstChild();!listItemNode.isNull();listItemNode = listItemNode.nextSibling()) {
                QDomElement listItem = listItemNode.toElement();
                if (!listItem.isNull()) {
                    list.push_back(listItem.text());
                }
            }
            m_lists[element.attribute("name", "").toStdString()] = list;
        }

        for (QDomElement element = hightLightingElement.firstChildElement("itemDatas"); !element.isNull(); element = element.nextSiblingElement("itemDatas")) {
            for (QDomNode itemDataNode = element.firstChild();!itemDataNode.isNull();itemDataNode = itemDataNode.nextSibling()) {
                QDomElement itemDataItem = itemDataNode.toElement();

                if (itemDataItem.isNull()) continue;

                if (itemDataItem.attribute("name", "") == "") {
                    FATAL;
                }
                m_formats[itemDataItem.attribute("name", "").toStdString()] = DGLHLTextCharFormat(itemDataItem);
            }
        }

        for (QDomElement element = hightLightingElement.firstChildElement("contexts"); !element.isNull(); element = element.nextSiblingElement("contexts")) {
            for (QDomNode contextNode = element.firstChild();!contextNode.isNull();contextNode = contextNode.nextSibling()) {

                QDomElement contextItem = contextNode.toElement();

                if (contextItem.isNull()) continue;

                if (contextItem.attribute("name", "") == "") {
                    FATAL;
                }

                m_contexts[contextItem.attribute("name", "").toStdString()] = boost::make_shared<DGLHLContext>(contextItem, this);

                if (m_contexts.size() == 1) {
                    m_defContext = (*m_contexts.begin()).second.get();
                }
            }
        }

        for (auto i = m_contexts.begin(); i != m_contexts.end(); i++) {
            i->second->link(this);
        }

        QDomElement generalElement = docElem.firstChildElement("general");
        if (!generalElement.isNull()) {
            QDomElement keywordsElement = generalElement.firstChildElement("keywords");
            if (!keywordsElement.isNull()) {
                m_case_sensitive = (generalElement.attribute("casesensitive", "1") == "1");
                assert(m_case_sensitive); //only case sensitive currently implemented
            }
        }
    }

    const DGLHLContext* getDefaultContext() const {
        return m_defContext;
    }

    const DGLHLContext* getContext(std::string name) const {
        auto ret = m_contexts.find(name);
        if (ret == m_contexts.end()) {
            FATAL;
        }
        return (*ret).second.get();
    }

    const DGLHLTextCharFormat* getFormat(std::string name) const {
        auto ret = m_formats.find(name);
        if (ret == m_formats.end()) {
            FATAL;
        }
        return &(*ret).second;
    }

    const DGLHLRuleKeyword::keywordList_t* getKeywordList(std::string name) const {
        auto ret = m_lists.find(name);
        if (ret == m_lists.end()) {
            FATAL;
        }
        return &(*ret).second;
    }

private:
    bool m_case_sensitive;
    

    std::map<std::string, DGLHLRuleKeyword::keywordList_t> m_lists;
    
    std::map<std::string, boost::shared_ptr<DGLHLContext> > m_contexts;
    DGLHLContext* m_defContext;

    std::map<std::string, DGLHLTextCharFormat> m_formats;
};

DGLHLRuleKeyword::DGLHLRuleKeyword(const DGLHLData* data, std::string _string, QString formatName, QString actionName):DGLHLRuleBase(data, formatName, actionName, HL_DEBUG(QString(__FUNCTION__) + ":" + QString::fromStdString(_string))) {
    const keywordList_t* list = data->getKeywordList(_string);
    QString regexStr;
    for (size_t i = 0; i < list->size(); i++) {
        regexStr += QString(i?"|":"") + "\\b" + QRegExp::escape((*list)[i]);
    }
    m_regex = QRegExp(regexStr);
}


DGLHLActionBase* DGLHLActionBase::Create(const DGLHLData* data, const QString name) {
    if (name == "#stay") {
        return new DGLHLActionStay;
    } else if (name.left(strlen("#pop")) == "#pop") {
        int i = 0; 
        const int len = strlen("#pop");
        while (i * len < name.length()) {
            if (name.mid(i * len, len) != "#pop") {
                FATAL;
            }
            i++;
        }
        return new DGLHLActionPop(i);
    } else if (name[0] != '#') {
        return new DGLHLActionSetContext(data->getContext(name.toStdString()));
    } else
        FATAL;
}

DGLHLRuleBase::DGLHLRuleBase(const DGLHLData* data, QString formatName, QString actionName, HL_DEBUG(QString debugRuleName)) {
    m_action = boost::shared_ptr<DGLHLActionBase>(DGLHLActionBase::Create(data, actionName));
    m_format = data->getFormat(formatName.toStdString());
    HL_DEBUG(m_debugRuleName =  formatName + ":" + debugRuleName);
}

DGLHLContext::DGLHLContext(const QDomElement& xml, const DGLHLData* data): m_xml(xml) {
    if (xml.isNull() || xml.tagName() != "context") {
        FATAL;
    }

    m_LineEndAction = boost::make_shared<DGLHLActionStay>();

    for (int i = 0; i < xml.attributes().count(); i++) {
        QString name = xml.attributes().item(i).toAttr().nodeName();
        QString val = xml.attributes().item(i).toAttr().nodeValue();
        if (name == "name") {
            HL_DEBUG(m_debugName = val);
            continue;
        } else if (name == "attribute") {
            m_DefaultFormat = data->getFormat(val.toStdString());
        } else if (name == "lineEndContext") {
            m_LineEndAction = boost::shared_ptr<DGLHLActionBase>(DGLHLActionBase::Create(data, val));       
        } else {
            FATAL;
        }
    }
}

void DGLHLContext::link(const DGLHLData* data) {
    for (QDomNode node = m_xml.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement element = node.toElement();
        if (element.isNull()) {
            continue;
        }
        m_rules.push_back(boost::shared_ptr<DGLHLRuleBase>(DGLHLRuleBase::Create(data, element)));
    }
}
    
DGLSyntaxHighlighterGLSL::DGLSyntaxHighlighterGLSL(QTextDocument *parent): QSyntaxHighlighter(parent) {
    if (!s_data) {
        s_data = boost::make_shared<DGLHLData>();
    }
}

void DGLSyntaxHighlighterGLSL::highlightBlock(const QString &text) {
    HLState currentState(s_data.get());
    
    int previousStateIdx = previousBlockState();

    if (previousStateIdx != -1) {
        currentState = m_hlStates[previousBlockState()];
    }

    int pos = 0; 
    while (pos < text.length()) {
        DGLHLContext::HLResult res = currentState.getContext()->doHighlight(text.mid(pos));

        if (res.pos) {
            //format all unmatched text with default format of current context
            setFormat(pos, res.pos, currentState.getContext()->getDefaultFormat()->getFormat());
        }

        if (res.size) {
            assert(res.action);
            assert(res.format);
            //format all matched text with rule-dependent fotmat
            setFormat(pos + res.pos, res.size, res.format->getFormat());
        }           
        pos+= res.pos + res.size;

        if (res.action)
            res.action->doAction(currentState);

        if (!res.format && !res.action && (res.pos +  res.size == 0)) {
            //nothing was matched, do not try again
            break;
        }
    }

    if (currentState.getContext()->getLineEndAction()) {
        currentState.getContext()->getLineEndAction()->doAction(currentState);
    }

    if (previousStateIdx == -1 || true/*currentState != m_hlStates[previousStateIdx]*/) {
        m_hlStates.push_back(currentState);
        setCurrentBlockState(m_hlStates.size() - 1);
    }
}

DGLSyntaxHighlighterGLSL::HLState::HLState(const DGLHLData* data) {
    push(data->getDefaultContext());
}

const DGLHLContext* DGLSyntaxHighlighterGLSL::HLState::getContext() {
    return top();
}

boost::shared_ptr<DGLHLData> DGLSyntaxHighlighterGLSL::s_data;
