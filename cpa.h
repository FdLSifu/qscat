#ifndef CPA_H
#define CPA_H

#include <QFuture>
#include <QPair>
#include "curve.h"

class CPA
{
public:
    // Input
    Curve **curves;
    float **rawdata;
    int curves_number;
    int keyidx_to_guess;
    int start;
    int end;
    int samples_number;
    // Output
    // Dimension : byteidx, keyguessidx, correlations
    //QVector<QFuture<QVector<float>>> correlation_results;

    // Function
    CPA(QVector<Curve*> * curves, int keyidx_to_guess, int start, int end);
    ~CPA();

    void run();
    void setbyteidx(int i);

    // Internal
    // Dimension : byteidx, keyguessidx, guessesidx
    uint8_t* guesses;
    // Dimension : byteidx, keyguessidx, [sumg,sqrt()]
    float* guessessum;
    void construct_guess_hw();

    static void *pearson_correlation(void *param);
};

#endif // CPA_H
