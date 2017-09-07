#include "whiten.h"
#include "diskreadmda.h"
#include "diskwritemda.h"
#include "mda.h"
#include "msprefs.h"
#include <math.h>
#include <QDebug>
#include <QTime>
#include "get_sort_indices.h"
#include "eigenvalue_decomposition.h"
#include "matrix_mda.h"
#include "get_pca_features.h"
#include "msmisc.h"

//Mda get_whitening_matrix(Mda& COV);
void quantize(int N, double* X, double unit);

bool whiten(const QString& input, const QString& output)
{
    DiskReadMda X(input);
    int M = X.N1();
    int N = X.N2();

    Mda XXt(M, M);
    double* XXtptr = XXt.dataPtr();
    int chunk_size = PROCESSING_CHUNK_SIZE;
    if (N < PROCESSING_CHUNK_SIZE) {
        chunk_size = N;
    }

    {
        QTime timer;
        timer.start();
        int num_timepoints_handled = 0;
#pragma omp parallel for
        for (int timepoint = 0; timepoint < N; timepoint += chunk_size) {
            Mda chunk;
#pragma omp critical(lock1)
            {
                X.readChunk(chunk, 0, timepoint, M, qMin(chunk_size, N - timepoint));
            }
            double* chunkptr = chunk.dataPtr();
            Mda XXt0(M, M);
            double* XXt0ptr = XXt0.dataPtr();
            for (int i = 0; i < chunk.N2(); i++) {
                int aa = M * i;
                int bb = 0;
                for (int m1 = 0; m1 < M; m1++) {
                    for (int m2 = 0; m2 < M; m2++) {
                        XXt0ptr[bb] += chunkptr[aa + m1] * chunkptr[aa + m2];
                        bb++;
                    }
                }
            }
#pragma omp critical(lock2)
            {
                int bb = 0;
                for (int m1 = 0; m1 < M; m1++) {
                    for (int m2 = 0; m2 < M; m2++) {
                        XXtptr[bb] += XXt0ptr[bb];
                        bb++;
                    }
                }
                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer.elapsed() > 5000) || (num_timepoints_handled == N)) {
                    printf("%d/%d (%d%%)\n", num_timepoints_handled, N, (int)(num_timepoints_handled * 1.0 / N * 100));
                    timer.restart();
                }
            }
        }
    }
    if (N > 1) {
        for (int ii = 0; ii < M * M; ii++) {
            XXtptr[ii] /= (N - 1);
        }
    }

    //Mda AA = get_whitening_matrix(COV);
    Mda WW;
    whitening_matrix_from_XXt(WW, XXt); // the result is symmetric (assumed below)
    double* WWptr = WW.dataPtr();

    DiskWriteMda Y;
    Y.open(MDAIO_TYPE_FLOAT32, output, M, N);
    {
        QTime timer;
        timer.start();
        int num_timepoints_handled = 0;
#pragma omp parallel for
        for (int timepoint = 0; timepoint < N; timepoint += chunk_size) {
            Mda chunk_in;
#pragma omp critical(lock1)
            {
                X.readChunk(chunk_in, 0, timepoint, M, qMin(chunk_size, N - timepoint));
            }
            double* chunk_in_ptr = chunk_in.dataPtr();
            Mda chunk_out(M, chunk_in.N2());
            double* chunk_out_ptr = chunk_out.dataPtr();
            for (int i = 0; i < chunk_in.N2(); i++) { // explicitly do mat-mat mult ... TODO replace w/ BLAS3
                int aa = M * i;
                int bb = 0;
                for (int m1 = 0; m1 < M; m1++) {
                    for (int m2 = 0; m2 < M; m2++) {
                        chunk_out_ptr[aa + m1] += chunk_in_ptr[aa + m2] * WWptr[bb]; // actually this does dgemm w/ WW^T
                        bb++; // but since symmetric, doesn't matter.
                    }
                }
            }
#pragma omp critical(lock2)
            {
                // The following is needed to make the output deterministic, due to a very tricky floating-point problem that I honestly could not track down
                // It has something to do with multiplying by very small values of WWptr[bb]. But I truly could not pinpoint the exact problem.
                quantize(chunk_out.totalSize(), chunk_out.dataPtr(), 0.001);
                Y.writeChunk(chunk_out, 0, timepoint);
                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer.elapsed() > 5000) || (num_timepoints_handled == N)) {
                    printf("%d/%d (%d%%)\n", num_timepoints_handled, N, (int)(num_timepoints_handled * 1.0 / N * 100));
                    timer.restart();
                }
            }
        }
    }
    Y.close();

    return true;
}

double quantize(double X, double unit)
{
    return (floor(X / unit + 0.5)) * unit;
}

void quantize(int N, double* X, double unit)
{
    for (int i = 0; i < N; i++) {
        X[i] = quantize(X[i], unit);
    }
}

/*
 COV = X' * X
 We want to find W (MxM) such that
 (WX)'*(WX)=1 //finish!!
*/

/*
Mda get_whitening_matrix(Mda& COV)
{
    // return M*M mixing matrix to be applied to channels
    int M = COV.N1();

    if (COV.N2()!=M) {
        qCritical() << "incorrect dimensions in get_whitening_matrix" << COV.N1() << COV.N2();
        abort();
    }

    Mda CC,sigma;
    pca_from_XXt(CC,sigma,COV,M);
    // CC is MxK, FF is KxM (where K=M) CC is has orthogonal rows
    // COV should be equal to CC*FF, with CC'*CC = 1


    Mda U(M, M), S(1, M);
    //eigenvalue_decomposition_sym(U, S, COV); // S is list of eigenvalues
    Mda S2(M, M);
    for (int m = 0; m < M; m++) {
        if (S.get(m)) {
            S2.set(1 / sqrt(S.get(m)), m, m);
        }
    }
    Mda W = matrix_multiply(matrix_multiply(U, S2), matrix_transpose(U)); // ok for small matrices only
    return W;
}
*/
