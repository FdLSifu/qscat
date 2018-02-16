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
    this->idx = 0;
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
    connect(ui->pushButton_all, SIGNAL(pressed()), this, SLOT(on_pushButton_pressed()));
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

/*QList<QLineSeries*>* AttackLog::getCorrSerie(int byte_idx)
{
    double rawdata;
    int i_point = 0;
    int i_guess = 0;
    QList<QLineSeries*> *ql = new QList<QLineSeries*>();
    FILE *data;
    data = fopen("./correlation.bin","rb");

    int nb_guess = 256;
    int nb_bytes = 16;

    fseek(data,0,SEEK_END);
    // Compute points number
    int nb_point = ftell(data)/(nb_guess*nb_bytes*sizeof(double));
    fseek(data,0,SEEK_SET);

    i_guess = 0;
    // Loop over all guesses
    while(i_guess < nb_guess)
    {
        QLineSeries * lineserie = new QLineSeries();
        lineserie->setUseOpenGL(true);

        i_point = 0;
        // Compute offset manually
        // correlation binary file is built as a 3D array [byte][guess][point]
        int offset = byte_idx*nb_guess*nb_point;
        offset += i_guess*nb_point;
        offset *= sizeof(double);
        fseek(data,offset,SEEK_SET);
        while(i_point < nb_point)
        {
            fread(&rawdata,sizeof(double),1,data);
            lineserie->append(i_point++,rawdata);
        }
        i_guess ++;
        ql->append(lineserie);
    }
    return ql;
}*/

QList<QLineSeries*>* AttackLog::getCorrSerie(int byte_idx)
{
    QPointF pmin,pmax;
    double rawdata;
    int i_point = 0;
    int i_guess = 0;
    QList<QLineSeries*> *ql = new QList<QLineSeries*>();
    QLineSeries * lineserieminmax = new QLineSeries();
    lineserieminmax->setUseOpenGL(true);

    FILE *data;
    data = fopen("./correlation.bin","rb");

    // File not found
    if (data == 0)
        return ql;

    int nb_guess = 256;
    int nb_bytes = 16;

    fseek(data,0,SEEK_END);
    // Compute points number
    int nb_point = ftell(data)/(nb_guess*nb_bytes*sizeof(double));
    fseek(data,0,SEEK_SET);

    i_guess = 0;
    // Loop over all guesses
    while(i_guess < nb_guess)
    {
        i_point = 0;
        // Compute offset manually
        // correlation binary file is built as a 3D array [byte][guess][point]
        int offset = byte_idx*nb_guess*nb_point;
        offset += i_guess*nb_point;
        offset *= sizeof(double);
        fseek(data,offset,SEEK_SET);
        while(i_point < nb_point)
        {
            fread(&rawdata,sizeof(double),1,data);
            if (i_guess == 0)
            {
                lineserieminmax->append(i_point,rawdata);
                lineserieminmax->append(i_point,rawdata);
            }
            else
            {
                pmin = lineserieminmax->at(2*i_point);
                if (rawdata < pmin.y())
                    lineserieminmax->replace(2*i_point,i_point,rawdata);
                pmax = lineserieminmax->at((2*i_point)+1);
                if (rawdata > pmax.y())
                    lineserieminmax->replace((2*i_point)+1,i_point,rawdata);
            }
            i_point ++;
        }
        i_guess ++;
    }
    ql->append(lineserieminmax);
    return ql;
}

void AttackLog::on_pushButton_pressed()
{

    if (QObject::sender() == ui->pushButton_1)
        idx ^= 1UL << 0;
    else if (QObject::sender() == ui->pushButton_2)
        idx ^= 1UL << 1;
    else if (QObject::sender() == ui->pushButton_3)
        idx ^= 1UL << 2;
    else if (QObject::sender() == ui->pushButton_4)
        idx ^= 1UL << 3;
    else if (QObject::sender() == ui->pushButton_5)
        idx ^= 1UL << 4;
    else if (QObject::sender() == ui->pushButton_6)
        idx ^= 1UL << 5;
    else if (QObject::sender() == ui->pushButton_7)
        idx ^= 1UL << 6;
    else if (QObject::sender() == ui->pushButton_8)
        idx ^= 1UL << 7;
    else if (QObject::sender() == ui->pushButton_9)
        idx ^= 1UL << 8;
    else if (QObject::sender() == ui->pushButton_10)
        idx ^= 1UL << 9;
    else if (QObject::sender() == ui->pushButton_11)
        idx ^= 1UL << 10;
    else if (QObject::sender() == ui->pushButton_12)
        idx ^= 1UL << 11;
    else if (QObject::sender() == ui->pushButton_13)
        idx ^= 1UL << 12;
    else if (QObject::sender() == ui->pushButton_14)
        idx ^= 1UL << 13;
    else if (QObject::sender() == ui->pushButton_15)
        idx ^= 1UL << 14;
    else if (QObject::sender() == ui->pushButton_16)
        idx ^= 1UL << 15;
    else if (QObject::sender() == ui->pushButton_all)
        idx = 0xFFFF;
    else
        assert(false);

    if(!ui->corrchart->chart()->series().isEmpty())
    {
        ui->corrchart->chart()->removeAllSeries();
        ui->corrchart->chart()->removeAxis(ui->corrchart->chart()->axisX());
        ui->corrchart->chart()->removeAxis(ui->corrchart->chart()->axisY());
    }

    for (int i = 0; i < 16; i ++)
    {
        if ((idx >> i)&1)
        {
            QList<QLineSeries*> * ql = getCorrSerie(i);
            for (int i = 0; i < ql->length(); i++)
            {
                ui->corrchart->chart()->addSeries(ql->at(i));
            }
        }
        QCoreApplication::processEvents();
    }
    ui->corrchart->chart()->createDefaultAxes();

}
