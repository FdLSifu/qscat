#include "synchrodialog.h"
#include "ui_synchrodialog.h"
#include <QComboBox>
#include <QSpinBox>
#include <QErrorMessage>
#include <QValueAxis>
#include "scatool.h"

QProgressBar *  SynchroDialog::qprogressbar = 0;

SynchroDialog::SynchroDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SynchroDialog)
{
    ui->setupUi(this);

    ui->methodcombo->addItem("Sum Of Differences");

    this->synchropasses = QList<Synchro *>();

    this->pattern_bar = new QLineSeries();
    this->window_bar = new QLineSeries();

    connect(this,&SynchroDialog::rejected,this,&SynchroDialog::closed);
    connect(ui->leftpattern,&QSpinBox::editingFinished,this,&SynchroDialog::pattern_value_changed);
    connect(ui->rightpattern,&QSpinBox::editingFinished,this,&SynchroDialog::pattern_value_changed);
    connect(ui->leftwindow,&QSpinBox::editingFinished,this,&SynchroDialog::pattern_value_changed);
    connect(ui->rightwindow,&QSpinBox::editingFinished,this,&SynchroDialog::pattern_value_changed);
}

void SynchroDialog::show()
{
    this->window_bar->show();
    this->pattern_bar->show();
    qobject_cast<QDialog*>(this)->show();
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
    qreal lw = ui->leftwindow->text().toDouble();
    qreal rw = ui->rightwindow->text().toDouble();
    qreal lp = ui->leftpattern->text().toDouble();
    qreal rp = ui->rightpattern->text().toDouble();

    if ((ScaTool::main_plot->chart()->series().indexOf(this->pattern_bar) < 0) && (ScaTool::main_plot->chart()->series().indexOf(this->window_bar) < 0))
    {
        ScaTool::main_plot->chart()->addSeries(this->pattern_bar);
        ScaTool::main_plot->chart()->addSeries(this->window_bar);
    }
    if (ScaTool::main_plot->chart()->axes().length() != 0)
    {
        qreal ymin = qobject_cast<QValueAxis *>(ScaTool::main_plot->chart()->axisY())->min();
        qreal ymax = qobject_cast<QValueAxis *>(ScaTool::main_plot->chart()->axisY())->max();
        qreal pos = ymin + (5*(ymax-ymin)/100);

        QPen pp;
        pp.setWidth(10);
        pp.setCapStyle(Qt::RoundCap);
        this->pattern_bar->setPen(pp);
        this->pattern_bar->clear();

        QPen pw;
        pw.setWidth(10);
        pw.setBrush(Qt::Dense5Pattern);
        pw.setCapStyle(Qt::RoundCap);
        this->window_bar->setPen(pw);
        this->window_bar->clear();

        if (lp <= rp)
        {
            this->pattern_bar->append(lp,pos);
            this->pattern_bar->append(rp,pos);
            this->window_bar->append(lp+lw,pos);
            this->window_bar->append(rp+rw,pos);
        }
        {
            this->pattern_bar->append(lp,pos);
            this->pattern_bar->append(lp,pos);
            this->window_bar->append(lp,pos);
            this->window_bar->append(lp,pos);
        }

        if (this->pattern_bar->attachedAxes().length() == 0)
        {
            this->pattern_bar->attachAxis(ScaTool::main_plot->chart()->axisX());
            this->pattern_bar->attachAxis(ScaTool::main_plot->chart()->axisY());
        }
        if (this->window_bar->attachedAxes().length() == 0)
        {
            this->window_bar->attachAxis(ScaTool::main_plot->chart()->axisX());
            this->window_bar->attachAxis(ScaTool::main_plot->chart()->axisY());
        }
    }

    this->window_bar->show();
    this->pattern_bar->show();
}

void SynchroDialog::closed()
{
    this->window_bar->hide();
    this->pattern_bar->hide();
//    e->accept();
}
