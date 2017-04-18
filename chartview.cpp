#include "chartview.h"
#include "scatool.h"
#include <QtGui/QMouseEvent>
#include <QValueAxis>

ChartView::ChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent),
    m_isTouching(false)
{
    setRubberBand(QChartView::HorizontalRubberBand);
}

ChartView::ChartView(QWidget *parent) :
    QChartView(parent),
    m_isTouching(false)
{
    setMouseTracking(true);
    setInteractive(true);
    setRubberBand(QChartView::HorizontalRubberBand);
}

Chart *ChartView::chart()
{
    return static_cast<Chart *>(QChartView::chart());
}

bool ChartView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin) {
        // By default touch events are converted to mouse events. So
        // after this event we will get a mouse event also but we want
        // to handle touch events as gestures only. So we need this safeguard
        // to block mouse events that are actually generated from touch.
        m_isTouching = true;

        // Turn off animations when handling gestures they
        // will only slow us down.
        //chart()->setAnimationOptions(QChart::NoAnimation);
    }

    return QGraphicsView::viewportEvent(event);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (m_isTouching)
        return;
    QChartView::mousePressEvent(event);
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    if (chart()->axes().length() > 0)
    {
        qreal min = qobject_cast<QValueAxis *>(chart()->axisY())->min();
        qreal max = qobject_cast<QValueAxis *>(chart()->axisY())->max();

        qreal numdegree = event->delta()/8;
        qreal numSteps = ((max-min)/50);

        // Wheel forward
        if (numdegree > 0)
        {
            chart()->axisY()->setRange(QVariant(min+numSteps),QVariant(max-numSteps));
        }
        // Wheel backward
        else if (numdegree < 0)
        {
            chart()->axisY()->setRange(QVariant(min-numSteps),QVariant(max+numSteps));
        }
    }

}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isTouching)
        return;
    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isTouching)
        m_isTouching = false;

    // Because we disabled animations when touch event was detected
    // we must put them back on.

    QChartView::mouseReleaseEvent(event);
}


void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QChartView::keyPressEvent(event);
        break;
    }
}
