#include "attackwindow.h"
#include "ui_attackwindow.h"
#include "scatool.h"
#include <assert.h>
#include <QFileDialog>
#include <QSpinBox>
#include <QTableWidget>
#include <QByteArray>
#include <qdebug.h>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QProcess>

Attackwindow::Attackwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Attackwindow)
{
    ui->setupUi(this);

    int algo_idx = 0;
    algo_map["Message"] = algo_idx++;
    algo_map["AES"] = algo_idx++;
    algo_map["DES"] = algo_idx++;

    ui->algoBox->clear();
    ui->algoBox->addItems(QStringList()<<"Identity"<<"AES"<<"DES");

    ui->functionBox->clear();
    ui->functionBox->addItems(QStringList()<<"Input"<<"1st round output SBOX"<<"1st XOR");

    ui->methodBox->clear();
    ui->methodBox->addItems(QStringList() << "CPA");

    ui->byteIdxTable->resizeColumnsToContents();
    ui->byteIdxTable->resizeRowsToContents();

    ui->spinnb_traces->setMaximum(0);

}

Attackwindow::~Attackwindow()
{
    delete ui;
}

void Attackwindow::on_DataButton_pressed()
{
    // Open file dialog
    QString fname = QFileDialog::getOpenFileName(this,QString("Select data set file"));

    QStringList ql;

    qf.setFileName(fname);
    if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);

    int r = 0;
    while(!qf.atEnd())
    {
        QString qs = qf.readLine();
        QByteArray qba = QByteArray::fromHex(qs.toLatin1());
        ql.append(qs);
        ui->tableWidget->insertRow(r);
        ui->tableWidget->setItem(r,0,new QTableWidgetItem(ql.at(r)));
        r++;
    }

    ui->spinnb_traces->setMaximum(r);
    ui->spinnb_traces->setValue(r);

    qf.close();
}

void Attackwindow::on_spinpts_start_valueChanged(int arg1)
{
    //Update value range
    if (ScaTool::curves->length() > 0)
    {
        ui->spinpts_start->setRange(0,ScaTool::curves->first()->length());
        ui->spinpts_end->setRange(0,ScaTool::curves->first()->length());
    }

    // Respect min and max
    if (arg1 > ui->spinpts_end->value())
        ui->spinpts_end->setValue(ui->spinpts_start->value());
}

void Attackwindow::on_spinpts_end_valueChanged(int arg1)
{
    //Update value range
    if (ScaTool::curves->length() > 0)
    {
        ui->spinpts_start->setRange(0,ScaTool::curves->first()->length());
        ui->spinpts_end->setRange(0,ScaTool::curves->first()->length());
    }

    // Respect min and max
    if (ui->spinpts_start->value() > arg1)
        ui->spinpts_start->setValue(ui->spinpts_end->value());
}

void Attackwindow::on_ClearButton_pressed()
{
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->spinnb_traces->setMaximum(0);
    ui->spinnb_traces->setValue(0);
}

void Attackwindow::on_attackButton_pressed()
{

    // Get values from UI
    int attack_method = ui->methodBox->currentIndex();
    int algo = ui->algoBox->currentIndex();
    int sel_fun = ui->functionBox->currentIndex();
    int pts_min = ui->spinpts_start->value();
    int pts_max = ui->spinpts_end->value();
    int nb_pts = pts_max - pts_min;
    int nb_traces = ui->spinnb_traces->value();
    //ui->byteIdxTable->selectedItems()

    // Create a working directory
    QTemporaryDir tdir;
    assert(tdir.isValid());

    // Create traces data file
    QFile trace(tdir.path() + "/trace.bin");
    assert(trace.open(QIODevice::ReadWrite));
    for (int i = 0; i < nb_traces; i++)
    {
        Curve* c = ScaTool::curves->at(i);
        float * buf = c->getrawdata(&nb_pts,pts_min - c->xoffset);
        trace.write(reinterpret_cast<const char*>(buf), nb_pts<<2);
        // free buf
        free(buf);

    }
    // Create config file
    QFile config(tdir.path() + "/CONFIG");
    assert(config.open(QIODevice::ReadWrite | QIODevice::Text));
    config.write("[Traces]\n");
    config.write("files=1\n");
    config.write("trace_type=f\n");
    config.write("transpose=false\n");
    config.write("index=0\n");
    config.write("nsamples="+QString::number(nb_pts).toUtf8()+"\n");
    config.write("trace="+tdir.path().toUtf8() +"/trace.bin " +QString::number(nb_traces).toUtf8()+ " "+QString::number(nb_pts).toUtf8()+"\n");

    config.write("[Guesses]\n");
    config.write("files=1\n");
    config.write("guess_type=u\n");
    config.write("transpose=false\n");
    config.write("guess=" + qf.fileName().toUtf8() + " " + QString::number(nb_traces).toUtf8() + " 16\n");
    config.write("[General]\n");
    config.write("threads=8\n");
    config.write("order=1\n");
    config.write("return_type=double\n");
    config.write("algorithm=AES\n");
    config.write("position=LUT/AES_AFTER_SBOX\n");
    config.write("round=0\n");
    config.write("bytenum=all\n");
    config.write("bitnum=all\n");

    config.write("memory=4G\n");
    config.write("top=20\n");

    config.close();
    trace.close();


    // Launch daredevil
    QProcess daredevil;
    QString fname = QFileDialog::getOpenFileName(this,QString("Select daredevil binary file"));
    daredevil.start(fname + " -c " + config.fileName(),QIODevice::ReadOnly);
    daredevil.waitForFinished();
    // Delete files&dir
    config.flush();
    trace.flush();
    trace.remove();
    tdir.remove();

}
