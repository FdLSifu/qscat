#include "cpa.h"
#include "aes.h"
#include <assert.h>
#include <QtConcurrent/QtConcurrent>

#define INDEX(x,y,X) ((y*X) + x)
CPA::CPA(QVector<Curve*> * vcurves, int sel_fun, int start, int end)
{
    this->keyidx_to_guess = 0;
    this->byteidx = QList<int>();
    this->sel_fun = sel_fun;
    this->start = start;
    this->end = end;
    this->samples_number = end-start;
    this->curves_number = vcurves->length();
    this->guesses = (uint8_t*)malloc(sizeof(uint8_t)*256*curves_number);
    this->guessessum = (float*)malloc(sizeof(float)*256*2);
    this->curves = (Curve**)malloc(sizeof(Curve*)*curves_number);
    this->rawdata = (float**)malloc(sizeof(float*)*samples_number);
    if (samples_number >= 0)
    {
        for (int pts = 0; pts < samples_number; pts++)
            rawdata[pts] = (float*)malloc(sizeof(float)*curves_number);
        // Load traces in RAM
        for (int i = 0; i < curves_number; i++)
        {
            Curve *c = vcurves->at(i);
            this->curves[i] = c;

            int length;
            float *buffer = curves[i]->getrawdata(&length,c->xoffset);
            // Copy and transpose to gain A LOT OF TIME
            for (int pts = 0; pts < samples_number; pts++)
                rawdata[pts][i] = buffer[pts+start];
            free(buffer);
        }
    }

    correlation = (float***)malloc(sizeof(float**)*16);
    for (int bi = 0; bi < 16; bi ++)
    {
        correlation[bi] = (float**)malloc(sizeof(float*)*256);
        for(int k = 0; k < 256; k++)
        {
            correlation[bi][k] = (float*)malloc(sizeof(float)*samples_number);
        }
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

    for (int bi = 0; bi < 16; bi ++)
    {
        for(int k = 0; k < 256; k++)
            free(correlation[bi][k]);
        free(correlation[bi]);
    }
    free(correlation);
}

void CPA::setcurrentbyteidx(int i)
{
    this->keyidx_to_guess = i;
    construct_guess_hw();
}

void CPA::construct_guess_hw()
{
    if(curves_number < 0)
        return;

    uint8_t (*func)(uint8_t,uint8_t);

    switch(sel_fun)
    {
        case 0: // AES SBOX
            func = &aes_sbox;
            break;
        case 1: // AES MULT_INV
            func = &aes_multinv;
            break;
        case 2: // XOR
            func = &generic_xor;
            break;
        case 3: // SBOX INV
            func = &aes_sboxinv;
            break;
        default:
            assert(false);

    }
    int byte_idx = keyidx_to_guess;
    for(int kguess = 0; kguess < 256; kguess ++)
    {
        float sumg = 0;
        float sumg2 = 0;

        for (int i = 0; i < curves_number; i++)
        {
            Curve *c = curves[i];
            uint8_t data = c->input[byte_idx];
            uint8_t guess = (*func)(data,kguess);
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

void CPA::run(CPA *cpa)
{
    bool allocdone = false;
    std::pair<CPA*,int> *param;
    for(int bidx = 0; bidx < cpa->byteidx.length(); bidx ++)
    {
        int currentbyte = cpa->byteidx.at(bidx);
        cpa->setcurrentbyteidx(currentbyte);
        int t = cpa->start;
        int n_threads = 4;

        if(cpa->samples_number<=0)
            return;

        int workload = cpa->samples_number/n_threads;

        while(workload < 1) {
            n_threads -= 1;
            workload = cpa->samples_number/n_threads;
        }

        pthread_t threads[n_threads];
        if (allocdone == false)
        {
            param = (std::pair<CPA*,int> *)malloc(sizeof(std::pair<CPA*,int>)*n_threads);
            allocdone = true;
        }
        while(t < cpa->end)
        {
            int n = 0;
            while( n < n_threads && t < cpa->end)
            {
                param[n].first = cpa;
                param[n].second = t;
                int rc = pthread_create(&threads[n], NULL, (CPA::pearson_correlation),(void*)&param[n]);
                assert(rc==0);
                t++;
                n++;
            }
            while ( n-- > 0)
            {
                int rc = pthread_join(threads[n], NULL);
                assert(rc==0);
            }
        }
        emit cpa->finished(currentbyte);
    }
    if(allocdone == true)
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
        float data = cpa->rawdata[time-cpa->start][i];
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
           sumxy += cpa->rawdata[time-cpa->start][i]*cpa->guesses[INDEX(k,i,256)];
       }
       // Pearson correlation computation
       float r = ((cpa->curves_number*sumxy)-(sumx*sumy))/(sqrt_denumx*sqrt_denumy);

       cpa->correlation[byte_idx][k][time-cpa->start] = r;

   }
   return NULL;
}
