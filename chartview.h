#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include "chart.h"
#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>

QT_CHARTS_USE_NAMESPACE

//![1]
class ChartView : public QChartView
//![1]
{
public:
    ChartView(QChart *chart, QWidget *parent = 0);
    ChartView(QWidget *parent = 0);
    Chart *chart();
//![2]
protected:
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
//![2]

private:
    bool m_isTouching;
};

#endif
