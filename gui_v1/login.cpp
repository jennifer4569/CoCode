#include "login.h"
#include "ui_login.h"
#include <QSqlDatabase>
#include <QDebug>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSql>
#include <sys/types.h>
#include "mainwindow.h"
login::login(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    ui->groupBox->resize(700,500);
    db=QSqlDatabase::addDatabase("QSQLITE");
    //change path as needed depending on where database is
  //  db.setDatabaseName("C:/Users/senfr/Documents/CoCode/remote_repo/CoCode/source_control/server_files/login.db/");
    if(!db.open()){
      qDebug() << "Error opening database: " << db.lastError();
    }

}

login::~login()
{
    delete ui;
}


bool login::validate_credentials(QString username, QString password){
    db.setDatabaseName("login.db");
    if(!db.open()){
    //  qDebug() << "Error opening database: " << db.lastError();
    }


    QString command  = "SELECT COUNT(*) FROM USERS WHERE ";
    command += "username='" + username + "' AND ";
    command += "password='" + password + "';";
    QSqlQuery qry;
    if(!qry.exec(command)){
      qDebug() << "Error: " << qry.lastError();
      db.close();
      //is this right??
      return EXIT_FAILURE;
    }
    else{
        int count =0;
        while(qry.next()){
            count++;
        }
        if(count==1){
            return true;
        }
        else{
            return false;
        }

    }
}
void login::on_pushButton_clicked()
{
    QString username,password;
    username=ui->username->text();
    password=ui->password->text();
    bool check = true;//validate_credentials(username,password);
    //if correct credentials
    if(check){
        close();
        MainWindow* w = new MainWindow;
        w->showMaximized();
        w->show();
    }
    else{
        qInfo()<<"not valid credentials";
    }
}

//new user button
