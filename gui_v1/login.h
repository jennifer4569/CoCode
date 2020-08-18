#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>

#include <QSql>
#include <QFileInfo>
#include <QDebug>
#include <QSqlDatabase>

namespace Ui {
class login;
}

class login : public QMainWindow
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();
    bool validate_credentials(QString username, QString password);
    bool valid(QString str);
private slots:
    void on_pushButton_clicked();

private:
    Ui::login *ui;
    QSqlDatabase db;
};

#endif // LOGIN_H
