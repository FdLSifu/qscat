#ifndef SYNCHRO_H
#define SYNCHRO_H

#include "curve.h"
#include <QFuture>
#include <QRunnable>
#include <QObject>

#define SYNCHRO_METHOD_SOD 0

class Synchro : public QObject, public QRunnable
{
   Q_OBJECT
public:
    // Fields
    QList<Curve *> curves;
    QList<int> curve_offset;
    QList<QFuture<qreal>> result;
    int curve_ref_idx;
    int numpass;
    int method;
    int leftpattern,rightpattern;
    int leftwindow,rightwindow;
    int precision;
    bool preview;
    static bool stop;
    static QMutex mutex;
    // Constructor
    Synchro(int num);

    // Function
    qreal static min_dist_curve(Synchro *s,int idx);
private:
    void run();

signals:
    void finish();
};

#endif // SYNCHRO_H
