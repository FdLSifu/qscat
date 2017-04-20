#include "curve.h"
#include "scatool.h"
#include <assert.h>
#include <QLineSeries>
#include <QValueAxis>
#include <QtGlobal>

Curve::Curve(int id) :
    QObject()
{
    this->fn = "";
    this->cname = "";
    this->type = "float32";
    this->displayed = false;
    this->idx = id;
    this->fullseries = 0;
    this->displayseries = 0;
    this->color = QColor(Qt::white);
}

Curve::~Curve()
{
    if (this->fullseries != 0)
        resetFullSeries();
    if (this->displayseries != 0)
        resetDisplaySeries();
}

QColor Curve::getColor()
{
    return this->color;
}

void Curve::setColor(QColor c)
{
    this->color = c;
    if (this->displayseries)
    {
        this->displayseries->setColor(c);
    }
}

void Curve::resetFullSeries()
{
    this->fullseries->clear();
    delete this->fullseries;
    this->fullseries = 0;
}

void Curve::resetDisplaySeries()
{
    this->displayseries->clear();
    delete this->displayseries;
    this->displayseries = 0;
}

QtCharts::QLineSeries* Curve::getSubSeries(int xmin, int xmax)
{
    float *data = 0;
    int nbpoints = 0;

    data = getrawdata(&nbpoints);
    assert(data);

    xmin = std::max(0,xmin);
    xmax = std::min(nbpoints,xmax);

    assert(xmax > xmin);

    QtCharts::QLineSeries* subseries = new QtCharts::QLineSeries();

    for (int i = xmin; i < xmax; i ++)
    {
        subseries->append(i,data[i]);
    }

    free(data);

    return subseries;
}

QtCharts::QLineSeries* Curve::getFullSeries()
{
    if (this->fullseries)
    {
        return this->fullseries;
    }
    else
    {
        float *data = 0;
        int nbpoints = 0;

        data = getrawdata(&nbpoints);

        assert(data);

        QtCharts::QLineSeries* fullseries = new QtCharts::QLineSeries();

        for (int i = 0; i < nbpoints; i ++)
        {
            fullseries->append(i,data[i]);
        }


        free(data);

        this->fullseries = fullseries;
        return fullseries;
    }
}

void Curve::updateDisplaySeries(int width, float zoomfactor)
{
    float *data = 0;
    int nbpoints = 0;

    if (this->displayseries == 0)
        return;

    data = getrawdata(&nbpoints);

    assert(data);

    int factor = (nbpoints/width)/zoomfactor;
    // factor less than 3 no need to downsample
    if (factor < 3)
        factor = 1;

    this->displayseries->clear();

    QList<QPointF> points_list = downsample_minmax(data,factor,nbpoints);

    this->displayseries->replace(points_list);

    free(data);
}

void Curve::updateDisplaySeries()
{
    qreal xmin,xmax;

    int width = ScaTool::main_plot->chart()->windowFrameRect().width();

    if (ScaTool::main_plot->chart()->axes().length() > 0)
    {
        xmax = qobject_cast<QValueAxis *>(ScaTool::main_plot->chart()->axisX())->max();
        xmin = qobject_cast<QValueAxis *>(ScaTool::main_plot->chart()->axisX())->min();
    }
    else
    {
        xmin = 0;
        xmax = this->length();
    }

    float zoomfactor = ScaTool::main_plot->chart()->xaxis_width/(xmax-xmin);

    updateDisplaySeries(width,zoomfactor);
}

QList<QPointF> Curve::downsample_minmax(float *data,int factor, int nbpoints)
{
    QList<QPointF> points_list;
    float dmin = std::numeric_limits<float>::max();
    float dmax = std::numeric_limits<float>::min();
    for (int i = 0; i < nbpoints ; i ++)
    {
        if (factor > 1)
        {
            if (i%factor == 0)
            {
                dmin = std::min(data[i],dmin);
                dmax = std::max(data[i],dmax);
                points_list.append(QPointF(i+xoffset,dmin));
                points_list.append(QPointF(i+xoffset,dmax));

                dmin = std::numeric_limits<float>::max();
                dmax = std::numeric_limits<float>::min();
                continue;
            }
            dmin = std::min(data[i],dmin);
            dmax = std::max(data[i],dmax);
        }
        else
        {
            points_list.append(QPointF(i+xoffset,data[i]));
        }

    }
    return points_list;
}
QtCharts::QLineSeries* Curve::getDisplaySeries()
{
    // Create displayseries if not yet done
    if (!this->displayseries)
    {
        this->displayseries = new QtCharts::QLineSeries();


        this->updateDisplaySeries();

        // Customize displayseries
        this->displayseries->setUseOpenGL(true);
        // Check if color has been customized
        if (this->color != Qt::white)
            // If curve has a custom color apply it
            this->displayseries->setColor(this->color);

        // handler of displayed curve
        QObject::connect(this->displayseries,&QLineSeries::pressed,this,&Curve::curve_clicked);
        QObject::connect(this->displayseries,&QLineSeries::hovered,this,&Curve::curve_clicked);
    }
    return this->displayseries;
}

bool Curve::isLoaded()
{
    // Check if curve is loaded
    if (!this->displayseries)
        return false;
    else
        return true;
}

int Curve::length()
{
    int nbpoints;
    // get size
    FILE *file = fopen(fn.toLatin1().data(),"rb");
    assert(file);

    std::fseek(file,0,SEEK_END);

    int shift = 2;

    nbpoints = ftell(file)>>shift;
    fclose(file);

    return nbpoints;
}

// data must be freed by the caller
float * Curve::getrawdata(int *length)
{
    float *data;
    FILE *file = fopen(fn.toLatin1().data(),"rb");
    assert(file);

    std::fseek(file,0,SEEK_END);

    *length  = ftell(file)>>2;
    std::fseek(file,0,0);

    // allocate data memory
    data = (float *)malloc((*length)*sizeof(float));
    // read full data // Can be optimize to read only display data
    std::fread(data,sizeof(float),*length,file);
    fclose(file);

    return data;
}

void Curve::curve_clicked(QPointF pt)
{
    ScaTool::statusbar->showMessage("Curve coordinate: (" + QString::number(pt.x()) + ", " + QString::number(pt.y()) + ")",0);
}

void Curve::shift(int offset)
{
    this->xoffset += offset;

    if (this->displayed)
    {
        updateDisplaySeries();
    }
}

