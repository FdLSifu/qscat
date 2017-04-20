#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>

QT_BEGIN_NAMESPACE
class QGestureEvent;
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE


class Chart : public QChart
{
public:
    int xaxis_width = 0;
    float factor = 1;
    //explicit Chart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    Chart();
    ~Chart();

protected:
    bool sceneEvent(QEvent *event);

private:
    bool gestureEvent(QGestureEvent *event);
public slots:
    void on_rangeChanged(qreal,qreal);

};

#endif // CHART_H
