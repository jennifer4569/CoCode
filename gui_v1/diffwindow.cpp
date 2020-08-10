#include "diffwindow.h"
#include "ui_diffwindow.h"

DiffWindow::DiffWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiffWindow)
{
    ui->setupUi(this);
    setWindowTitle("Diff");

    ui->plainTextEdit->setReadOnly(true);
    ui->plainTextEdit_2->setReadOnly(true);
}

DiffWindow::~DiffWindow()
{
    delete ui;
}
