#include "curve.h"
#include "scatool.h"
#include <assert.h>
#include <QLineSeries>
#include <QValueAxis>
#include <QtGlobal>
#include <QColorDialog>
#include <stdint.h>

Curve::Curve(int id) :
    QObject()
{
    this->fn = "";
    this->cname = "";
    this->type = Curve::CurveType(FLOAT32);
    this->displayed = false;
    this->idx = id;
    this->fullseries = 0;
    this->displayseries = 0;
    this->color = QColor(Qt::white);
}

Curve::~Curve()
{
    // Disable curve if displayed
    if (displayed)
    {
        chkbox->setChecked(false);
        ScaTool::main_plot->chart()->removeSeries(getDisplaySeries());
    }
    // Remove from table list
    ScaTool::curve_table->removeRow(this);
    // Remove from synchro list
    ScaTool::synchrodialog->removeRefItem(cname);
    // Remove from internal list
    Q_ASSERT(ScaTool::curves->removeOne(this));

    // Delete fullseries
    if (this->fullseries != 0)
        resetFullSeries();
    // Delete display series
    if (this->displayseries != 0)
        resetDisplaySeries();

    // Delete related objects
    if (this->chkbox != 0)
        delete this->chkbox;
    if (this->color_btn != 0)
        delete this->color_btn;
    if (this->type_cmbbox != 0)
        delete this->type_cmbbox;

    if (ScaTool::curves->isEmpty())
    {
        qDeleteAll(ScaTool::synchrodialog->synchropasses.begin(),ScaTool::synchrodialog->synchropasses.end());
        for (int i = 0 ; i < ScaTool::main_plot->chart()->axes(Qt::Horizontal).length(); i ++)
            ScaTool::main_plot->chart()->removeAxis(ScaTool::main_plot->chart()->axes().at(i));
        for (int i = 0 ; i < ScaTool::main_plot->chart()->axes(Qt::Vertical).length(); i ++)
            ScaTool::main_plot->chart()->removeAxis(ScaTool::main_plot->chart()->axes().at(i));
        ScaTool::curve_table->firstDisplayed = true;
        ScaTool::synchrodialog->clearRefItem();
    }
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

    assert(xmax >= xmin);

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
    // Curve local min max
    float lmin = std::numeric_limits<float>::max();
    float lmax = -std::numeric_limits<float>::max();
    // Curve absolute min max
    this->min = lmin;
    this->max = lmax;
    for (int i = 0; i < nbpoints ; i ++)
    {
        if (factor > 1)
        {
            if (i%factor == 0)
            {
                lmin = std::min(data[i],lmin);
                lmax = std::max(data[i],lmax);
                points_list.append(QPointF(i+xoffset,lmin));
                points_list.append(QPointF(i+xoffset,lmax));

                // Point has been added to be displayed
                // Keep track on max/min to have adequated range
                this->max = std::max(lmax,this->max);
                this->min = std::min(lmin,this->min);

                // Reset dmin dmax
                lmin = std::numeric_limits<float>::max();
                lmax = -std::numeric_limits<float>::max();
                continue;
            }
            lmin = std::min(data[i],lmin);
            lmax = std::max(data[i],lmax);
        }
        else
        {
            points_list.append(QPointF(i+xoffset,data[i]));
            // Point has been added to be displayed
            // Keep track on max/min to have adequated range
            this->max = std::max(data[i],this->max);
            this->min = std::min(data[i],this->min);
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
    int nbpoints,shift;

    switch (this->type) {
    case Curve::CurveType(DOUBLE):
        shift = 3;
        break;
    case Curve::CurveType(FLOAT32):
    case Curve::CurveType(UINT32):
    case Curve::CurveType(INT32):
        shift = 2;
        break;
    case Curve::CurveType(UINT16):
    case Curve::CurveType(INT16):
        shift = 1;
        break;
    case Curve::CurveType(UINT8):
    case Curve::CurveType(INT8):
        shift = 0;
        break;
    default:
        assert(0);
        break;
    }
    if (ncol == 0)
    {
        FILE *file = fopen(fn.toLatin1().data(),"rb");
        assert(file);

        std::fseek(file,0,SEEK_END);
        nbpoints = ftell(file)>>shift;
        fclose(file);
    }
    else
    {
        nbpoints = ncol>>shift;
    }
    return nbpoints;
}

// data must be freed by the caller
float * Curve::getrawdata(int *length, int single_offset)
{
    float *data = 0;
    uint32_t *bufferui32;
    int32_t *bufferi32;
    uint16_t *bufferui16;
    int16_t *bufferi16;
    uint8_t *bufferui8;
    int8_t *bufferi8;
    double *bufferd;
    bool is_file_open;
    int tr_off = row*ncol;
    int size = 0;

    QFile file(fn);
    is_file_open = file.open(QIODevice::ReadOnly);
    assert(is_file_open == true);

    if (ncol)
        size = ncol;
    else
        size = file.size();

    file.seek(tr_off);
    switch (this->type)
    {
        case Curve::CurveType(FLOAT32):
            *length  = size/sizeof(float);
            // allocate data memory
            data = (float *)malloc((*length)*sizeof(float));
            // read full data
            if (single_offset > 0)
            {

                for(int i = 0; i < single_offset; i++)
                    data[i] = 0;
                file.read(reinterpret_cast<char*>(data+single_offset), sizeof(float) * (*length - single_offset));
            }
            else
            {
                file.seek(tr_off + (sizeof(float) * -single_offset));
                file.read(reinterpret_cast<char*>(data),sizeof(float) * (*length + single_offset));
                for(int i = *length + single_offset; i < *length ; i++)
                    data[i] = 0;

            }

            break;
        case Curve::CurveType(UINT32):
            *length  = size/sizeof(uint32_t);
            data = (float *)malloc((*length)*sizeof(float));
            // read full data
            if (single_offset > 0)
            {

                for(int i = 0; i < single_offset; i++)
                    data[i] = 0;

                bufferui32 = (uint32_t*)malloc((*length - single_offset)*sizeof(uint32_t));
                file.read(reinterpret_cast<char*>(bufferui32),sizeof(uint32_t) * (*length - single_offset));
                for(int i = 0; i < (*length - single_offset); i++)
                    (data+single_offset)[i] = (float)(bufferui32[i]);
                free(bufferui32);
            }
            else
            {
                file.seek(tr_off + (sizeof(uint32_t) * -single_offset));

                bufferui32 = (uint32_t*)malloc((*length + single_offset)*sizeof(uint32_t));
                file.read(reinterpret_cast<char*>(bufferui32),sizeof(uint32_t) * (*length + single_offset));
                for(int i = 0; i < (*length + single_offset); i++)
                    (data)[i] = (float)(bufferui32[i]);
                free(bufferui32);

                for(int i = *length + single_offset; i < *length ; i++)
                    data[i] = 0;

            }
            break;
        case Curve::CurveType(INT32):
            *length  = size/sizeof(int32_t);
            data = (float *)malloc((*length)*sizeof(float));
            // read full data
            if (single_offset > 0)
            {

                for(int i = 0; i < single_offset; i++)
                    data[i] = 0;

                bufferi32 = (int32_t*)malloc((*length - single_offset)*sizeof(int32_t));
                file.read(reinterpret_cast<char*>(bufferi32),sizeof(int32_t) * (*length - single_offset));
                for(int i = 0; i < (*length - single_offset); i++)
                    (data+single_offset)[i] = (float)(bufferi32[i]);
                free(bufferi32);
            }
            else
            {
                file.seek(tr_off + (sizeof(int32_t) * -single_offset));

                bufferi32 = (int32_t*)malloc((*length + single_offset)*sizeof(int32_t));
                file.read(reinterpret_cast<char*>(bufferi32),sizeof(int32_t) * (*length + single_offset));
                for(int i = 0; i < (*length + single_offset); i++)
                    (data)[i] = (float)(bufferi32[i]);
                free(bufferi32);

                for(int i = *length + single_offset; i < *length ; i++)
                    data[i] = 0;

            }
            break;
        case Curve::CurveType(UINT16):
            *length  = size/sizeof(uint16_t);
            data = (float *)malloc((*length)*sizeof(float));
            // read full data
            if (single_offset > 0)
            {

                for(int i = 0; i < single_offset; i++)
                    data[i] = 0;

                bufferui16 = (uint16_t*)malloc((*length - single_offset)*sizeof(uint16_t));
                file.read(reinterpret_cast<char*>(bufferui16),sizeof(uint16_t) * (*length - single_offset));
                for(int i = 0; i < (*length - single_offset); i++)
                    (data+single_offset)[i] = (float)(bufferui16[i]);
                free(bufferui16);
            }
            else
            {
                file.seek(tr_off + (sizeof(uint16_t) * -single_offset));

                bufferui16 = (uint16_t*)malloc((*length + single_offset)*sizeof(uint16_t));
                file.read(reinterpret_cast<char*>(bufferui16),sizeof(uint16_t) * (*length + single_offset));
                for(int i = 0; i < (*length + single_offset); i++)
                    (data)[i] = (float)(bufferui16[i]);
                free(bufferui16);

                for(int i = *length + single_offset; i < *length ; i++)
                    data[i] = 0;

            }
            break;
        case Curve::CurveType(INT16):
            *length  = size/sizeof(int16_t);
            data = (float *)malloc((*length)*sizeof(float));
            // read full data
            if (single_offset > 0)
            {

                for(int i = 0; i < single_offset; i++)
                    data[i] = 0;

                bufferi16 = (int16_t*)malloc((*length - single_offset)*sizeof(int16_t));
                file.read(reinterpret_cast<char*>(bufferi16),sizeof(int16_t) * (*length - single_offset));
                for(int i = 0; i < (*length - single_offset); i++)
                    (data+single_offset)[i] = (float)(bufferi16[i]);
                free(bufferi16);
            }
            else
            {
                file.seek(tr_off + (sizeof(int16_t) * -single_offset));

                bufferi16 = (int16_t*)malloc((*length + single_offset)*sizeof(int16_t));
                file.read(reinterpret_cast<char*>(bufferi16),sizeof(int16_t) * (*length + single_offset));
                for(int i = 0; i < (*length + single_offset); i++)
                    (data)[i] = (float)(bufferi16[i]);
                free(bufferi16);

                for(int i = *length + single_offset; i < *length ; i++)
                    data[i] = 0;

            }
            break;
        case Curve::CurveType(UINT8):
            *length  = size/sizeof(uint8_t);
            data = (float *)malloc((*length)*sizeof(float));
            // read full data
            if (single_offset > 0)
            {

                for(int i = 0; i < single_offset; i++)
                    data[i] = 0;

                bufferui8 = (uint8_t*)malloc((*length - single_offset)*sizeof(uint8_t));
                file.read(reinterpret_cast<char*>(bufferui8),sizeof(uint8_t) * (*length - single_offset));
                for(int i = 0; i < (*length - single_offset); i++)
                    (data+single_offset)[i] = (float)(bufferui8[i]);
                free(bufferui8);
            }
            else
            {
                file.seek(tr_off + (sizeof(uint8_t) * -single_offset));

                bufferui8 = (uint8_t*)malloc((*length + single_offset)*sizeof(uint8_t));
                file.read(reinterpret_cast<char*>(bufferui8),sizeof(uint8_t) * (*length + single_offset));
                for(int i = 0; i < (*length + single_offset); i++)
                    (data)[i] = (float)(bufferui8[i]);
                free(bufferui8);

                for(int i = *length + single_offset; i < *length ; i++)
                    data[i] = 0;

            }
            break;
        case Curve::CurveType(INT8):
            *length  = size/sizeof(int8_t);
            data = (float *)malloc((*length)*sizeof(float));
            // read full data
            if (single_offset > 0)
            {

                for(int i = 0; i < single_offset; i++)
                    data[i] = 0;

                bufferi8 = (int8_t*)malloc((*length - single_offset)*sizeof(int8_t));
                file.read(reinterpret_cast<char*>(bufferi8),sizeof(int8_t) * (*length - single_offset));
                for(int i = 0; i < (*length - single_offset); i++)
                    (data+single_offset)[i] = (float)(bufferi8[i]);
                free(bufferi8);
            }
            else
            {
                file.seek(tr_off + (sizeof(int8_t) * -single_offset));

                bufferi8 = (int8_t*)malloc((*length + single_offset)*sizeof(int8_t));
                file.read(reinterpret_cast<char*>(bufferi8),sizeof(int8_t) * (*length + single_offset));
                for(int i = 0; i < (*length + single_offset); i++)
                    (data)[i] = (float)(bufferi8[i]);
                free(bufferi8);

                for(int i = *length + single_offset; i < *length ; i++)
                    data[i] = 0;

            }
            break;
        case Curve::CurveType(DOUBLE):
            *length  = size/sizeof(double);
            data = (float *)malloc((*length)*sizeof(float));
            // read full data
            if (single_offset > 0)
            {

                for(int i = 0; i < single_offset; i++)
                    data[i] = 0;

                bufferd = (double*)malloc((*length - single_offset)*sizeof(double));
                file.read(reinterpret_cast<char*>(bufferd),sizeof(double) * (*length - single_offset));
                for(int i = 0; i < (*length - single_offset); i++)
                    (data+single_offset)[i] = (float)(bufferd[i]);
                free(bufferd);
            }
            else
            {
                file.seek(tr_off + (sizeof(double) * -single_offset));

                bufferd = (double*)malloc((*length + single_offset)*sizeof(double));
                file.read(reinterpret_cast<char*>(bufferd),sizeof(double) * (*length + single_offset));

                for(int i = 0; i < (*length + single_offset); i++)
                    (data)[i] = (float)(bufferd[i]);

                free(bufferd);

                for(int i = *length + single_offset; i < *length ; i++)
                    data[i] = 0;

            }
            break;
        default:
            assert(0);
            break;
    }

    file.close();

    return data;
}

void Curve::curve_clicked(QPointF pt)
{
    QString curve_name = "undef";
    Curve* currCurv = ScaTool::getCurveFromDisplaySerie(this->displayseries);
    if(currCurv != NULL)
    {
        curve_name = currCurv->cname;
    }
    ScaTool::statusbar->showMessage("Curve " + curve_name + " x=" + QString::number(pt.x()) + " y=" + QString::number(pt.y()), 0);
}

void Curve::shift(int offset)
{
    this->xoffset += offset;

    if (this->displayed)
    {
        updateDisplaySeries();
    }
    emit this->shifted();
}

void Curve::setcolorbtn(QPushButton *colorbtn)
{
    if (this->color_btn != 0)
        delete this->color_btn;

    this->color_btn = colorbtn;
    colorbtn->setPalette(QPalette(this->getColor()));
}

void Curve::setchkbox(QCheckBox * chkbox)
{
    if (this->chkbox != 0)
        delete this->chkbox;
    this->chkbox = chkbox;
}

void Curve::settypecmbbox(QComboBox * typecmbbox)
{
    if (this->type_cmbbox != 0)
        delete this->type_cmbbox;
    this->type_cmbbox = typecmbbox;
}

void Curve::chkbox_toggled(bool state)
{
    Curve * curve = this;

    // Check if curve already displayed
    if (curve->displayed)
    {
        // set ui color button to default color
        curve->color_btn->setPalette(QPalette(Qt::white));
        // Set displayed to false (state is false here)
        curve->displayed = state;
        // Hide the curve from display
        curve->getDisplaySeries()->hide();
    }
    else
    {
        // set displayed to true (state is true here)
        curve->displayed = state;

        // Check if series is already present
        if (curve->isLoaded())
        {
            // Display it
            curve->getDisplaySeries()->show();
        }
        else
        {
            // Curve doesn't exist yet, we need to create it
            QtCharts::QLineSeries * curseries = curve->getDisplaySeries();
            ScaTool::main_plot->chart()->addSeries(curseries);
            if (ScaTool::curve_table->firstDisplayed)
            {
                // We create axis realted to added series
                ScaTool::main_plot->chart()->createDefaultAxes();
                // Set original width to ease zoom work
                ScaTool::main_plot->chart()->xaxis_width = curve->length();

                // handler
                connect(qobject_cast<QValueAxis *>(ScaTool::main_plot->chart()->axisX()), &QValueAxis::rangeChanged,ScaTool::main_plot->chart(), &Chart::on_rangeChanged);

                // no more firstdisplayed
                ScaTool::curve_table->firstDisplayed = false;
            }
            else
            {
                // reuse existed axis
                curseries->attachAxis(ScaTool::main_plot->chart()->axisX());
                curseries->attachAxis(ScaTool::main_plot->chart()->axisY());
            }
        }
        // Update color button from color curve
        curve->color_btn->setPalette(QPalette( curve->getDisplaySeries()->color()));
    }
}


void Curve::colorbtn_pressed()
{
    QPushButton * colorbtn = (QPushButton*)sender();
    Curve * curve = this;
    QColorDialog qcd(0);
    qcd.setWindowTitle("Pick a color");
    qcd.exec();
    QColor color = qcd.selectedColor();

    if (qcd.result())
        // Update curve color
        curve->setColor(color);
    else
        return;

    // Update curve color on list
    if (curve->displayed)
    {
        colorbtn->setPalette(QPalette(color));
        curve->displayseries->setColor(color);

        // Trick to redraw
        emit curve->displayseries->pointsReplaced();
    }

    // Update if curve is loaded but not displayed
    if (curve->isLoaded())
    {
        curve->getDisplaySeries()->setColor(curve->color);
    }

}

void Curve::curve_type_changed(int type)
{
    this->type = type;
    this->updateDisplaySeries();
}
