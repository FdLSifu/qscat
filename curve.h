#ifndef CURVE_H
#define CURVE_H

#include <QString>
#include <QColor>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>

class Curve : public QObject
{
public:

    // Fields
    QString fn;
    QString cname;
    bool displayed;
    QtCharts::QLineSeries* fullseries;
    QtCharts::QLineSeries* displayseries;
    int display_width = 0;
    int idx;
    int xoffset = 0;
    int yoffset = 0;

    // Constructor
    Curve(int id);

    // Destructor
    ~Curve();

    // Function
    QtCharts::QLineSeries* getFullSeries();
    QtCharts::QLineSeries* getDisplaySeries();

    QList<QPointF> downsample_minmax(float *data,int factor, int absmin, int absmax);

    QColor getColor();

    int length();

    void resetFullSeries();
    void resetDisplaySeries();
    void updateDisplaySeries(int width, float zoomfactor, int xmin, int xmax);
    void shift(int offset);

public slots:
    void curve_clicked(QPointF pt);

};

#endif // CURVE_H
