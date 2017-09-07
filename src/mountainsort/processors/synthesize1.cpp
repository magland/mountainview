/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/5/2016
*******************************************************/

#include "synthesize1.h"
#include "mda.h"
#include <random>
#include "msmisc.h"
#include "get_sort_indices.h"

#include <mda32.h>

double rand_uniform(double a, double b);
int rand_int(int a, int b);
double eval_waveform(Mda& W, int m, double t, int k, int waveforms_oversamp);

bool synthesize1(const QString& waveforms_in_path, const QString& info_in_path, const QString& timeseries_out_path, const QString& firings_true_path, const synthesize1_opts& opts)
{
    int N = opts.N;
    double noise_level = opts.noise_level;
    int waveforms_oversamp = opts.waveforms_oversamp;
    double samplerate = opts.samplerate;

    Mda W(waveforms_in_path);
    Mda info(info_in_path);
    int M = W.N1();
    int T0 = W.N2();
    int K = W.N3();

    if ((T0 % waveforms_oversamp) != 0) {
        qWarning() << "T0 is not divisible by waveforms_oversamp" << T0 << waveforms_oversamp;
        return false;
    }

    int T = T0 / waveforms_oversamp;

    //define true times and labels
    QVector<double> times;
    QVector<int> labels;
    QVector<double> hscales, vscales;
    for (int k = 0; k < K; k++) {
        int pop = (int)(info.value(0, k) / samplerate * N);
        double refractory_period = info.value(1, k);
        double vscale1 = info.value(2, k);
        double vscale2 = info.value(3, k);
        double hscale1 = info.value(4, k);
        double hscale2 = info.value(5, k);
        printf("k=%d, pop=%d, refr=%g ms (%g timepoints)\n", k, pop, refractory_period, refractory_period / 1000 * samplerate);
        QVector<double> times_k;
        for (int a = 0; a < pop; a++) {
            double t0 = rand_uniform(T + 1, N - T - 1);
            times_k << t0;
        }
        qSort(times_k);
        double last_t0 = 0;
        for (int i = 0; i < times_k.count(); i++) {
            double t0 = times_k[i];
            if (t0 >= last_t0 + refractory_period / 1000 * samplerate) {
                last_t0 = t0;
                double vscale0 = 1;
                if ((vscale1) || (vscale2))
                    vscale0 = rand_uniform(vscale1, vscale2);
                double hscale0 = 1;
                if ((hscale1) || (hscale2))
                    hscale0 = rand_uniform(hscale1, hscale2);
                times << times_k[i];
                labels << k + 1;
                hscales << hscale0;
                vscales << vscale0;
            }
        }
    }

    Mda32 X(M, N);
    // Random noise
    generate_randn(X.totalSize(), X.dataPtr());
    for (int n = 0; n < N; n++) {
        for (int m = 0; m < M; m++) {
            double val = X.get(m, n);
            val = val * noise_level;
            X.set(val, m, n);
        }
    }

    QList<int> inds = get_sort_indices(times);
    QVector<double> times2;
    QVector<int> labels2;
    for (int i = 0; i < times.count(); i++) {
        times2 << times[inds[i]];
        labels2 << labels[inds[i]];
    }

    Mda firings(3, times2.count());
    for (int i = 0; i < times2.count(); i++) {
        double t0 = times2[i];
        double k0 = labels2[i];
        double hscale0 = hscales[i];
        double vscale0 = vscales[i];
        firings.setValue(t0, 1, i);
        firings.setValue(k0, 2, i);
        for (int t = (int)t0 - T / 2; t < (int)t0 + T / 2; t++) {
            for (int m = 0; m < M; m++) {
                double val = vscale0 * eval_waveform(W, m, (t - t0) * hscale0, k0 - 1, waveforms_oversamp);
                X.setValue(X.value(m, t) + val, m, t);
            }
        }
    }
    firings.write64(firings_true_path);

    return X.write32(timeseries_out_path);
}

double eval_waveform(Mda& W, int m, double t, int k, int waveforms_oversamp)
{
    int center0 = (int)((W.N2() + 1) / 2) - 1;
    double ind = center0 + t * waveforms_oversamp;
    int ind0 = (int)ind;
    int ind1 = (int)(ind + 1);
    double p = ind - ind0;
    if ((ind0 < 0) || (ind1 >= W.N2()))
        return 0;
    double val0 = W.value(m, ind0, k);
    double val1 = W.value(m, ind1, k);
    return p * val1 + (1 - p) * val0;
}

int rand_int(int a, int b)
{
    if (b <= a)
        return a;
    return a + (qrand() % (b - a + 1));
}

double rand_uniform(double a, double b)
{
    double val = (qrand() + 0.5) / RAND_MAX;
    return a + val * (b - a);
}

void generate_randn(size_t N, float* X)
{
    std::random_device rd; //to generate a non-deterministic integer (I believe)
    std::mt19937 e2(rd()); //the random number generator, seeded by rd() (I believe)
    std::normal_distribution<> dist(0, 1); //to generate random numbers from normal distribution
    for (size_t n = 0; n < N; n++) {
        X[n] = dist(e2);
    }
}
