#include "attackwindow.h"
#include "ui_attackwindow.h"

attackwindow::attackwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::attackwindow)
{
    ui->setupUi(this);
}

attackwindow::~attackwindow()
{
    delete ui;
}
