#include "synchro.h"
#include "mainwindow.h"
#include "assert.h"

Synchro::Synchro(int num):
      QObject()
{
    this->numpass = num;
    this->method = SYNCHRO_METHOD_SOD;
    this->precision = 1;
    QObject::connect(this,&Synchro::finish,ScaTool::synchrodialog,&SynchroDialog::update_progressdialog);
}

void Synchro::run()
{
    if (method == SYNCHRO_METHOD_SOD)
    {
        result.clear();
        for(int i = 0; i < this->curves.length() ; i++)
        {
            result.append(Synchro::min_dist_curve(i));
        }
    }
}

qreal Synchro::min_dist_curve(int idx)
{
    qreal dist = 0;
    int offset = 0;

    qreal distmin = std::numeric_limits<qreal>::max();
    Curve * curve = this->curves.at(idx);
    Curve * cur_ref = this->curves.at(this->curve_ref_idx);

    int initial_ref_offset = this->curve_offset.at(this->curve_ref_idx);
    int initial_offset = this->curve_offset.at(idx);

    QLineSeries * ref_subseries = cur_ref->getSubSeries(initial_ref_offset+leftpattern,initial_ref_offset+rightpattern);
    QLineSeries * work_subseries = curve->getSubSeries(initial_offset+leftpattern+leftwindow, initial_offset+rightpattern+rightwindow);

    if (cur_ref != curve)
    {
        for (int s = leftwindow; s < rightwindow; s++)
        {
            dist = 0;

            for (int p = leftpattern ; p < rightpattern; p += precision)
                dist += qAbs(ref_subseries->at(p-leftpattern).y() - work_subseries->at(p+s-leftpattern-leftwindow).y());

            distmin = std::min(dist,distmin);
            if (distmin == dist)
            {
                offset = -s;
            }
        }
    }
    else
    {
        offset = 0;
        distmin = 0;
    }
    {
        // apply shift
        curve->shift(offset - curve->xoffset + initial_offset);

        // Populate offsets
        if (curve->offsets.length() > numpass)
            curve->offsets.replace(numpass,curve->xoffset);
        else if (curve->offsets.length() == numpass)
            curve->offsets.append(curve->xoffset);
        else
            // Impossible but who knows
            assert(false);

        delete ref_subseries;
        delete work_subseries;
    }
    // emit signals for progressbar
    emit this->finish();

    return distmin;
}

void finish()
{
}
