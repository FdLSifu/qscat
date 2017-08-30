#include "attacklog.h"
#include "ui_attacklog.h"
#include <QString>
#include <QHeaderView>
#include <QDebug>

AttackLog::AttackLog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttackLog)
{
    ui->setupUi(this);
}

AttackLog::~AttackLog()
{
    delete ui;
}

void AttackLog::getSplitResult(QString txt, QString delim1, QString delim2,
                               QStringList *list1, QStringList *list2)
{
    QString chunk = txt.split(delim1).at(1);
    *list1 = chunk.split(delim2).at(0).split("\n");
    *list2 = chunk.split(delim2).at(1).split("\n");
}

void AttackLog::fillSumMaxKey(QString txt)
{
    int row  = 0;
    QStringList sum_list, max_list;

    getSplitResult(txt, "key sum(abs):", "key max(abs):", &sum_list, &max_list);

    ui->list_keysum->clear();
    for (int i = 0; i < sum_list.length() && row < 5; i++) {
        QString str = sum_list.at(i);
        if (str.length() == 0)
            continue;
        ui->list_keysum->insertRow(row);
        ui->list_keysum->setItem(row, 0, new QTableWidgetItem(str.mid(12)));
        row++;
    }

    ui->list_keymax->clear();
    row = 0;
    for (int i = 0; i < max_list.length() && row < 5; i++) {
        QString str = max_list.at(i);
        if (str.length() == 0)
            continue;
        ui->list_keymax->insertRow(row);
        ui->list_keymax->setItem(row, 0, new QTableWidgetItem(str.mid(12)));
        row++;
    }

    ui->list_keysum->resizeColumnsToContents();
    ui->list_keysum->resizeRowsToContents();
    ui->list_keymax->resizeColumnsToContents();
    ui->list_keymax->resizeRowsToContents();
}

void AttackLog::fillSumMaxCorr(QString txt)
{
    int row  = 0;
    QStringList sum_list, max_list;
    ui->list_corrsum->clear();
    ui->list_corrmax->clear();

    for (int byte = 0; byte < 16; byte++) {
        getSplitResult(txt, "key byte #" + QString::number(byte) +
            " according to sum(abs(bit_correlations)):", "key byte #" +
              QString::number(byte) +
            " according to highest abs(bit_correlations):",
              &sum_list, &max_list);

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
    }
    ui->list_corrsum->resizeColumnsToContents();
    ui->list_corrsum->resizeRowsToContents();
    ui->list_corrmax->resizeColumnsToContents();
    ui->list_corrmax->resizeRowsToContents();
}


void AttackLog::showlog(QString stdout)
{
    this->show();
    ui->daredevil_log->clear();
    ui->daredevil_log->appendPlainText(stdout);
    fillSumMaxKey(stdout);
    fillSumMaxCorr(stdout);
}
