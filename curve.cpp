#include "curve.h"
#include "scatool.h"
#include <assert.h>
#include <QLineSeries>

Curve::Curve(int id) :
    QObject()
{
    this->fn = "";
    this->displayed = false;
    this->idx = id;
    this->fullseries = 0;
    this->displayseries = 0;
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
    if (this->displayseries)
    {
        return this->displayseries->color();
    }
    else
    {
        QColor *gray = new QColor(Qt::gray);
        return *gray;
    }
}

void Curve::resetFullSeries()
{
    delete this->fullseries;
    this->fullseries = 0;
}

void Curve::resetDisplaySeries()
{
    delete this->displayseries;
    this->displayseries = 0;
}

QtCharts::QLineSeries* Curve::getFullSeries()
{
    if (this->fullseries)
    {
        return this->fullseries;
    }
    else
    {
        // get size
        FILE *file = fopen(fn.toLatin1().data(),"rb");
        assert(file);

        std::fseek(file,0,SEEK_END);
        int nbpoints = ftell(file)>>2;
        std::fseek(file,0,0);

        float *data;
        // allocate data memory
        data = (float *)malloc(nbpoints<<2);
        // read data
        std::fread(data,sizeof(float),nbpoints,file);

        fclose(file);

        QtCharts::QLineSeries* fullseries = new QtCharts::QLineSeries();

        for (int i = 0; i < nbpoints; i ++)
        {
            fullseries->append(i,data[i]);
        }


        free(data);

        // Customize fullseries
        fullseries->setUseOpenGL(true);
        fullseries->setPointsVisible();

        // handler
        QObject::connect(fullseries,&QLineSeries::clicked,this,&Curve::curve_clicked);
        QObject::connect(fullseries,&QLineSeries::hovered,this,&Curve::curve_clicked);
        this->fullseries = fullseries;
        return fullseries;
    }
}

void Curve::updateDisplaySeries(int width, float zoomfactor, int xmin, int xmax)
{
    // get size
    FILE *file = fopen(fn.toLatin1().data(),"rb");
    assert(file);

    std::fseek(file,0,SEEK_END);
    int nbpoints = ftell(file)>>2;
    std::fseek(file,0,0);

    float *data;
    // allocate data memory
    data = (float *)malloc(nbpoints<<2);
    // read data
    std::fread(data,sizeof(float),nbpoints,file);
    fclose(file);


    int factor = (nbpoints/width)/zoomfactor;
    if (factor < 1)
        factor = 1;

    this->displayseries->hide();
    this->displayseries->clear();

    int absmin = std::max(0,xmin-10);
    int absmax = std::min(nbpoints,xmax+10);

    float dmin = data[absmin];
    float dmax = data[absmin];
    for (int i = absmin; i < absmax; i ++)
    {
        if (factor > 1)
        {
            if (i%factor == 0)
            {
                this->displayseries->append(i,dmin);
                this->displayseries->append(i,dmax);
                dmin = std::numeric_limits<float>::max();
                dmax = std::numeric_limits<float>::min();
                continue;
            }
            dmin = std::min(data[i],dmin);
            dmax = std::max(data[i],dmax);
        }
        else
        {
            this->displayseries->append(i,data[i]);
        }

    }
    this->displayseries->show();

    free(data);
}

QtCharts::QLineSeries* Curve::getDisplaySeries()
{
    if (!this->displayseries)
    {
        this->displayseries = new QtCharts::QLineSeries();
        int width = ScaTool::main_plot->chart()->windowFrameRect().width();

        updateDisplaySeries(width,1,0,this->length());

        // Customize displayseries
        this->displayseries->setUseOpenGL(true);
        this->displayseries->setPointsVisible();

        // handler
        QObject::connect(this->displayseries,&QLineSeries::clicked,this,&Curve::curve_clicked);
        QObject::connect(this->displayseries,&QLineSeries::hovered,this,&Curve::curve_clicked);
    }
    return this->displayseries;
}

int Curve::length()
{
    // get size
    FILE *file = fopen(fn.toLatin1().data(),"rb");
    assert(file);

    std::fseek(file,0,SEEK_END);
    int nbpoints = ftell(file)>>2;
    fclose(file);

    return nbpoints;
}

void Curve::curve_clicked(QPointF pt)
{
    ScaTool::statusbar->showMessage("Curve coordinate: (" + QString::number(pt.x()) + ", " + QString::number(pt.y()) + ")",0);
}

void Curve::shift(int offset)
{
    this->xoffset += offset;

    QLineSeries * cseries = getFullSeries();

    QList<QPointF> *ptlist = new QList<QPointF>;

    for (int i = 0 ; i < cseries->count(); i ++)
    {
        QPointF pt = cseries->at(i);
        pt.rx() += offset;
        ptlist->append(pt);
    }
    cseries->replace(*ptlist);
}

