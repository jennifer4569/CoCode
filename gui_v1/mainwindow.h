#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <syntax_highlighter.h>
#include<QFile>
#include<QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileSystemModel>
#include <QtCore>
#include <QtGui>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionNew_triggered();

    void on_actionOpen_File_triggered();

    void on_actionSave_As_triggered();

    void on_actionPrint_triggered();

    void on_actionExit_triggered();

    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionCut_triggered();

    void on_actionUndo_triggered();

    void on_actionRedo_triggered();

    void on_actionSave_triggered();

    void on_actionNew_File_triggered();

    void on_tabWidget_tabCloseRequested(int index);

    void on_tabWidget_currentChanged(int index);

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

private:
    Ui::MainWindow *ui;
    QString currentFile = "";
    QPointer<syntax_highlighter> highlighter;
    QFileSystemModel *model;
};
#endif // MAINWINDOW_H
