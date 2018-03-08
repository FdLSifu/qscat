#include "chart.h"
#include "scatool.h"
#include <QtWidgets/QGesture>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtCharts/QAbstractAxis>
Chart::Chart()
  : QChart(QChart::ChartTypeCartesian, 0, 0)
{
    // Seems that QGraphicsView (QChartView) does not grab gestures.
    // They can only be grabbed here in the QGraphicsWidget (QChart).
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
    legend()->hide();
    setMargins(QMargins(0,0,0,0));
    setAcceptHoverEvents(true);

}

Chart::~Chart()
{

}


bool Chart::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent *>(event));
    return QChart::event(event);
}

bool Chart::gestureEvent(QGestureEvent *event)
{
    if (QGesture *gesture = event->gesture(Qt::PanGesture)) {
        QPanGesture *pan = static_cast<QPanGesture *>(gesture);
        QChart::scroll(-(pan->delta().x()), pan->delta().y());
    }

    if (QGesture *gesture = event->gesture(Qt::PinchGesture)) {
        QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);
        if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged)
            QChart::zoom(pinch->scaleFactor());
    }
    return true;
}

void Chart::on_rangeChanged(qreal xmin,qreal xmax)
{

    float factor = xaxis_width/(xmax - xmin);
    // On scroll no update
    if (this->factor == factor)
        return;

    this->factor = factor;

    for (int i = 0; i < ScaTool::curves->length(); i++)
    {
        Curve *c = ScaTool::curves->at(i);
        if (c->displayed)
            c->updateDisplaySeries(this->windowFrameRect().width(),factor);
    }
}

