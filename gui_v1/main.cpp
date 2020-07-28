#include "mainwindow.h"

#include <QApplication>
#include <QFileSystemModel>
#include <QTreeView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    w.show();
    return a.exec();
}
