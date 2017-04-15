#ifndef SCATOOL_H
#define SCATOOL_H

#include <QListWidgetItem>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QColor>
#include <QStatusBar>
#include <QThreadPool>
#include <QDockWidget>
#include "curve.h"
#include "listwidget.h"
#include "chartview.h"
#include "synchrodialog.h"

class ScaTool
{
public:
    static QColor Colors[7];
    static QList<Curve*> *curves;
    static ChartView *main_plot;
    static QtCharts::QChartView *down_plot;
    static QDockWidget *dockcurves;
    static ListWidget *qlistwidget;
    static SynchroDialog *synchrodialog;
    static QStatusBar * statusbar;
    static void show_file_window();
    static void sync_sod(QList<Curve *> * lcurves, Curve *ref_curve, int lwin, int rwin, int lpattern, int rpattern, int precision);
    static Curve* getSelectedCurve();
    static Curve* getCurveFromDisplaySerie(QLineSeries * serie);
    static Curve * getCurveByName(QString name);
};

#endif // SCATOOL_H
