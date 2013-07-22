#pragma warning(push)
#pragma warning(disable:4512) // assignment operator could not be generated
#include <QtGui>
#pragma warning(pop)

#include<QApplication>

#include "qhexedit_p.h"
#include "commands.h"

const int HEXCHARS_IN_LINE = 47;
const int GAP_ADR_HEX = 10;
const int GAP_HEX_ASCII = 16;
const int BYTES_PER_LINE = 16;

QHexEditPrivate::QHexEditPrivate(QScrollArea *_parent) : QWidget(_parent)
{
    _undoStack = new QUndoStack(this);

    _scrollArea = _parent;
    setAddressWidth(4);
    setAddressOffset(0);
    setAddressArea(true);
    setAsciiArea(false);
    setFloatArea(true);
    setHighlighting(true);
    setOverwriteMode(true);
    setReadOnly(false);
    setAddressAreaColor(QColor(0xd4, 0xd4, 0xd4, 0xff));
    setHighlightingColor(QColor(0xff, 0xff, 0x99, 0xff));
    setSelectionColor(QColor(0x6d, 0x9e, 0xff, 0xff));
    setFont(QFont("Courier", 10));

    _size = 0;
    resetSelection(0);

    setFocusPolicy(Qt::StrongFocus);

    connect(&_cursorTimer, SIGNAL(timeout()), this, SLOT(updateCursor()));
    _cursorTimer.setInterval(500);
    _cursorTimer.start();
}

void QHexEditPrivate::setAddressOffset(int offset)
{
    _xData.setAddressOffset(offset);
    adjust();
}

int QHexEditPrivate::addressOffset()
{
    return _xData.addressOffset();
}

void QHexEditPrivate::setData(const QByteArray &_data)
{
    _xData.setData(_data);
    _undoStack->clear();
    adjust();
    setCursorPos(0);
}

QByteArray QHexEditPrivate::data()
{
    return _xData.data();
}

void QHexEditPrivate::setAddressAreaColor(const QColor &color)
{
    _addressAreaColor = color;
    update();
}

QColor QHexEditPrivate::addressAreaColor()
{
    return _addressAreaColor;
}

void QHexEditPrivate::setHighlightingColor(const QColor &color)
{
    _highlightingColor = color;
    update();
}

QColor QHexEditPrivate::highlightingColor()
{
    return _highlightingColor;
}

void QHexEditPrivate::setSelectionColor(const QColor &color)
{
    _selectionColor = color;
    update();
}

QColor QHexEditPrivate::selectionColor()
{
    return _selectionColor;
}

void QHexEditPrivate::setReadOnly(bool readOnly)
{
    _readOnly = readOnly;
}

bool QHexEditPrivate::isReadOnly()
{
    return _readOnly;
}

XByteArray & QHexEditPrivate::xData()
{
    return _xData;
}

int QHexEditPrivate::indexOf(const QByteArray & ba, int from)
{
    if (from > (_xData.data().length() - 1))
        from = _xData.data().length() - 1;
    int idx = _xData.data().indexOf(ba, from);
    if (idx > -1)
    {
        int curPos = idx*2;
        setCursorPos(curPos + ba.length()*2);
        resetSelection(curPos);
        setSelection(curPos + ba.length()*2);
        ensureVisible();
    }
    return idx;
}

void QHexEditPrivate::insert(int index, const QByteArray & ba)
{
    if (ba.length() > 0)
    {
        if (_overwriteMode)
        {
            QUndoCommand *arrayCommand= new ArrayCommand(&_xData, ArrayCommand::replace, index, ba, ba.length());
            _undoStack->push(arrayCommand);
            emit dataChanged();
        }
        else
        {
            QUndoCommand *arrayCommand= new ArrayCommand(&_xData, ArrayCommand::insert, index, ba, ba.length());
            _undoStack->push(arrayCommand);
            emit dataChanged();
        }
    }
}

void QHexEditPrivate::insert(int index, char ch)
{
    QUndoCommand *charCommand = new CharCommand(&_xData, CharCommand::insert, index, ch);
    _undoStack->push(charCommand);
    emit dataChanged();
}

int QHexEditPrivate::lastIndexOf(const QByteArray & ba, int from)
{
    from -= ba.length();
    if (from < 0)
        from = 0;
    int idx = _xData.data().lastIndexOf(ba, from);
    if (idx > -1)
    {
        int curPos = idx*2;
        setCursorPos(curPos);
        resetSelection(curPos);
        setSelection(curPos + ba.length()*2);
        ensureVisible();
    }
    return idx;
}

void QHexEditPrivate::remove(int index, int len)
{
    if (len > 0)
    {
        if (len == 1)
        {
            if (_overwriteMode)
            {
                QUndoCommand *charCommand = new CharCommand(&_xData, CharCommand::replace, index, char(0));
                _undoStack->push(charCommand);
                emit dataChanged();
            }
            else
            {
                QUndoCommand *charCommand = new CharCommand(&_xData, CharCommand::remove, index, char(0));
                _undoStack->push(charCommand);
                emit dataChanged();
            }
        }
        else
        {
            QByteArray ba = QByteArray(len, char(0));
            if (_overwriteMode)
            {
                QUndoCommand *arrayCommand = new ArrayCommand(&_xData, ArrayCommand::replace, index, ba, ba.length());
                _undoStack->push(arrayCommand);
                emit dataChanged();
            }
            else
            {
                QUndoCommand *arrayCommand= new ArrayCommand(&_xData, ArrayCommand::remove, index, ba, len);
                _undoStack->push(arrayCommand);
                emit dataChanged();
            }
        }
    }
}

void QHexEditPrivate::replace(int index, char ch)
{
    QUndoCommand *charCommand = new CharCommand(&_xData, CharCommand::replace, index, ch);
    _undoStack->push(charCommand);
    resetSelection();
    emit dataChanged();
}

void QHexEditPrivate::replace(int index, const QByteArray & ba)
{
    QUndoCommand *arrayCommand= new ArrayCommand(&_xData, ArrayCommand::replace, index, ba, ba.length());
    _undoStack->push(arrayCommand);
    resetSelection();
    emit dataChanged();
}

void QHexEditPrivate::replace(int _pos, int len, const QByteArray &after)
{
    QUndoCommand *arrayCommand= new ArrayCommand(&_xData, ArrayCommand::replace, _pos, after, len);
    _undoStack->push(arrayCommand);
    resetSelection();
    emit dataChanged();
}

void QHexEditPrivate::setAddressArea(bool addressArea)
{
    _addressArea = addressArea;
    adjust();

    setCursorPos(_cursorPosition);
}

void QHexEditPrivate::setAddressWidth(int addressWidth)
{
    _xData.setAddressWidth(addressWidth);

    setCursorPos(_cursorPosition);
}

void QHexEditPrivate::setAsciiArea(bool asciiArea)
{
    _asciiArea = asciiArea;
    if (_floatArea && _asciiArea)
        _floatArea = false;
    adjust();
}

void QHexEditPrivate::setFloatArea(bool floatArea)
{
    _floatArea = floatArea;
    if (_floatArea && _asciiArea)
        _asciiArea = false;
    adjust();
}

void QHexEditPrivate::setFont(const QFont &_font)
{
    QWidget::setFont(_font);
    adjust();
}

void QHexEditPrivate::setHighlighting(bool mode)
{
    _highlighting = mode;
    update();
}

void QHexEditPrivate::setOverwriteMode(bool __overwriteMode)
{
    _overwriteMode = __overwriteMode;
}

bool QHexEditPrivate::overwriteMode()
{
    return _overwriteMode;
}

void QHexEditPrivate::redo()
{
    _undoStack->redo();
    emit dataChanged();
    setCursorPos(_cursorPosition);
    update();
}

void QHexEditPrivate::undo()
{
    _undoStack->undo();
    emit dataChanged();
    setCursorPos(_cursorPosition);
    update();
}

QString QHexEditPrivate::toRedableString()
{
    return _xData.toRedableString();
}


QString QHexEditPrivate::selectionToReadableString()
{
    return _xData.toRedableString(getSelectionBegin(), getSelectionEnd());
}

void QHexEditPrivate::keyPressEvent(QKeyEvent *_event)
{
    int charX = (_cursorX - _xPosHex) / _charWidth;
    int posX = (charX / 3) * 2 + (charX % 3);
    int posBa = (_cursorY / _charHeight) * BYTES_PER_LINE + posX / 2;


/*****************************************************************************/
/* Cursor movements */
/*****************************************************************************/

    if (_event->matches(QKeySequence::MoveToNextChar))
    {
        setCursorPos(_cursorPosition + 1);
        resetSelection(_cursorPosition);
    }
    if (_event->matches(QKeySequence::MoveToPreviousChar))
    {
        setCursorPos(_cursorPosition - 1);
        resetSelection(_cursorPosition);
    }
    if (_event->matches(QKeySequence::MoveToEndOfLine))
    {
        setCursorPos(_cursorPosition | (2 * BYTES_PER_LINE -1));
        resetSelection(_cursorPosition);
    }
    if (_event->matches(QKeySequence::MoveToStartOfLine))
    {
        setCursorPos(_cursorPosition - (_cursorPosition % (2 * BYTES_PER_LINE)));
        resetSelection(_cursorPosition);
    }
    if (_event->matches(QKeySequence::MoveToPreviousLine))
    {
        setCursorPos(_cursorPosition - (2 * BYTES_PER_LINE));
        resetSelection(_cursorPosition);
    }
    if (_event->matches(QKeySequence::MoveToNextLine))
    {
        setCursorPos(_cursorPosition + (2 * BYTES_PER_LINE));
        resetSelection(_cursorPosition);
    }

    if (_event->matches(QKeySequence::MoveToNextPage))
    {
        setCursorPos(_cursorPosition + (((_scrollArea->viewport()->height() / _charHeight) - 1) * 2 * BYTES_PER_LINE));
        resetSelection(_cursorPosition);
    }
    if (_event->matches(QKeySequence::MoveToPreviousPage))
    {
        setCursorPos(_cursorPosition - (((_scrollArea->viewport()->height() / _charHeight) - 1) * 2 * BYTES_PER_LINE));
        resetSelection(_cursorPosition);
    }
    if (_event->matches(QKeySequence::MoveToEndOfDocument))
    {
        setCursorPos(_xData.size() * 2);
        resetSelection(_cursorPosition);
    }
    if (_event->matches(QKeySequence::MoveToStartOfDocument))
    {
        setCursorPos(0);
        resetSelection(_cursorPosition);
    }

/*****************************************************************************/
/* Select commands */
/*****************************************************************************/
    if (_event->matches(QKeySequence::SelectAll))
    {
        resetSelection(0);
        setSelection(2*_xData.size() + 1);
    }
    if (_event->matches(QKeySequence::SelectNextChar))
    {
        int _pos = _cursorPosition + 1;
        setCursorPos(_pos);
        setSelection(_pos);
    }
    if (_event->matches(QKeySequence::SelectPreviousChar))
    {
        int _pos = _cursorPosition - 1;
        setSelection(_pos);
        setCursorPos(_pos);
    }
    if (_event->matches(QKeySequence::SelectEndOfLine))
    {
        int _pos = _cursorPosition - (_cursorPosition % (2 * BYTES_PER_LINE)) + (2 * BYTES_PER_LINE);
        setCursorPos(_pos);
        setSelection(_pos);
    }
    if (_event->matches(QKeySequence::SelectStartOfLine))
    {
        int _pos = _cursorPosition - (_cursorPosition % (2 * BYTES_PER_LINE));
        setCursorPos(_pos);
        setSelection(_pos);
    }
    if (_event->matches(QKeySequence::SelectPreviousLine))
    {
        int _pos = _cursorPosition - (2 * BYTES_PER_LINE);
        setCursorPos(_pos);
        setSelection(_pos);
    }
    if (_event->matches(QKeySequence::SelectNextLine))
    {
        int _pos = _cursorPosition + (2 * BYTES_PER_LINE);
        setCursorPos(_pos);
        setSelection(_pos);
    }

    if (_event->matches(QKeySequence::SelectNextPage))
    {
        int _pos = _cursorPosition + (((_scrollArea->viewport()->height() / _charHeight) - 1) * 2 * BYTES_PER_LINE);
        setCursorPos(_pos);
        setSelection(_pos);
    }
    if (_event->matches(QKeySequence::SelectPreviousPage))
    {
        int _pos = _cursorPosition - (((_scrollArea->viewport()->height() / _charHeight) - 1) * 2 * BYTES_PER_LINE);
        setCursorPos(_pos);
        setSelection(_pos);
    }
    if (_event->matches(QKeySequence::SelectEndOfDocument))
    {
        int _pos = _xData.size() * 2;
        setCursorPos(_pos);
        setSelection(_pos);
    }
    if (_event->matches(QKeySequence::SelectStartOfDocument))
    {
        int _pos = 0;
        setCursorPos(_pos);
        setSelection(_pos);
    }

/*****************************************************************************/
/* Edit Commands */
/*****************************************************************************/
if (!_readOnly)
{
    /* Hex input */
        QChar key = _event->text()[0];
        if ((key>='0' && key<='9') || (key>='a' && key <= 'f'))
        {
            if (getSelectionBegin() != getSelectionEnd())
            {
                posBa = getSelectionBegin();
                remove(posBa, getSelectionEnd() - posBa);
                setCursorPos(2*posBa);
                resetSelection(2*posBa);
            }

            // If insert mode, then insert a byte
            if (_overwriteMode == false)
                if ((charX % 3) == 0)
                {
                    insert(posBa, char(0));
                }

            // Change content
            if (_xData.size() > 0)
            {
                QByteArray hexValue = _xData.data().mid(posBa, 1).toHex();
                if ((charX % 3) == 0)
                    hexValue[0] = key.toLatin1();
                else
                    hexValue[1] = key.toLatin1();

                replace(posBa, QByteArray().fromHex(hexValue)[0]);

                setCursorPos(_cursorPosition + 1);
                resetSelection(_cursorPosition);
            }
        }

        /* Cut & Paste */
        if (_event->matches(QKeySequence::Cut))
        {
            QString result = QString();
            for (int idx = getSelectionBegin(); idx < getSelectionEnd(); idx++)
            {
                result += _xData.data().mid(idx, 1).toHex() + " ";
                if ((idx % 16) == 15)
                    result.append("\n");
            }
            remove(getSelectionBegin(), getSelectionEnd() - getSelectionBegin());
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(result);
            setCursorPos(getSelectionBegin());
            resetSelection(getSelectionBegin());
        }

        if (_event->matches(QKeySequence::Paste))
        {
            QClipboard *clipboard = QApplication::clipboard();
            QByteArray ba = QByteArray().fromHex(clipboard->text().toLatin1());
            insert(_cursorPosition / 2, ba);
            setCursorPos(_cursorPosition + 2 * ba.length());
            resetSelection(getSelectionBegin());
        }


        /* Delete char */
        if (_event->matches(QKeySequence::Delete))
        {
            if (getSelectionBegin() != getSelectionEnd())
            {
                posBa = getSelectionBegin();
                remove(posBa, getSelectionEnd() - posBa);
                setCursorPos(2*posBa);
                resetSelection(2*posBa);
            }
            else
            {
                if (_overwriteMode)
                    replace(posBa, char(0));
                else
                    remove(posBa, 1);
            }
        }

        /* Backspace */
        if ((_event->key() == Qt::Key_Backspace) && (_event->modifiers() == Qt::NoModifier))
            {
                if (getSelectionBegin() != getSelectionEnd())
                {
                    posBa = getSelectionBegin();
                    remove(posBa, getSelectionEnd() - posBa);
                    setCursorPos(2*posBa);
                    resetSelection(2*posBa);
                }
                else
                {
                    if (posBa > 0)
                    {
                        if (_overwriteMode)
                            replace(posBa - 1, char(0));
                        else
                            remove(posBa - 1, 1);
                        setCursorPos(_cursorPosition - 2);
                    }
                }
            }

        /* undo */
        if (_event->matches(QKeySequence::Undo))
        {
            undo();
        }

        /* redo */
        if (_event->matches(QKeySequence::Redo))
        {
            redo();
        }

    }

    if (_event->matches(QKeySequence::Copy))
    {
        QString result = QString();
        for (int idx = getSelectionBegin(); idx < getSelectionEnd(); idx++)
        {
            result += _xData.data().mid(idx, 1).toHex() + " ";
            if ((idx % 16) == 15)
                result.append('\n');
        }
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(result);
    }

    // Switch between insert/overwrite mode
    if ((_event->key() == Qt::Key_Insert) && (_event->modifiers() == Qt::NoModifier))
    {
        _overwriteMode = !_overwriteMode;
        setCursorPos(_cursorPosition);
        overwriteModeChanged(_overwriteMode);
    }

    ensureVisible();
    update();
}

void QHexEditPrivate::mouseMoveEvent(QMouseEvent * _event)
{
    _blink = false;
    update();
    int actPos = cursorPos(_event->pos());
    setCursorPos(actPos);
    setSelection(actPos);
}

void QHexEditPrivate::mousePressEvent(QMouseEvent * _event)
{
    _blink = false;
    update();
    int cPos = cursorPos(_event->pos());
    resetSelection(cPos);
    setCursorPos(cPos);
}

void QHexEditPrivate::paintEvent(QPaintEvent *_event)
{
    QPainter painter(this);

    // draw some patterns if needed
    painter.fillRect(_event->rect(), this->palette().color(QPalette::Base));
    if (_addressArea)
        painter.fillRect(QRect(_xPosAdr, _event->rect().top(), _xPosHex - GAP_ADR_HEX + 2, height()), _addressAreaColor);
    if (_asciiArea || _floatArea)
    {
        int linePos = _xPosAscii - (GAP_HEX_ASCII / 2);
        painter.setPen(Qt::gray);
        painter.drawLine(linePos, _event->rect().top(), linePos, height());
    }

    painter.setPen(this->palette().color(QPalette::WindowText));

    // calc position
    int firstLineIdx = ((_event->rect().top()/ _charHeight) - _charHeight) * BYTES_PER_LINE;
    if (firstLineIdx < 0)
        firstLineIdx = 0;
    int lastLineIdx = ((_event->rect().bottom() / _charHeight) + _charHeight) * BYTES_PER_LINE;
    if (lastLineIdx > _xData.size())
        lastLineIdx = _xData.size();
    int yPosStart = ((firstLineIdx) / BYTES_PER_LINE) * _charHeight + _charHeight;

    // paint address area
    if (_addressArea)
    {
        for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx; lineIdx += BYTES_PER_LINE, yPos +=_charHeight)
        {
            QString address = QString("%1")
                              .arg(lineIdx + _xData.addressOffset(), _xData.realAddressNumbers(), 16, QChar('0'));
            painter.drawText(_xPosAdr, yPos, address);
        }
    }

    // paint hex area
    QByteArray hexBa(_xData.data().mid(firstLineIdx, lastLineIdx - firstLineIdx + 1).toHex());
    QBrush highLighted = QBrush(_highlightingColor);
    QPen colHighlighted = QPen(this->palette().color(QPalette::WindowText));
    QBrush selected = QBrush(_selectionColor);
    QPen colSelected = QPen(Qt::white);
    QPen colStandard = QPen(this->palette().color(QPalette::WindowText));

    painter.setBackgroundMode(Qt::TransparentMode);

    for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx; lineIdx += BYTES_PER_LINE, yPos +=_charHeight)
    {
        QByteArray hex;
        int xPos = _xPosHex;
        for (int colIdx = 0; ((lineIdx + colIdx) < _xData.size() && (colIdx < BYTES_PER_LINE)); colIdx++)
        {
            int posBa = lineIdx + colIdx;
            if ((getSelectionBegin() <= posBa) && (getSelectionEnd() > posBa))
            {
                painter.setBackground(selected);
                painter.setBackgroundMode(Qt::OpaqueMode);
                painter.setPen(colSelected);
            }
            else
            {
                if (_highlighting)
                {
                    // hilight diff bytes
                    painter.setBackground(highLighted);
                    if (_xData.dataChanged(posBa))
                    {
                        painter.setPen(colHighlighted);
                        painter.setBackgroundMode(Qt::OpaqueMode);
                    }
                    else
                    {
                        painter.setPen(colStandard);
                        painter.setBackgroundMode(Qt::TransparentMode);
                    }
                }
            }

            // render hex value
            if (colIdx == 0)
            {
                hex = hexBa.mid((lineIdx - firstLineIdx) * 2, 2);
                painter.drawText(xPos, yPos, hex);
                xPos += 2 * _charWidth;
            } else {
                hex = hexBa.mid((lineIdx + colIdx - firstLineIdx) * 2, 2).prepend(" ");
                painter.drawText(xPos, yPos, hex);
                xPos += 3 * _charWidth;
            }

        }
    }
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setPen(this->palette().color(QPalette::WindowText));

    // paint ascii area
    if (_asciiArea)
    {
        for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx; lineIdx += BYTES_PER_LINE, yPos +=_charHeight)
        {
            int xPosAscii = _xPosAscii;
            for (int colIdx = 0; ((lineIdx + colIdx) < _xData.size() && (colIdx < BYTES_PER_LINE)); colIdx++)
            {
                painter.drawText(xPosAscii, yPos, _xData.asciiChar(lineIdx + colIdx));
                xPosAscii += _charWidth;
            }
        }
    } else if (_floatArea) // paint float area
    {
        for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx; lineIdx += BYTES_PER_LINE, yPos +=_charHeight)
        {
            int xPosAscii = _xPosAscii;
            for (int colIdx = 0; ((lineIdx + colIdx) < _xData.size() && (colIdx < BYTES_PER_LINE)); colIdx+=sizeof(float))
            {
                painter.drawText(xPosAscii, yPos, _xData.floatString8(lineIdx + colIdx) + " ");
                xPosAscii += _charWidth * 9;
            }
        }
    }

    // paint cursor
    if (_blink && !_readOnly && hasFocus())
    {
        if (_overwriteMode)
            painter.fillRect(_cursorX, _cursorY + _charHeight - 2, _charWidth, 2, this->palette().color(QPalette::WindowText));
        else
            painter.fillRect(_cursorX, _cursorY, 2, _charHeight, this->palette().color(QPalette::WindowText));
    }

    if (_size != _xData.size())
    {
        _size = _xData.size();
        emit currentSizeChanged(_size);
    }
}

void QHexEditPrivate::setCursorPos(int position)
{
    // delete cursor
    _blink = false;
    update();

    // cursor in range?
    if (_overwriteMode)
    {
        if (position > (_xData.size() * 2 - 1))
            position = _xData.size() * 2 - 1;
    } else {
        if (position > (_xData.size() * 2))
            position = _xData.size() * 2;
    }

    if (position < 0)
        position = 0;

    // calc position
    _cursorPosition = position;
    _cursorY = (position / (2 * BYTES_PER_LINE)) * _charHeight + 4;
    int _x = (position % (2 * BYTES_PER_LINE));
    _cursorX = (((_x / 2) * 3) + (_x % 2)) * _charWidth + _xPosHex;

    // immiadately draw cursor
    _blink = true;
    update();
    emit currentAddressChanged(_cursorPosition/2);
}

int QHexEditPrivate::cursorPos(QPoint _pos)
{
    int result = -1;
    // find char under cursor
    if ((_pos.x() >= _xPosHex) && (_pos.x() < (_xPosHex + HEXCHARS_IN_LINE * _charWidth)))
    {
        int _x = (_pos.x() - _xPosHex) / _charWidth;
        if ((_x % 3) == 0)
            _x = (_x / 3) * 2;
        else
            _x = ((_x / 3) * 2) + 1;
        int _y = ((_pos.y() - 3) / _charHeight) * 2 * BYTES_PER_LINE;
        result = _x + _y;
    }
    return result;
}

int QHexEditPrivate::cursorPos()
{
    return _cursorPosition;
}

void QHexEditPrivate::resetSelection()
{
    _selectionBegin = _selectionInit;
    _selectionEnd = _selectionInit;
}

void QHexEditPrivate::resetSelection(int _pos)
{
    if (_pos < 0)
        _pos = 0;
    _pos = _pos / 2;
    _selectionInit = _pos;
    _selectionBegin = _pos;
    _selectionEnd = _pos;
}

void QHexEditPrivate::setSelection(int _pos)
{
    if (_pos < 0)
        _pos = 0;
    _pos = _pos / 2;
    if (_pos >= _selectionInit)
    {
        _selectionEnd = _pos;
        _selectionBegin = _selectionInit;
    }
    else
    {
        _selectionBegin = _pos;
        _selectionEnd = _selectionInit;
    }
}

int QHexEditPrivate::getSelectionBegin()
{
    return _selectionBegin;
}

int QHexEditPrivate::getSelectionEnd()
{
    return _selectionEnd;
}


void QHexEditPrivate::updateCursor()
{
    if (_blink)
        _blink = false;
    else
        _blink = true;
    update(_cursorX, _cursorY, _charWidth, _charHeight);
}

void QHexEditPrivate::adjust()
{
    _charWidth = fontMetrics().width(QLatin1Char('9'));
    _charHeight = fontMetrics().height();

    _xPosAdr = 0;
    if (_addressArea)
        _xPosHex = _xData.realAddressNumbers()*_charWidth + GAP_ADR_HEX;
    else
        _xPosHex = 0;
    _xPosAscii = _xPosHex + HEXCHARS_IN_LINE * _charWidth + GAP_HEX_ASCII;

    // tell QAbstractScollbar, how big we are
    setMinimumHeight(((_xData.size()/16 + 1) * _charHeight) + 5);
    if(_asciiArea)
        setMinimumWidth(_xPosAscii + (BYTES_PER_LINE * _charWidth));
    else
        setMinimumWidth(_xPosHex + HEXCHARS_IN_LINE * _charWidth);

    update();
}

void QHexEditPrivate::ensureVisible()
{
    // scrolls to cursorx, cusory (which are set by setCursorPos)
    // x-margin is 3 pixels, y-margin is half of charHeight
    _scrollArea->ensureVisible(_cursorX, _cursorY + _charHeight/2, 3, _charHeight/2 + 2);
}
