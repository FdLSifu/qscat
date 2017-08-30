#ifndef ATTACKLOG_H
#define ATTACKLOG_H

#include <QDialog>

namespace Ui {
class AttackLog;
}

class AttackLog : public QDialog
{
    Q_OBJECT

public:
    explicit AttackLog(QWidget *parent = 0);
    ~AttackLog();
    void showlog(QString stdout);

private:
    void fillSumMaxKey(QString txt);
    void getSplitResult(QString txt, QString delim1, QString delim2,
                        QStringList *list1, QStringList *list2);
    void fillSumMaxCorr(QString txt);
    Ui::AttackLog *ui;
};

#endif // ATTACKLOG_H
