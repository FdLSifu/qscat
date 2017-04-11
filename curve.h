#ifndef CURVE_H
#define CURVE_H

#include <QString>
#include <QColor>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>

class Curve : public QObject
{
public:
    QString fn;
    bool displayed;
    QtCharts::QLineSeries* series;
    int idx;
    int xoffset = 0;
    int yoffset = 0;
    int xsync = 0;

    Curve(int id);
    QtCharts::QLineSeries* getSeries();
    void shift(int offset);
    QColor getColor();
public slots:
    void curve_clicked(QPointF pt);
    void curve_hovered(QPointF pt, bool b);

};

#endif // CURVE_H
