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

    ui->runpreview->setEnabled(false);
    ui->runsynchro->setEnabled(false);

    ui->threshold->setChart(new QChart());

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

void SynchroDialog::clearRefItem()
{
    ui->refcombo->clear();
}

void SynchroDialog::removeRefItem(QString name)
{
    int idx = ui->refcombo->findText(name);
    ui->refcombo->removeItem(idx);
}
SynchroDialog::~SynchroDialog()
{
    delete ui;
}

void SynchroDialog::on_runpreview_pressed()
{
    preview = true;
    return on_runsynchro_pressed();
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
        int passnum = ui->stepcombo->currentIndex();
        runningsynchro = synchropasses.at(passnum);

        runningsynchro->curves.clear();
        for (int i = 0; i < ScaTool::curves->length(); i++)
        {
            Curve * c = ScaTool::curves->at(i);
            // For preview add only displayed curves
            if (!this->preview | c->displayed)
                runningsynchro->curves.append(c);
        }

        if (!this->preview){
            // Trick to set length - 2 as the reference curve is done synchronized
            SynchroDialog::qprogressbar = new QProgressBar(this);
            SynchroDialog::qprogressbar->setMinimum(0);
            SynchroDialog::qprogressbar->setMaximum(runningsynchro->curves.length()-2);
            ScaTool::statusbar->addPermanentWidget(SynchroDialog::qprogressbar);
            SynchroDialog::qprogressbar->show();
        }

        // Get curve's offset
        runningsynchro->curve_offset.clear();
        for(int i = 0; i < runningsynchro->curves.length() ; i++)
        {
            Curve *c = runningsynchro->curves.at(i);
            // Apply previous pass if any
            if(passnum > 0)
                runningsynchro->curve_offset.append(runningsynchro->curves.at(i)->offsets.at(passnum-1));
            else
                runningsynchro->curve_offset.append(0);
        }

        runningsynchro->curve_ref_idx = 0;
        runningsynchro->leftwindow = ui->leftwindow->text().toInt();
        runningsynchro->rightwindow = ui->rightwindow->text().toInt();
        runningsynchro->leftpattern = ui->leftpattern->text().toInt();
        runningsynchro->rightpattern = ui->rightpattern->text().toInt();
        runningsynchro->precision = ui->precision->text().toInt();
        runningsynchro->preview = this->preview;
        runningsynchro->setAutoDelete(false);
        QThreadPool::globalInstance()->tryStart(runningsynchro);

        this->preview = false;
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

            QThreadPool::globalInstance()->releaseThread();

            QLineSeries *series = new QLineSeries();

            for(int i = 0; i < runningsynchro->result.length() ; i++)
                series->append(i,runningsynchro->result.at(i));

            if(!ui->threshold->chart()->series().isEmpty())
            {
                ui->threshold->chart()->removeAllSeries();
                ui->threshold->chart()->removeAxis(ui->threshold->chart()->axisX());
                ui->threshold->chart()->removeAxis(ui->threshold->chart()->axisY());
            }
            ui->threshold->chart()->addSeries(series);
            ui->threshold->chart()->createDefaultAxes();

        }
    }
}

void SynchroDialog::on_addstep_pressed()
{

    Synchro *sync = new Synchro(synchropasses.length());
    synchropasses.append(sync);

    ui->stepcombo->addItem("Pass "+QString::number(synchropasses.length()));
    ui->runpreview->setEnabled(true);
    ui->runsynchro->setEnabled(true);
}

void SynchroDialog::on_removestep_pressed()
{
    if (!synchropasses.isEmpty())
    {
        Synchro * sync = synchropasses.last();
        synchropasses.removeLast();
        ui->stepcombo->removeItem(synchropasses.length());
        delete sync;
    }
    if(synchropasses.isEmpty())
    {
        ui->runpreview->setEnabled(false);
        ui->runsynchro->setEnabled(false);
    }
}

void SynchroDialog::pattern_value_changed()
{
    qreal lw = ui->leftwindow->text().toDouble();
    qreal rw = ui->rightwindow->text().toDouble();
    qreal lp = ui->leftpattern->text().toDouble();
    qreal rp = ui->rightpattern->text().toDouble();

    // Get nbpoints
    int nbpoints = 0;
    for(int i = 0; i < ScaTool::curves->length(); i++)
    {
        if(nbpoints == 0)
            nbpoints = ScaTool::curves->at(i)->length();
        else
            nbpoints = std::min(nbpoints,ScaTool::curves->at(i)->length());
    }

    // Set range
    ui->leftpattern->setMinimum(0);
    ui->rightpattern->setMinimum(0);
    ui->leftpattern->setMaximum(nbpoints);
    ui->rightpattern->setMaximum(nbpoints);

    // Check range
    if(lp > rp)
    {
        rp = lp;
        ui->rightpattern->setValue(rp);
    }
    if (lp+lw < 0)
    {
        lw = -lp;
        ui->leftwindow->setValue(lw);
    }
    if (rp+rw > nbpoints)
    {
        rw = nbpoints - rp;
        ui->rightwindow->setValue(rw);
    }

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
}

void SynchroDialog::on_stepcombo_currentIndexChanged(int index)
{
    // We have selected a pass, let's update the graph
    for (int i = 0; i < ScaTool::curves->length(); i++)
    {
        if (ScaTool::curves->at(i)->displayed == true)
        {
            Curve *c = ScaTool::curves->at(i);
            if (c->offsets.length() > index)
                c->shift(c->offsets.at(index)-c->xoffset);
        }
    }

}
