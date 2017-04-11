#include "synchrodialog.h"
#include "ui_synchrodialog.h"
#include <QComboBox>
#include <QSpinBox>
#include "scatool.h"

SynchroDialog::SynchroDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SynchroDialog)
{
    ui->setupUi(this);

    ui->methodcombo->addItem("Sum Of Differences");

    this->synchropasses = new QList<Synchro *>();

    Synchro *sync = new Synchro(0);
    synchropasses->append(sync);

    ui->stepcombo->addItem("Pass 1");

    this->rubpattern = new QRubberBand(QRubberBand::Line,ScaTool::main_plot);
    this->rubsearch = new QRubberBand(QRubberBand::Line,ScaTool::main_plot);
}

void SynchroDialog::addRefItem(QString name)
{
    ui->refcombo->addItem(name);
}

SynchroDialog::~SynchroDialog()
{
    delete ui;
}

void SynchroDialog::on_leftpattern_valueChanged(int arg1)
{
    // Get pass by ui->stepcombo->itemText()
    //QPointF pt = ScaTool::main_plot->chart()->mapToValue();
}

void SynchroDialog::on_runpreview_pressed()
{
    QList<Curve *> *curve_displayed = new QList<Curve *>();

    for (int i = 0; i < ScaTool::curves->length(); i++)
    {
        if (ScaTool::curves->at(i)->displayed == true)
            curve_displayed->append(ScaTool::curves->at(i));
    }

    Curve *ref_curve = ScaTool::getCurveByName(ui->refcombo->currentText());
    if (curve_displayed->indexOf(ref_curve) == -1)
    {
        printf("Error reference curve not displayed\n");
        return;
    }
    else
    {
        ScaTool::sync_sod(curve_displayed, ref_curve, ui->leftwindow->text().toInt(), ui->rightwindow->text().toInt(), ui->leftpattern->text().toInt(), ui->rightpattern->text().toInt(),ui->precision->text().toInt());
    }

}

void SynchroDialog::on_runsynchro_pressed()
{
    ScaTool::sync_sod(ScaTool::curves, ScaTool::getCurveByName(ui->refcombo->currentText()), ui->leftwindow->text().toInt(), ui->rightwindow->text().toInt(), ui->leftpattern->text().toInt(), ui->rightpattern->text().toInt(),ui->precision->text().toInt());
}
