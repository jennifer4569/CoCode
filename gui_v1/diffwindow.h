#ifndef DIFFWINDOW_H
#define DIFFWINDOW_H

#include <QWidget>

namespace Ui {
class DiffWindow;
}

class DiffWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DiffWindow(QWidget *parent = nullptr);
    ~DiffWindow();

private:
    Ui::DiffWindow *ui;
};

#endif // DIFFWINDOW_H
