#include "cpa.h"
#include "aes.h"
#include <assert.h>
#include <QtConcurrent/QtConcurrent>

#define INDEX(x,y,X) ((y*X) + x)
CPA::CPA(QVector<Curve*> * vcurves, int keyidx_to_guess, int start, int end)
{
    this->keyidx_to_guess = keyidx_to_guess;
    this->start = start;
    this->end = end;
    this->samples_number = end-start;
    this->curves_number = vcurves->length();
    this->guesses = (uint8_t*)malloc(sizeof(uint8_t)*256*curves_number);
    this->guessessum = (float*)malloc(sizeof(float)*256*2);
    this->curves = (Curve**)malloc(sizeof(Curve*)*curves_number);
    this->rawdata = (float**)malloc(sizeof(float*)*samples_number);
    for (int pts = 0; pts < samples_number; pts++)
        rawdata[pts] = (float*)malloc(sizeof(float)*curves_number);
    // Load traces in RAM
    for (int i = 0; i < curves_number; i++)
    {
        Curve *c = vcurves->at(i);
        this->curves[i] = c;

        int length;
        float *buffer = curves[i]->getrawdata(&length);
        // Copy and transpose
        for (int pts = 0; pts < samples_number; pts++)
            rawdata[pts][i] = buffer[pts];
        free(buffer);
    }
}

CPA::~CPA()
{
    free(guesses);
    free(guessessum);
    for(int i = 0; i < samples_number; i++)
        free(rawdata[i]);
    free(curves);
    free(rawdata);
}

void CPA::setbyteidx(int i)
{
    this->keyidx_to_guess = i;
    construct_guess_hw();
}

void CPA::construct_guess_hw()
{
    int byte_idx = keyidx_to_guess;
    for(int kguess = 0; kguess < 256; kguess ++)
    {
        float sumg = 0;
        float sumg2 = 0;

        for (int i = 0; i < curves_number; i++)
        {
            Curve *c = curves[i];
            uint8_t data = c->input[byte_idx];
            uint8_t guess = HW[AES_SBOX[(uint8_t)(data^kguess)]];
            //uint8_t guess = HW[data^kguess];
            this->guesses[INDEX(kguess,i,256)] = guess;

            // Compute sum and sum of square
            sumg += guess;
            sumg2 += guess*guess;
        }
        float sqrt_denumg = sqrt((curves_number*sumg2) - (sumg*sumg));
        this->guessessum[INDEX(kguess,0,256)] = sumg;
        this->guessessum[INDEX(kguess,1,256)] = sqrt_denumg;
    }
}

void CPA::run()
{
    int t = start;
    int n_threads = 4;

    int workload = samples_number/n_threads;

    while(workload < 1) {
        n_threads -= 1;
        workload = samples_number/n_threads;
    }

    pthread_t threads[n_threads];
    std::pair<CPA*,int> *param = (std::pair<CPA*,int> *)malloc(sizeof(std::pair<CPA*,int>)*n_threads);

    while(t < end)
    {
        for(int n = 0; n < n_threads ; n++)
        {
            param[n].first = this;
            param[n].second = t;
            int rc = pthread_create(&threads[n], NULL, (CPA::pearson_correlation),(void*)&param[n]);
            assert(rc==0);
            t++;
        }
        for(int n = 0; n < n_threads ; n++)
        {
            int rc = pthread_join(threads[n], NULL);
            assert(rc==0);
        }
    }
    free(param);
}


void *CPA::pearson_correlation(void * param)
{
    CPA *cpa = ((std::pair<CPA*,int>*)param)->first;
    int time = ((std::pair<CPA*,int>*)param)->second;
    float sumx = 0;
    float sumx2 = 0;
    // ### pre computation as it is independant to key guess
    // Loop over curves to build traces over correlation will be computated
    for (int i = 0; i < cpa->curves_number; i++)
    {
        float data = cpa->rawdata[time][i];
        sumx += data; // Sum(xi)
        sumx2 += data*data; // Sum(xi^2)
    }

    // Computre std deviation
    float sqrt_denumx = sqrt((cpa->curves_number*sumx2) - (sumx*sumx));
    // ### End of pre computation

    // Correlation is computed based on https://en.wikipedia.org/wiki/Pearson_correlation_coefficient
    // Another reference is https://eprint.iacr.org/2015/260
    // Computational Aspects of Correlation Power Analysis -  Paul Bottinelli and Joppe W. Bos
    // rnum = n*Sum(xi*yi) - Sum(xi)*Sum(yi)
    // rdenum = Sqrt(n*Sum(xi^2) - Sum(xi)^2) * Sqrt(n*Sum(yi^2) - Sum(yi)^2)
    // correlation = rnum/rdenum
    // x are our traces and y are our guesses
    // We avoid double computation by precomputation

    // Loop over key guess and compute correlation
   int byte_idx = cpa->keyidx_to_guess;

   for (int k = 0; k < 256; k++)
   {
       // Retrieve previously computed values

       float sumy = cpa->guessessum[INDEX(k,0,256)];
       float sqrt_denumy = cpa->guessessum[INDEX(k,1,256)];
       float sumxy = 0;
       // Compute Sum(xi*yi)
       for(int i = 0; i < cpa->curves_number; i++)
       {
           sumxy += cpa->rawdata[time][i]*cpa->guesses[INDEX(k,i,256)];
       }
       // Pearson correlation computation
       float r = ((cpa->curves_number*sumxy)-(sumx*sumy))/(sqrt_denumx*sqrt_denumy);
       // DEBUG PURPOSE
       if (r < -0.7)
           qDebug("DEBUG: Max correlation for byte %d time %d with key %d : %f\n",byte_idx,time,k,r);
       //corr.append(r);

   }
   return NULL;
}
