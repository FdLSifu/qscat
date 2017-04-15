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
    QtCharts::QLineSeries* fullseries;
    QtCharts::QLineSeries* displayseries;
    int display_width = 0;
    int idx;
    int xoffset = 0;
    int yoffset = 0;
    int xsync = 0;
    ~Curve();
    Curve(int id);
    int length();
    void resetFullSeries();
    void resetDisplaySeries();
    QtCharts::QLineSeries* getFullSeries();
    QtCharts::QLineSeries* getDisplaySeries();
    void updateDisplaySeries(int width, float zoomfactor, int xmin, int xmax);
    void shift(int offset);
    QColor getColor();
public slots:
    void curve_clicked(QPointF pt);

};

#endif // CURVE_H
