#include "synchrodialog.h"
#include "ui_synchrodialog.h"
#include <QComboBox>
#include <QSpinBox>
#include <QErrorMessage>
#include "scatool.h"

QProgressBar *  SynchroDialog::qprogressbar = 0;
QLineSeries *   SynchroDialog::dn = 0;

SynchroDialog::SynchroDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SynchroDialog)
{
    ui->setupUi(this);

    ui->methodcombo->addItem("Sum Of Differences");

    this->synchropasses = QList<Synchro *>();

    this->rubpattern = new QRubberBand(QRubberBand::Line,ScaTool::main_plot);
    this->rubsearch = new QRubberBand(QRubberBand::Line,ScaTool::main_plot);

    connect(ui->leftpattern,&QSpinBox::editingFinished,this,&SynchroDialog::pattern_value_changed);
    connect(ui->rightpattern,&QSpinBox::editingFinished,this,&SynchroDialog::pattern_value_changed);
    connect(ui->leftwindow,&QSpinBox::editingFinished,this,&SynchroDialog::window_value_changed);
    connect(ui->rightwindow,&QSpinBox::editingFinished,this,&SynchroDialog::window_value_changed);
}

void SynchroDialog::addRefItem(QString name)
{
    ui->refcombo->addItem(name);
}

SynchroDialog::~SynchroDialog()
{
    delete ui;
    qDeleteAll(synchropasses);
}

void SynchroDialog::on_runpreview_pressed()
{
    QList<Curve *> curve_displayed  = QList<Curve *>();
    for (int i = 0; i < ScaTool::curves->length(); i++)
    {
        if (ScaTool::curves->at(i)->displayed == true)
            curve_displayed.append(ScaTool::curves->at(i));
    }

    Curve *ref_curve = ScaTool::getCurveByName(ui->refcombo->currentText());
    if (curve_displayed.indexOf(ref_curve) == -1)
    {
        printf("Error reference curve not displayed\n");
        return;
    }
    else
    {
        ScaTool::sync_sod(&curve_displayed, ref_curve, ui->leftwindow->text().toInt(), ui->rightwindow->text().toInt(), ui->leftpattern->text().toInt(), ui->rightpattern->text().toInt(),ui->precision->text().toInt());
    }

}

void SynchroDialog::on_runsynchro_pressed()
{
    if (SynchroDialog::qprogressbar != 0)
    {
        QErrorMessage *qerror = new QErrorMessage();
        qerror->showMessage("Synchronization is on going please be patient");
    }
    else
    {
        // Trick to set length - 2 as the reference curve is done synchronized
        SynchroDialog::qprogressbar = new QProgressBar(this);
        SynchroDialog::qprogressbar->setMinimum(0);
        SynchroDialog::qprogressbar->setMaximum(ScaTool::curves->length()-2);
        ScaTool::statusbar->addPermanentWidget(SynchroDialog::qprogressbar);
        SynchroDialog::qprogressbar->show();
        ScaTool::sync_sod(ScaTool::curves, ScaTool::getCurveByName(ui->refcombo->currentText()), ui->leftwindow->text().toInt(), ui->rightwindow->text().toInt(), ui->leftpattern->text().toInt(), ui->rightpattern->text().toInt(),ui->precision->text().toInt());
    }
}


void SynchroDialog::update_progressdialog()
{
    if (SynchroDialog::qprogressbar)
    {
        SynchroDialog::qprogressbar->setValue(SynchroDialog::qprogressbar->value()+1);
        if(SynchroDialog::qprogressbar->value() == (SynchroDialog::qprogressbar->maximum() - 1))
        {
            ScaTool::statusbar->removeWidget(SynchroDialog::qprogressbar);
            delete SynchroDialog::qprogressbar;
            SynchroDialog::qprogressbar = 0;
        }
    }
}

void SynchroDialog::on_addstep_pressed()
{

    Synchro *sync = new Synchro(0);
    synchropasses.append(sync);

    ui->stepcombo->addItem("Pass "+QString::number(synchropasses.length()));
}



void SynchroDialog::pattern_value_changed()
{
    qreal l = ui->leftpattern->text().toDouble();
    qreal r = ui->rightpattern->text().toDouble();
    qreal w = r-l;



    /*QLineSeries *up = new QLineSeries();
    up->append(l,-0.4);
    up->append(r,-0.4);
    up->attachAxis(ScaTool::main_plot->chart()->axisX());
    up->attachAxis(ScaTool::main_plot->chart()->axisY());
*/
    if (flag)
    {
        flag = false;
        dn = new QLineSeries();
        ScaTool::main_plot->chart()->addSeries(dn);
    }
    dn->clear();
    dn->append(l,-0.4);
    dn->append(r,-0.4);
    dn->attachAxis(ScaTool::main_plot->chart()->axisX());
    dn->attachAxis(ScaTool::main_plot->chart()->axisY());


/*    ScaTool::main_plot->chart()->removeSeries(pol);
    delete pol;
    pol = new QAreaSeries();
    pol->setLowerSeries(dn);
    pol->attachAxis(ScaTool::main_plot->chart()->axisX());
    pol->attachAxis(ScaTool::main_plot->chart()->axisY());
    ScaTool::main_plot->chart()->addSeries(pol);
    */
}

void SynchroDialog::window_value_changed()
{

}

