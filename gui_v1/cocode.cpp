#include "cocode.h"
#include "linenumber.h"
#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>
//CoCode::CoCode()
//{
    CoCode::CoCode(QWidget *parent) : QPlainTextEdit(parent)
    {
        linenumber = new class linenumber(this);

        connect(this, &CoCode::blockCountChanged, this, &CoCode::updatelinenumberWidth);
        connect(this, &CoCode::updateRequest, this, &CoCode::updatelinenumber);
        connect(this, &CoCode::cursorPositionChanged, this, &CoCode::highlightCurrentLine);
        updatelinenumberWidth(0);
        highlightCurrentLine();
    }
    int CoCode::linenumberWidth()
    {
        int digits = 1;
        int max = qMax(1, blockCount());
        while (max >= 10) {
            max /= 10;
            ++digits;
        }

        int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

        return space;
    }
    void CoCode::updatelinenumberWidth(int /* newBlockCount */)
    {
        setViewportMargins(linenumberWidth(), 0, 0, 0);
    }
    void CoCode::updatelinenumber(const QRect &rect, int dy)
    {
        if (dy)
            linenumber->scroll(0, dy);
        else
            linenumber->update(0, rect.y(), linenumber->width(), rect.height());

        if (rect.contains(viewport()->rect()))
            updatelinenumberWidth(0);
    }
    void CoCode::resizeEvent(QResizeEvent *e)
    {
        QPlainTextEdit::resizeEvent(e);

        QRect cr = contentsRect();
        linenumber->setGeometry(QRect(cr.left(), cr.top(), linenumberWidth(), cr.height()));
    }
    void CoCode::highlightCurrentLine()
    {
        QList<QTextEdit::ExtraSelection> extraSelections;

        if (!isReadOnly()) {
            QTextEdit::ExtraSelection selection;

            QColor lineColor = QColor(Qt::yellow).lighter(160);

            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }

        setExtraSelections(extraSelections);
    }
    void CoCode::linenumberPaintEvent(QPaintEvent *event)
    {
        QPainter painter(linenumber);
        painter.fillRect(event->rect(), Qt::lightGray);
        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());
        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QString number = QString::number(blockNumber + 1);
                painter.setPen(Qt::black);
                painter.drawText(0, top, linenumber->width(), fontMetrics().height(),
                                 Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }
    }
//}
