#ifndef SCATOOL_H
#define SCATOOL_H

#include "chart.h"
#include "chartview.h"
#include <QListWidgetItem>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QColor>
#include <QStatusBar>
#include <QThreadPool>
#include <QDockWidget>
#include "curve.h"
#include "chartview.h"
#include "synchrodialog.h"
#include "curvelistwidget.h"
#include "attackwindow.h"
#include "curvetablemodel.h"

class ScaTool
{
public:
    static QVector<Curve*> *curves;
    static ChartView *main_plot;
    static QtCharts::QChartView *down_plot;
    static QDockWidget *dockcurves;
    static CurveListWidget *curve_table;
    static CurveTableModel * curve_table_model;
    static SynchroDialog *synchrodialog;
    static Attackwindow *attackdialog;
    static QStatusBar * statusbar;
    static int global_type;

    static void sync_sod(QVector<Curve *> * lcurves, Curve *ref_curve, int lwin, int rwin, int lpattern, int rpattern, int precision);
    static Curve* getCurveFromDisplaySerie(QLineSeries * serie);
};

#endif // SCATOOL_H
