#include "synchro.h"
#include "mainwindow.h"
#include "assert.h"
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

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
            // Start the computation.
            QFuture<qreal> future = QtConcurrent::run(Synchro::min_dist_curve,this,i);
            result.append(future);
        }
    }
}

qreal Synchro::min_dist_curve(Synchro *sync,int idx)
{
    qreal dist = 0;
    int offset = 0;

    qreal distmin = std::numeric_limits<qreal>::max();
    Curve * curve = sync->curves.at(idx);
    Curve * cur_ref = sync->curves.at(sync->curve_ref_idx);

    int initial_ref_offset = sync->curve_offset.at(sync->curve_ref_idx);
    int initial_offset = sync->curve_offset.at(idx);

    QLineSeries * ref_subseries = cur_ref->getSubSeries(sync->leftpattern-initial_ref_offset,sync->rightpattern-initial_ref_offset);
    QLineSeries * work_subseries = curve->getSubSeries(sync->leftpattern+sync->leftwindow-initial_offset, sync->rightpattern+sync->rightwindow-initial_offset);

    if (cur_ref != curve)
    {
        for (int s = sync->leftwindow; s < sync->rightwindow; s++)
        {
            dist = 0;

            for (int p = sync->leftpattern ; p < sync->rightpattern; p += sync->precision)
                dist += qAbs(ref_subseries->at(p-sync->leftpattern).y() - work_subseries->at(p+s-sync->leftpattern-sync->leftwindow).y());

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
        if (curve->offsets.length() > sync->numpass)
            curve->offsets.replace(sync->numpass,offset + initial_offset);
        else if (curve->offsets.length() == sync->numpass)
            curve->offsets.append(offset + initial_offset);
        else
            // Impossible but who knows
            assert(false);

        delete ref_subseries;
        delete work_subseries;
    }
    // emit signals for progressbar
    emit sync->finish();

    return distmin;
}

void finish()
{
}
