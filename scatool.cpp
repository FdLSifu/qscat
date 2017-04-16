#include "scatool.h"
#include <QListWidget>
#include <QStringList>
#include <QObject>
#include <QtGlobal>
#include <algorithm>
#include <QThread>
#include "mainwindow.h"
#include "listwidget.h"
#include "synchro.h"

QColor ScaTool::Colors[7]  = {Qt::red,Qt::blue, Qt::green, Qt::black, Qt::darkRed, Qt::darkBlue, Qt::darkGreen};

QList<Curve*> * ScaTool::curves = 0;
QDockWidget *ScaTool::dockcurves = 0;
ListWidget *ScaTool::qlistwidget = 0;
SynchroDialog *ScaTool::synchrodialog = 0;
ChartView *ScaTool::main_plot = 0;
QtCharts::QChartView *ScaTool::down_plot = 0;
QStatusBar * ScaTool::statusbar = 0;

void ScaTool::show_file_window()
{

    for (QList<Curve*>::iterator it = ScaTool::curves->begin();
         it != ScaTool::curves->end(); ++it) {
        Curve *curve = *it;

        QListWidgetItem *item = qlistwidget->getItemByName(curve->cname);
        if (curve->displayed)
            item->setTextColor(curve->displayseries->color());
        else
            item->setTextColor(Qt::gray);
    }

    if (!ScaTool::qlistwidget->isVisible())
        ScaTool::qlistwidget->show();
}

Curve * ScaTool::getSelectedCurve()
{

    QList<QListWidgetItem *> itemlist = ScaTool::qlistwidget->selectedItems();

    if ((!ScaTool::qlistwidget->isVisible()) or (itemlist.length() != 1))
        return 0;
    else
        return getCurveByName(itemlist.first()->text());

}

Curve * ScaTool::getCurveByName(QString name)
{

    for (QList<Curve*>::iterator it = ScaTool::curves->begin();
         it != ScaTool::curves->end(); ++it) {

        Curve *curve = *it;

        if (curve->cname == name)
        {
            return curve;

        }
    }
    return 0;
}
Curve* ScaTool::getCurveFromDisplaySerie(QLineSeries * serie)
{
    for (QList<Curve*>::iterator it = ScaTool::curves->begin();
         it != ScaTool::curves->end(); ++it) {

        Curve *curve = *it;

        if (curve->displayseries == serie)
        {
            return curve;

        }
    }
    return 0;
}

void ScaTool::sync_sod(QList<Curve *> * lcurves, Curve *ref_curve, int lwin, int rwin, int lpattern, int rpattern, int precision)
{
    int idx_ref = lcurves->indexOf(ref_curve);

    for(int i = 0; i < lcurves->length(); i++)
    {
        Curve *c = lcurves->at(i);

        if (idx_ref != i)
        {
            Synchro *sync = new Synchro(0);
            sync->curve = c;
            sync->cur_ref = ref_curve;
            sync->leftwindow = lwin;
            sync->rightwindow = rwin;
            sync->leftpattern = lpattern;
            sync->rightpattern = rpattern;
            sync->precision = precision;

            QThreadPool::globalInstance()->start(sync);
        }
    }
}
