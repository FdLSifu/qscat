#include "attacklog.h"
#include "ui_attacklog.h"
#include <QString>
#include <QHeaderView>
#include <QProcess>
#include <QDebug>

AttackLog::AttackLog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttackLog)
{
    ui->setupUi(this);
    this->is_complete = 0;
    this->next_byte = 0;
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
