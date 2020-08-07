#ifndef LINENUMBER_H
#define LINENUMBER_H
#include <QWidget>
#include "cocode.h"
class linenumber : public QWidget
{
public:
    linenumber(CoCode *editor) : QWidget(editor), Co(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(Co->linenumberWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Co->linenumberPaintEvent(event);
    }

private:
    CoCode *Co;
};
#endif // LINENUMBER_H
