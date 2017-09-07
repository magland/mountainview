/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/27/2016
*******************************************************/

#include <math.h>
#include "jisotonic.h"
#include <stdlib.h>
#include <stdio.h>

int find_index_of_minimum(int N, double* X);
double compute_ks4(int N, double* counts1, double* counts2);

bool isocut4(int N, double* dipscore, double* cutpoint, double* samples)
{
    printf("test %d\n", __LINE__);

    double num_bins_factor = 1;
    int num_bins = ceil(sqrt(N * 1.0 / 2) * num_bins_factor);

    double* X = (double*)malloc(sizeof(double) * N);
    double* intervals = (double*)malloc(sizeof(double) * num_bins);
    int* inds = (int*)malloc(sizeof(int) * (num_bins + 1));
    double* X_sub = (double*)malloc(sizeof(double) * (num_bins + 1));
    double* spacings = (double*)malloc(sizeof(double) * num_bins);
    double* multiplicities = (double*)malloc(sizeof(double) * num_bins);
    double* densities = (double*)malloc(sizeof(double) * num_bins);
    double* inverse_spacings = (double*)malloc(sizeof(double) * num_bins);
    double* densities_unimodal_fit = (double*)malloc(sizeof(double) * num_bins);
    double* densities_resid = (double*)malloc(sizeof(double) * num_bins);
    double* densities_resid_fit = (double*)malloc(sizeof(double) * num_bins);
    double* unimodal_fit_counts = (double*)malloc(sizeof(double) * num_bins);

    printf("test %d\n", __LINE__);

    jisotonic_sort(N, X, samples);

    printf("test %d\n", __LINE__);

    int num_bins_1 = ceil(num_bins * 1.0 / 2);
    int num_bins_2 = num_bins - num_bins_1;

    for (int i = 0; i < num_bins_1; i++) {
        intervals[i] = i + 1;
    }
    for (int i = 0; i < num_bins_2; i++) {
        intervals[num_bins - 1 - i] = i + 1;
    }
    int sum_intervals = 0;
    for (int i = 0; i < num_bins; i++) {
        sum_intervals += intervals[i];
    }
    double alpha = (N - 1) * 1.0 / sum_intervals;
    for (int i = 0; i < num_bins; i++) {
        intervals[i] = intervals[i] * alpha;
    }
    inds[0] = 0;
    double cumsum = 0;
    for (int i = 0; i < num_bins; i++) {
        cumsum += intervals[i];
        inds[i + 1] = floor(cumsum);
        if (inds[i + 1] >= N) {
            return false;
        }
    }

    printf("test %d\n", __LINE__);

    for (int i = 0; i < num_bins + 1; i++) {
        X_sub[i] = X[inds[i]];
        if (i > 0) {
            spacings[i - 1] = X_sub[i] - X_sub[i - 1];
            multiplicities[i - 1] = inds[i] - inds[i - 1];
            if (!spacings[i - 1]) {
                return false;
            }
            densities[i - 1] = multiplicities[i - 1] / spacings[i - 1];
            inverse_spacings[i - 1] = 1 / spacings[i - 1];
        }
    }

    printf("test %d\n", __LINE__);

    jisotonic_updown(num_bins, densities_unimodal_fit, densities, multiplicities);

    printf("test %d\n", __LINE__);

    for (int i = 0; i < num_bins; i++) {
        densities_resid[i] = densities[i] - densities_unimodal_fit[i];
    }
    jisotonic_downup(num_bins, densities_resid_fit, densities_resid, inverse_spacings);

    printf("test %d\n", __LINE__);

    int cutpoint_index = find_index_of_minimum(num_bins, densities_resid_fit);
    printf("test %d %d %d\n", __LINE__, cutpoint_index, num_bins);
    *cutpoint = (X_sub[cutpoint_index] + X_sub[cutpoint_index + 1]) / 2;
    printf("test %d\n", __LINE__);

    for (int i = 0; i < num_bins; i++) {
        unimodal_fit_counts[i] = densities_unimodal_fit[i] * spacings[i];
    }
    *dipscore = compute_ks4(num_bins, multiplicities, unimodal_fit_counts);

    printf("test %d\n", __LINE__);

    free(X);
    free(intervals);
    free(inds);
    free(X_sub);
    free(spacings);
    free(multiplicities);
    free(densities);
    free(inverse_spacings);
    free(densities_unimodal_fit);
    free(densities_resid);
    free(densities_resid_fit);
    free(unimodal_fit_counts);

    printf("test %d\n", __LINE__);

    return true;
}

int find_index_of_minimum(int N, double* X)
{
    if (N == 0)
        return 0;
    int ret = 0;
    for (int i = 0; i < N; i++) {
        if (X[i] <= X[ret])
            ret = i;
    }
    return ret;
}

double compute_ks4(int N, double* counts1, double* counts2)
{
    double sum_counts1 = 0;
    double sum_counts2 = 0;
    for (int i = 0; i < N; i++) {
        sum_counts1 += counts1[i];
        sum_counts2 += counts2[i];
    }
    double S1 = 0, S2 = 0;
    double max_diff = -9999;
    for (int i = 0; i < N; i++) {
        S1 += counts1[i] / sum_counts1;
        S2 += counts2[i] / sum_counts2;
        double diff = fabs(S1 - S2);
        if (diff >= max_diff)
            max_diff = diff;
    }
    double ks = max_diff * sqrt((sum_counts1 + sum_counts2) / 2);
    return ks;
}
