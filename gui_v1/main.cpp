#include "mainwindow.h"
#include "login.h"
#include <QApplication>
#include <QFileSystemModel>
#include <QTreeView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //if you want to open editor
    //change to: MainWindow w;
    MainWindow w;
     w.showMaximized();
    w.show();
    qInfo() << QDir::currentPath();
    return a.exec();
}
