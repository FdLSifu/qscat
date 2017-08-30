#include "attacklog.h"
#include "ui_attacklog.h"
#include <QString>
#include <QHeaderView>

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

void AttackLog::showlog(QString stdout)
{
    this->show();
    ui->daredevil_log->clear();
    ui->daredevil_log->appendPlainText(stdout);

    QStringList key_list = stdout.split("Most probable key sum(abs):");
    QString key_str = (key_list.at(1));
    QStringList key_list_split = key_str.split("Most probable key max(abs):");
    QStringList sum_list = key_list_split.at(0).split("\n");
    QStringList max_list = key_list_split.at(1).split("\n");

    ui->list_keysum->clear();
    ui->list_keysum->setRowCount(0);

    int row = 0;
    for (int i =0; i < sum_list.length() && row < 5; i++) {
        QString str = sum_list.at(i);
        if (str.length() > 4) {
            ui->list_keysum->insertRow(row);
            ui->list_keysum->setItem(row, 0, new QTableWidgetItem(str.remove(0,12)));
            row++;
        }
    }

    ui->list_keymax->clear();
    ui->list_keymax->setRowCount(0);

    row = 0;
    for (int i =0; i < max_list.length() && row < 5; i++) {
        QString str = max_list.at(i);
        if (str.length() > 4) {
            ui->list_keymax->insertRow(row);
            ui->list_keymax->setItem(row, 0, new QTableWidgetItem(str.remove(0,12)));
            row++;
        }
    }

    ui->list_keysum->resizeColumnsToContents();
    ui->list_keysum->resizeRowsToContents();
    ui->list_keymax->resizeColumnsToContents();
    ui->list_keymax->resizeRowsToContents();
}
