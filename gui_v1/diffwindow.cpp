#include "diffwindow.h"
#include "ui_diffwindow.h"

DiffWindow::DiffWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiffWindow)
{
    ui->setupUi(this);
}

DiffWindow::~DiffWindow()
{
    delete ui;
}
