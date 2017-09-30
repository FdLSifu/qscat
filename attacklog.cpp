#include "attacklog.h"
#include "ui_attacklog.h"
#include <QString>
#include <QHeaderView>
#include <QProcess>
#include <QDebug>
#include <QPushButton>
#include <assert.h>

AttackLog::AttackLog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttackLog)
{
    ui->setupUi(this);
    this->is_complete = 0;
    this->next_byte = 0;
    ui->log_label->setText("Attack in progress ...");

    connect(ui->pushButton_1, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_2, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_3, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_4, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_5, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_6, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_7, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_8, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_9, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_10, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_11, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_12, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_13, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_14, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_15, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
    connect(ui->pushButton_16, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
}

AttackLog::~AttackLog()
{
    delete ui;
}

int AttackLog::getSplitResult(QString txt, QString delim1, QString delim2,
                               QStringList *list1, QStringList *list2)
{
    QStringList chunklist = txt.split(delim1);

    if (chunklist.size() < 2)
        return -1;

    QString chunk = txt.split(delim1).at(1);
    QStringList chunklist2 = chunk.split(delim2);

    if (chunklist2.size() < 2)
        return -1;

    *list1 = chunk.split(delim2).at(0).split("\n");
    *list2 = chunk.split(delim2).at(1).split("\n");

    if (list1->size() < 8 || list2->size() < 8)
            return -1;

    return 0;
}

void AttackLog::fillSumMaxKey(QString txt)
{
    int status, row  = 0;
    QStringList sum_list, max_list;

    if (this->is_complete)
        return;

    status = getSplitResult(txt, "key sum(abs):", "key max(abs):", &sum_list, &max_list);
    if (status < 0)
        return;

    ui->list_keymax->clear();
    for (int i = 0; i < sum_list.length() && row < 5; i++) {
        QString str = sum_list.at(i);
        if (str.length() == 0)
            continue;
	QStringList filter = str.split(": ");
        ui->list_keysum->insertRow(row);
        ui->list_keysum->setItem(row, 0, new QTableWidgetItem(filter.at(2)));
        row++;
    }

    ui->list_keymax->clear();
    row = 0;
    for (int i = 0; i < max_list.length() && row < 5; i++) {
        QString str = max_list.at(i);
        if (str.length() == 0)
            continue;
	QStringList filter = str.split(": ");
        ui->list_keymax->insertRow(row);
        ui->list_keymax->setItem(row, 0, new QTableWidgetItem(filter.at(2)));
        row++;
    }

    ui->list_keysum->resizeColumnsToContents();
    ui->list_keysum->resizeRowsToContents();
    ui->list_keymax->resizeColumnsToContents();
    ui->list_keymax->resizeRowsToContents();
    ui->daredevil_log->clear();
    ui->daredevil_log->appendPlainText(txt);
    this->is_complete = 1;
}

void AttackLog::fillSumMaxCorr(QString txt)
{
    int row  = 0;
    QStringList sum_list, max_list;

    if (this->next_byte == 16)
        return;

    for (int byte = this->next_byte; byte < 16; byte++) {
        int status = getSplitResult(txt, "key byte #" + QString::number(byte) +
            " according to sum(abs(bit_correlations)):", "key byte #" +
              QString::number(byte) +
            " according to highest abs(bit_correlations):",
              &sum_list, &max_list);

        if (status < 0)
            continue;

        row = 0;
        for (int i = 0; i < sum_list.length() && row < 5; i++) {
            QString tmp = sum_list.at(i);
            QStringList corrl = tmp.split("sum:");
            if (corrl.at(0).length() == 0)
                continue;
            QString str = "   " + corrl.at(0).mid(4)
                        + "\n(" + corrl.at(1).mid(1) + ")";
            ui->list_corrsum->setItem(row, byte, new QTableWidgetItem(str));
            row++;
        }

        row = 0;
        for (int i = 0; i < max_list.length() && row < 5; i++) {
            QString tmp = max_list.at(i);
            QStringList corrl = tmp.split("peak:");
            if (corrl.at(0).length() == 0)
                continue;
            QString str = "   " + corrl.at(0).mid(4)
                        + "\n(" + corrl.at(1).mid(1) + ")";
            ui->list_corrmax->setItem(row, byte, new QTableWidgetItem(str));
            row++;
        }
        this->next_byte = byte+1;
    }
    ui->list_corrsum->resizeColumnsToContents();
    ui->list_corrsum->resizeRowsToContents();
    ui->list_corrmax->resizeColumnsToContents();
    ui->list_corrmax->resizeRowsToContents();
}

void AttackLog::updateLabelLog(QString txt)
{
    ui->log_label->setText(txt);
}

QList<QLineSeries*>* AttackLog::getCorrSerie(int byte_idx)
{
    int i_point = 0;
    int i_guess = 0;
    QList<QLineSeries*> *ql = new QList<QLineSeries*>();
    FILE *data;
    data = fopen("./correlation.bin","rb");

    int nb_guess = 256;

    fseek(data,0,SEEK_END);
    int nb_point = ftell(data)/(nb_guess*16*sizeof(double));
    fseek(data,0,SEEK_SET);

    i_guess = 0;

    double (*rawdata)[256][nb_point] = (double(*)[256][nb_point])malloc(16*256*nb_point*sizeof(double));
    fread(rawdata,16*256*nb_point,sizeof(double),data);

    while(i_guess < nb_guess)
    {
        QLineSeries * lineserie = new QLineSeries();
        lineserie->setUseOpenGL(true);

        i_point = 0;

        while(i_point < nb_point)
        {
            lineserie->append(i_point,rawdata[byte_idx][i_guess][i_point]);
            i_point ++;
        }
        i_guess ++;
        ql->append(lineserie);
    }
    free(rawdata);
    return ql;
}

void AttackLog::on_pushButton_pressed()
{
    int idx = 0;
    if (QObject::sender() == ui->pushButton_1)
        idx=0;
    else if (QObject::sender() == ui->pushButton_2)
        idx=1;
    else if (QObject::sender() == ui->pushButton_3)
        idx=2;
    else if (QObject::sender() == ui->pushButton_4)
        idx=3;
    else if (QObject::sender() == ui->pushButton_5)
        idx=4;
    else if (QObject::sender() == ui->pushButton_6)
        idx=5;
    else if (QObject::sender() == ui->pushButton_7)
        idx=6;
    else if (QObject::sender() == ui->pushButton_8)
        idx=7;
    else if (QObject::sender() == ui->pushButton_9)
        idx=8;
    else if (QObject::sender() == ui->pushButton_10)
        idx=9;
    else if (QObject::sender() == ui->pushButton_11)
        idx=10;
    else if (QObject::sender() == ui->pushButton_12)
        idx=11;
    else if (QObject::sender() == ui->pushButton_13)
        idx=12;
    else if (QObject::sender() == ui->pushButton_14)
        idx=13;
    else if (QObject::sender() == ui->pushButton_15)
        idx=14;
    else if (QObject::sender() == ui->pushButton_16)
        idx=15;
    else
        assert(false);

    QList<QLineSeries*> * ql = getCorrSerie(idx);
    if(!ui->corrchart->chart()->series().isEmpty())
    {
        ui->corrchart->chart()->removeAllSeries();
        ui->corrchart->chart()->removeAxis(ui->corrchart->chart()->axisX());
        ui->corrchart->chart()->removeAxis(ui->corrchart->chart()->axisY());
    }

    for (int i = 0; i < ql->length(); i++)
    {
        ui->corrchart->chart()->addSeries(ql->at(i));
    }
    ui->corrchart->chart()->createDefaultAxes();
}
