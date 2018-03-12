#ifndef CURVE_H
#define CURVE_H

#include "chartview.h"
#include <QString>
#include <QColor>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QColor>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>

class Curve : public QObject
{
    Q_OBJECT
    enum CurveType
    {
        FLOAT32 =0,
        UINT32  =1,
        INT32   =2,
        UINT16  =3,
        INT16   =4,
        UINT8   =5,
        INT8    =6,
        DOUBLE  =7
    };

public:

    // Fields
    QString fn;
    QString cname;
    int type;
    QColor color;
    QCheckBox* chkbox = 0;
    QComboBox* type_cmbbox = 0;
    bool displayed;
    QtCharts::QLineSeries* fullseries;
    QtCharts::QLineSeries* displayseries;
    int display_width = 0;
    int idx;
    int xoffset = 0;
    QList<int> offsets;
    bool onefile = false;
    int row = 0;
    int ncol = 0;
    int length;
    float min;
    float max;
    QString textin;
    uint8_t *input;
    // Constructor
    Curve(int id,QString filename, int ncol, int row, bool onefile);

    // Destructor
    ~Curve();

    // Function
    QtCharts::QLineSeries*  getFullSeries();
    QtCharts::QLineSeries*  getDisplaySeries();
    float *getSubSeries(int xmin, int xmax);
    QList<QPointF> downsample_minmax(float *data, int factor, int nbpoints);

    QColor getColor();

    bool isLoaded();

    void setColor(QColor c);
    void setType(int type);
    int getLength();

    void resetFullSeries();
    void resetDisplaySeries();
    void updateDisplaySeries(int width, float zoomfactor);
    void updateDisplaySeries();
    void shift(int offset);

    float *getrawdata(int *length, int single_offset = 0);
    float get_floatvalueat(int time);

private:
    int _length();
public slots:
    void curve_clicked(QPointF pt);
    void chkbox_toggled(bool state);

};

#endif // CURVE_H
