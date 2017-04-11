#ifndef SYNCHRODIALOG_H
#define SYNCHRODIALOG_H

#include <QDialog>
#include <QRubberBand>
#include "synchro.h"

namespace Ui {
class SynchroDialog;
}

class SynchroDialog : public QDialog
{
    Q_OBJECT

public:
    QRubberBand *rubpattern;
    QRubberBand *rubsearch;
    QList<Synchro *> *synchropasses;
    explicit SynchroDialog(QWidget *parent = 0);
    void addRefItem(QString name);
    ~SynchroDialog();

public slots:
    void on_leftpattern_valueChanged(int arg1);

    void on_runpreview_pressed();
    
    void on_runsynchro_pressed();
    
private:
    Ui::SynchroDialog *ui;
};

#endif // SYNCHRODIALOG_H
