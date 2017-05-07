#include "scatool.h"
#include <QListWidget>
#include <QStringList>
#include <QObject>
#include <QtGlobal>
#include <algorithm>
#include <QThread>
#include "mainwindow.h"
#include "synchro.h"

QList<Curve*> * ScaTool::curves = 0;
QDockWidget *ScaTool::dockcurves = 0;
CurveListWidget *ScaTool::curve_table = 0;
SynchroDialog *ScaTool::synchrodialog = 0;
ChartView *ScaTool::main_plot = 0;
QtCharts::QChartView *ScaTool::down_plot = 0;
QStatusBar * ScaTool::statusbar = 0;

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
