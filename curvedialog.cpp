#include "curvedialog.h"
#include "ui_curvedialog.h"
#include <QTableWidget>

CurveDialog::CurveDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveDialog)
{
    ui->setupUi(this);
}

CurveDialog::~CurveDialog()
{
    delete ui;
}

void CurveDialog::addItem()
{
}
