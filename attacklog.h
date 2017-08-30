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
    Ui::AttackLog *ui;
};

#endif // ATTACKLOG_H
