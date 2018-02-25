#ifndef ATTACKWINDOW_H
#define ATTACKWINDOW_H

#include <QDialog>
#include <QMap>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>
#include <QTime>
#include "corrtablemodel.h"

namespace Ui {
class Attackwindow;
}

class Attackwindow : public QWidget
{
    Q_OBJECT

    CPA * cpa;
public:
    explicit Attackwindow(QWidget *parent = 0);
    ~Attackwindow();
    void setTraceNb(int t);
    void setPtsNb(int p);

private slots:
    void on_spinpts_start_editingFinished();
    void on_spinpts_end_editingFinished();
    void on_attackButton_pressed();

    void on_showattack_pressed();

private:
    Ui::Attackwindow *ui;
    QMap<QString, int> algo_map;
    QString stdout_log;
    QTime processTime;

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
};

#endif // ATTACKWINDOW_H
