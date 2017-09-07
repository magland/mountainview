#include "isocut.h"
#include "mda.h"
#include <math.h>
#include "jisotonic.h"
#include <stdio.h>
#include <stdlib.h>

double compute_ks(int N1, int N2, double* samples1, double* samples2);

//bool isocut(int N,double *cutpoint,double *samples_in) {
//    return isocut(N,cutpoint,samples_in,1.4,4);
//}

bool isocut(int N, double* cutpoint, const double* samples_in, double threshold)
{
    return isocut(N, cutpoint, samples_in, threshold, 4);
}

bool isocut(int N, double* cutpoint, const double* samples_in, double threshold, int minsize)
{
    *cutpoint = 0;
    double* samples = (double*)malloc(sizeof(double) * N);
    jisotonic_sort(N, samples, samples_in);

    int* N0s = (int*)malloc(sizeof(int) * (N * 2 + 2)); //conservative malloc
    int num_N0s = 0;
    for (int ii = 2; ii <= floor(log2(N / 2 * 1.0)); ii++) {
        N0s[num_N0s] = pow(2, ii);
        num_N0s++;
        N0s[num_N0s] = -pow(2, ii);
        num_N0s++;
    }
    N0s[num_N0s] = N;
    num_N0s++;

    bool found = false;
    for (int jj = 0; (jj < num_N0s) && (!found); jj++) {
        int N0 = N0s[jj];
        int NN0 = N0;
        if (N0 < 0)
            NN0 = -N0;
        double* samples0 = (double*)malloc(sizeof(double) * NN0);
        double* spacings0 = (double*)malloc(sizeof(double) * (NN0 - 1));
        double* spacings0_fit = (double*)malloc(sizeof(double) * (NN0 - 1));
        double* samples0_fit = (double*)malloc(sizeof(double) * NN0);

        if (N0 > 0) {
            for (int ii = 0; ii < NN0; ii++) {
                samples0[ii] = samples[ii];
            }
        }
        else {
            for (int ii = 0; ii < NN0; ii++) {
                samples0[NN0 - 1 - ii] = samples[N - 1 - ii];
            }
        }
        for (int ii = 0; ii < NN0 - 1; ii++) {
            spacings0[ii] = samples0[ii + 1] - samples0[ii];
        }
        jisotonic_downup(NN0 - 1, spacings0_fit, spacings0, 0);
        samples0_fit[0] = samples0[0];
        for (int ii = 1; ii < NN0; ii++) {
            samples0_fit[ii] = samples0_fit[ii - 1] + spacings0_fit[ii - 1];
        }
        double ks0 = compute_ks(NN0, NN0, samples0, samples0_fit);
        ks0 *= sqrt(NN0 * 1.0);
        if (ks0 >= threshold) {
            for (int ii = 0; ii < NN0 - 1; ii++) {
                if (spacings0_fit[ii])
                    spacings0[ii] = spacings0[ii] / spacings0_fit[ii];
            }
            jisotonic_updown(NN0 - 1, spacings0_fit, spacings0, 0);
            if (NN0 >= minsize * 2) {
                int max_ind = minsize - 1;
                double maxval = 0;
                for (int ii = minsize - 1; ii <= NN0 - 1 - minsize; ii++) {
                    if (spacings0_fit[ii] > maxval) {
                        maxval = spacings0_fit[ii];
                        max_ind = ii;
                    }
                }
                *cutpoint = (samples0[max_ind] + samples0[max_ind + 1]) / 2;
                found = true;
            }
        }

        free(samples0);
        free(spacings0);
        free(spacings0_fit);
        free(samples0_fit);
    }

    free(N0s);
    free(samples);

    return found;
}

double compute_ks(int N1, int N2, double* samples1, double* samples2)
{
    double max_dist = 0;
    int ii = 0;
    for (int j = 0; j < N2; j++) {
        while ((ii + 1 < N1) && (samples1[ii + 1] <= samples2[j]))
            ii++;
        const double dist = fabs(j * 1.0 / N2 - ii * 1.0 / N1);
        max_dist = std::max(max_dist, dist);
    }
    return max_dist;
}
