#ifndef SYNCHRODIALOG_H
#define SYNCHRODIALOG_H

#include <QDialog>
#include <QRubberBand>
#include <QProgressBar>
#include <QAreaSeries>
#include "synchro.h"

namespace Ui {
class SynchroDialog;
}

class SynchroDialog : public QDialog
{
    Q_OBJECT

public:
    // Fields
    static QLineSeries *dn;
    static QProgressBar* qprogressbar;

    bool flag = true;
    QRubberBand *rubpattern;
    QRubberBand *rubsearch;
    QList<Synchro *> synchropasses;

    // Constructor
    explicit SynchroDialog(QWidget *parent);
    ~SynchroDialog();

    // Functions
    void addRefItem(QString name);

public slots:
    void on_runpreview_pressed();
    
    void on_runsynchro_pressed();

    void update_progressdialog();
    
private slots:
    void on_addstep_pressed();
    void pattern_value_changed();
    void window_value_changed();

private:
    Ui::SynchroDialog *ui;
};

#endif // SYNCHRODIALOG_H
