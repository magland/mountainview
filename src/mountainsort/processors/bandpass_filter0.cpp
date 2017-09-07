#include "bandpass_filter0.h"
#include "diskreadmda.h"

#include <diskwritemda.h>
#include "omp.h"
#include "fftw3.h"
#include <QTime>
#include <math.h>
//#include "msprefs.h"
#include <QTime>

#ifdef USE_SSE2
#include <diskreadmda32.h>
#include <emmintrin.h>
#include <immintrin.h>
#endif

Mda32 do_bandpass_filter0(Mda32& X, double samplerate, double freq_min, double freq_max, double freq_wid);
bool do_fft_1d_r2c(int M, int N, float* out, float* in);
bool do_ifft_1d_c2r(int M, int N, float* out, float* in);
void multiply_complex_by_real_kernel(int M, int N, float* Y, double* kernel);
void define_kernel(int N, double* kernel, double samplefreq, double freq_min, double freq_max, double freq_wid);

bool bandpass_filter0(const QString& input_path, const QString& output_path, double samplerate, double freq_min, double freq_max, double freq_wid)
{
    QTime timer_total;
    timer_total.start();
    QMap<QString, int> elapsed_times;

    DiskReadMda32 X(input_path);
    const int M = X.N1();
    const int N = X.N2();

    DiskWriteMda Y(MDAIO_TYPE_FLOAT32, output_path, M, N);

    int num_threads = omp_get_max_threads();
    int memory_size = 0.1 * 1e9;
    int chunk_size = memory_size * 1.0 / (M * 4 * num_threads);
    chunk_size = qMin(N * 1.0, qMax(1e4 * 1.0, chunk_size * 1.0));
    int overlap_size = chunk_size / 5;
    printf("************ Using chunk size / overlap size: %d / %d (num threads=%d)\n", chunk_size, overlap_size, num_threads);

    {
        QTime timer_status;
        timer_status.start();
        int num_timepoints_handled = 0;
        int bytes_allocated = 0;
#pragma omp parallel for
        for (int timepoint = 0; timepoint < N; timepoint += chunk_size) {
            QMap<QString, int> elapsed_times_local;
            Mda32 chunk;
#pragma omp critical(lock1)
            {
                QTime timer;
                timer.start();
                X.readChunk(chunk, 0, timepoint - overlap_size, M, chunk_size + 2 * overlap_size);
                bytes_allocated += chunk.totalSize() * 4; //for debugging, if needed
                elapsed_times["readChunk"] += timer.elapsed();
            }
            {
                QTime timer;
                timer.start();
                chunk = do_bandpass_filter0(chunk, samplerate, freq_min, freq_max, freq_wid);
                elapsed_times_local["do_bandpass_filter0"] += timer.elapsed();
            }
            Mda32 chunk2;
            {
                QTime timer;
                timer.start();
                chunk.getChunk(chunk2, 0, overlap_size, M, chunk_size);
                elapsed_times_local["getChunk"] += timer.elapsed();
            }
#pragma omp critical(lock1)
            {
                elapsed_times["do_bandpass_filter0"] += elapsed_times_local["do_bandpass_filter0"];
                elapsed_times["getChunk"] += elapsed_times_local["getChunk"];

                {
                    QTime timer;
                    timer.start();
                    Y.writeChunk(chunk2, 0, timepoint);
                    elapsed_times["writeChunk"] += timer.elapsed();
                }
                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer_status.elapsed() > 5000) || (num_timepoints_handled == N) || (timepoint == 0)) {
                    printf("%d/%d (%d%%) - Elapsed(s): RC:%g, BPF:%g, GC:%g, WC:%g, Total:%g, %d threads\n",
                        num_timepoints_handled, N,
                        (int)(num_timepoints_handled * 1.0 / N * 100),
                        elapsed_times["readChunk"] * 1.0 / 1000,
                        elapsed_times["do_bandpass_filter0"] * 1.0 / 1000,
                        elapsed_times["getChunk"] * 1.0 / 1000,
                        elapsed_times["writeChunk"] * 1.0 / 1000,
                        timer_total.elapsed() * 1.0 / 1000,
                        omp_get_num_threads());
                    timer_status.restart();
                }
                bytes_allocated -= chunk.totalSize() * 4;
            }
        }
    }

    return true;
}

void multiply_by_factor(int N, float* X, double factor)
{
    /*int start = 0;
#ifdef USE_SSE2
    __m128d factor_m128 = _mm_load_pd1(&factor);
    for (; start < (N / 2) * 2; start += 2) {
        double* chunk = X + start;
        __m128d x = _mm_load_pd(chunk);
        __m128d result = _mm_mul_pd(x, factor_m128);
        _mm_store_pd(chunk, result);
    }
#endif
*/
    for (int i = 0; i < N; i++)
        X[i] *= factor;
}

Mda32 do_bandpass_filter0(Mda32& X, double samplerate, double freq_min, double freq_max, double freq_wid)
{
    int M = X.N1();
    int N = X.N2();
    int MN = M * N;
    Mda32 Y(M, N);
    float* Xptr = X.dataPtr();
    float* Yptr = Y.dataPtr();

    double* kernel0 = (double*)allocate(sizeof(double) * N);
    float* Xhat = (float*)allocate(sizeof(float) * MN * 2);
    define_kernel(N, kernel0, samplerate, freq_min, freq_max, freq_wid);

    do_fft_1d_r2c(M, N, Xhat, Xptr);
    multiply_complex_by_real_kernel(M, N, Xhat, kernel0);
    do_ifft_1d_c2r(M, N, Yptr, Xhat);

    multiply_by_factor(MN, Yptr, 1.0 / N);

    free(kernel0);
    free(Xhat);

    return Y;
}

bool do_fft_1d_r2c(int M, int N, float* out, float* in)
{
    /*
	if (num_threads>1) {
		fftw_init_threads();
		fftw_plan_with_nthreads(num_threads);
	}
	*/

    int MN = M * N;

    fftw_complex* in2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * MN);
    fftw_complex* out2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * MN);
    for (int ii = 0; ii < MN; ii++) {
        //in2[ii][0]=in[ii*2];
        //in2[ii][1]=in[ii*2+1];
        in2[ii][0] = in[ii];
        in2[ii][1] = 0;
    }

    /*
	 * From FFTW docs:
	 * howmany is the number of transforms to compute.
	 * The resulting plan computes howmany transforms,
	 * where the input of the k-th transform is at
	 * location in+k*idist (in C pointer arithmetic),
	 * and its output is at location out+k*odist.
	 * Plans obtained in this way can often be faster
	 * than calling FFTW multiple times for the individual
	 * transforms. The basic fftw_plan_dft interface corresponds
	 * to howmany=1 (in which case the dist parameters are ignored).
	 *
	 * Each of the howmany transforms has rank rank
	 * and size n, as in the basic interface.
	 * In addition, the advanced interface allows the
	 * input and output arrays of each transform to be
	 * row-major subarrays of larger rank-rank arrays,
	 * described by inembed and onembed parameters,
	 * respectively. {i,o}nembed must be arrays of length
	 * rank, and n should be elementwise less than or equal
	 * to {i,o}nembed. Passing NULL for an nembed parameter
	 * is equivalent to passing n (i.e. same physical and
	 * logical dimensions, as in the basic interface.)
	 *
	 * The stride parameters indicate that the j-th element
	 * of the input or output arrays is located at j*istride
	 * or j*ostride, respectively. (For a multi-dimensional array,
	 * j is the ordinary row-major index.) When combined with
	 * the k-th transform in a howmany loop, from above, this
	 * means that the (j,k)-th element is at j*stride+k*dist.
	 * (The basic fftw_plan_dft interface corresponds to a stride
	 * of 1.)
	 */
    fftw_plan p;
    int rank = 1;
    int n[] = { N };
    int howmany = M;
    int* inembed = n;
    int istride = M;
    int idist = 1;
    int* onembed = n;
    int ostride = M;
    int odist = 1;
    int sign = FFTW_FORWARD;
    unsigned flags = FFTW_ESTIMATE;
#pragma omp critical
    p = fftw_plan_many_dft(rank, n, howmany, in2, inembed, istride, idist, out2, onembed, ostride, odist, sign, flags);
    //p=fftw_plan_dft_1d(N,in2,out2,FFTW_FORWARD,FFTW_ESTIMATE);

    fftw_execute(p);
    for (int ii = 0; ii < MN; ii++) {
        out[ii * 2] = out2[ii][0];
        out[ii * 2 + 1] = out2[ii][1];
    }
    fftw_free(in2);
    fftw_free(out2);

/*
	if (num_threads>1) {
		fftw_cleanup_threads();
	}
	*/

#pragma omp critical
    fftw_destroy_plan(p);

    return true;
}

bool do_ifft_1d_c2r(int M, int N, float* out, float* in)
{
    /*
	if (num_threads>1) {
		fftw_init_threads();
		fftw_plan_with_nthreads(num_threads);
	}
	*/

    int MN = M * N;

    fftw_complex* in2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * MN);
    fftw_complex* out2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * MN);
    for (int ii = 0; ii < MN; ii++) {
        in2[ii][0] = in[ii * 2];
        in2[ii][1] = in[ii * 2 + 1];
    }

    fftw_plan p;
    int rank = 1;
    int n[] = { N };
    int howmany = M;
    int* inembed = n;
    int istride = M;
    int idist = 1;
    int* onembed = n;
    int ostride = M;
    int odist = 1;
    int sign = FFTW_BACKWARD;
    unsigned flags = FFTW_ESTIMATE;
#pragma omp critical
    p = fftw_plan_many_dft(rank, n, howmany, in2, inembed, istride, idist, out2, onembed, ostride, odist, sign, flags);
    //p=fftw_plan_dft_1d(N,in2,out2,FFTW_BACKWARD,FFTW_ESTIMATE);

    fftw_execute(p);
    for (int ii = 0; ii < MN; ii++) {
        out[ii] = out2[ii][0];
    }
    fftw_free(in2);
    fftw_free(out2);

/*
	if (num_threads>1) {
		fftw_cleanup_threads();
	}
	*/

#pragma omp critical
    fftw_destroy_plan(p);

    return true;
}

void multiply_complex_by_real_kernel(int M, int N, float* Y, double* kernel)
{
    int bb = 0;
    int aa = 0;
    /*
#ifdef USE_SSE2
    for (int i = 0; i < N; i++) {
        __m128d kernel_m128 = _mm_load_pd1(kernel + aa);
        for (int j = 0; j < M; j++) {
            __m128d Y_m128 = _mm_load_pd(Y + bb * 2);
            __m128d result = _mm_mul_pd(Y_m128, kernel_m128);
            _mm_store_pd(Y + bb * 2, result);
            bb++;
        }
        aa++;
    }
#else
*/
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            Y[bb * 2] *= kernel[aa];
            Y[bb * 2 + 1] *= kernel[aa];
            bb++;
        }
        aa++;
    }
    //#endif
}

void define_kernel(int N, double* kernel, double samplefreq, double freq_min, double freq_max, double freq_wid)
{
    // Matches ahb's code /matlab/processors/ms_bandpass_filter.m
    // improved ahb, changing tanh to erf, correct -3dB pts  6/14/16
    double T = N / samplefreq; // total time
    double df = 1 / T; // frequency grid
    double relwid = 3.0; // relative bottom-end roll-off width param, kills low freqs by factor 1e-5.

    //printf("filter params: %.15g %.15g %.15g \n", freq_min, freq_max, freq_wid); // debug
    //freq_wid = 1000.0; // *** why not correctly read in? override hack

    for (int i = 0; i < N; i++) {
        const double fgrid = (i <= (N + 1) / 2) ? df * i : df * (i - N); // why const? (ahb)
        const double absf = fabs(fgrid);
        double val = 1.0;
        if (freq_min != 0) { // (suggested by ahb) added on 3/3/16 by jfm
            if (i == 0)
                val = 0.0; // kill DC part exactly - ahb
            else
                val *= (1 + erf(relwid * (absf - freq_min) / freq_min)) / 2;
        }
        if (freq_max != 0) { // added on 3/3/16 by jfm
            val *= (1 - erf((absf - freq_max) / freq_wid)) / 2;
        }
        kernel[i] = sqrt(val); // note sqrt of filter func to apply to spectral intensity not ampl
    }
}
