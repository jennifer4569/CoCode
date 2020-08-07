#ifndef COCODE_H
#define COCODE_H
#include <QPlainTextEdit>

class CoCode : public QPlainTextEdit
{
    Q_OBJECT

public:
    CoCode(QWidget *parent = nullptr);

    void linenumberPaintEvent(QPaintEvent *event);
    int linenumberWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updatelinenumberWidth(int newBlockCount);
    void highlightCurrentLine();
    void updatelinenumber(const QRect &rect, int dy);

private:
    QWidget *linenumber;
};

#endif // COCODE_H
