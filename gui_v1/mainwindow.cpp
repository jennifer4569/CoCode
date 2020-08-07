#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cocode.h>
#include <QApplication>
#include <QTextEdit>
#include <QTimer>
#include <QFont>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    ui->tabWidget->addTab(new CoCode(),"untitled");

    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    highlighter = new syntax_highlighter(pTextEdit->document());

    model = new QFileSystemModel(this);
    model->setRootPath("C:/");

    model->setReadOnly(false);

    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->setRootPath("./"));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionNew_triggered()
{
    MainWindow* w = new MainWindow();
    w->showMaximized();
    w->show();
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open this file");
    QFile file(fileName);
    currentFile = fileName;
    if(!file.open(QIODevice::ReadOnly | QFile::Text)){
        QMessageBox::warning(this,"Warning","Cannot open file : " + file.errorString());
        return;
    }
   // setWindowTitle(fileName);
    QTextStream in(&file);
    QString text = in.readAll();
    if(ui->tabWidget->count()!=0){
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),fileName);
        CoCode* pTextEdit = NULL;
        QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
        pTextEdit = (CoCode*) pWidget;
        pTextEdit->setPlainText(text);
        highlighter->setDocument(pTextEdit->document());
    }
    else{
        //add tab
        ui->tabWidget->addTab(new CoCode(),"untitled");
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),fileName);
        CoCode* pTextEdit = NULL;
        QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
        pTextEdit = (CoCode*) pWidget;
        pTextEdit->setPlainText(text);
        highlighter->setDocument(pTextEdit->document());
    }
    file.close();
}

void MainWindow::on_actionSave_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save as");
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QFile::Text)){
        QMessageBox::warning(this,"Warning","Cannot save file : " + file.errorString());
        return;
    }
    currentFile = fileName;
    setWindowTitle(fileName);
    QTextStream out(&file);
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),fileName);
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    QString text =  pTextEdit->toPlainText();
    out << text;
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),fileName);
    file.close();
}

void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer;
    printer.setPrinterName("Printer Name");
    QPrintDialog pDialog(&printer,this);
    if(pDialog.exec()==QDialog::Rejected){
        QMessageBox::warning(this, "Warning","Cannot Access Printer");
        return;
    }
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    pTextEdit->print(&printer);
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionCopy_triggered()
{
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    pTextEdit->copy();
}


void MainWindow::on_actionPaste_triggered()
{
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    pTextEdit->paste();
}


void MainWindow::on_actionCut_triggered()
{
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    pTextEdit->cut();
}

void MainWindow::on_actionUndo_triggered()
{
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    pTextEdit->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    pTextEdit->redo();}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save");
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QFile::Text)){
        QMessageBox::warning(this,"Warning","Cannot save file : " + file.errorString());
        return;
    }
    currentFile = fileName;
    setWindowTitle(fileName);
    QTextStream out(&file);
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),fileName);
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;

    QString text = pTextEdit->toPlainText();
    out << text;
    file.close();
}

void MainWindow::on_actionNew_File_triggered()
{
    ui->tabWidget->addTab(new CoCode(),"untitled");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    highlighter->setDocument(pTextEdit->document());
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    ui->tabWidget->removeTab(index);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index>0){
        CoCode* pTextEdit = NULL;
        QWidget* pWidget= ui->tabWidget->widget(index);
        pTextEdit = (CoCode*) pWidget;
        highlighter->setDocument(pTextEdit->document());
    }
    else if(index==0){
        //seg faults here for some reason
        CoCode* pTextEdit = NULL;
        QWidget* pWidget= ui->tabWidget->widget(index);
        pTextEdit = (CoCode*) pWidget;
        highlighter = new syntax_highlighter(pTextEdit->document());
       /* if(ui->tabWidget->widget(index)!=NULL){
            CoCode* pTextEdit = NULL;
            QWidget* pWidget= ui->tabWidget->widget(index);
            pTextEdit = (CoCode*) pWidget;
            highlighter->setDocument(pTextEdit->document());
        }*/
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    pTextEdit->zoomIn();
}

void MainWindow::on_actionZoom_Out_triggered()
{
    CoCode* pTextEdit = NULL;
    QWidget* pWidget= ui->tabWidget->widget(ui->tabWidget->currentIndex());
    pTextEdit = (CoCode*) pWidget;
    pTextEdit->zoomOut();
}
