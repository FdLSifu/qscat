#ifndef CPA_H
#define CPA_H

#include <QFuture>
#include <QPair>
#include "curve.h"

class CPA : public QWidget
{
    Q_OBJECT
public:
    // Input
    Curve **curves;
    float **rawdata;
    int curves_number;
    int sel_fun;
    int keyidx_to_guess;
    int start;
    int end;
    int samples_number;
    // Output
    // byte x key x corr
    float ***correlation;

    QList<int> byteidx;

    // Function
    CPA(QVector<Curve*> * curves, int sel_fun, int start, int end);
    ~CPA();

    static void run(CPA *cpa);

private:
    // Internal
    // Dimension : byteidx, keyguessidx, guessesidx
    uint8_t* guesses;
    // Dimension : byteidx, keyguessidx, [sumg,sqrt()]
    float* guessessum;

    void construct_guess_hw();
    void setcurrentbyteidx(int i);
    static void *pearson_correlation(void *param);
signals:
    void finished(int byte);
};

#endif // CPA_H
