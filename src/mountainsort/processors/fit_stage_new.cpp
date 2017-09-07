/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/21/2016
*******************************************************/

#include "fit_stage.h"
#include <QList>
#include "mlcommon.h"
#include "mlcommon.h"
#include "diskreadmda.h"
#include <QTime>
#include <math.h>
#include "compute_templates_0.h"
#include "compute_detectability_scores.h"
#include "get_sort_indices.h"
#include "msprefs.h"
#include "omp.h"

QList<int> fit_stage_kernel(Mda& X, Mda& templates, QVector<double>& times, QVector<int>& labels, const fit_stage_opts& opts);
QList<int> fit_stage_kernel_old(Mda& X, Mda& templates, QVector<double>& times, QVector<int>& labels, const fit_stage_opts& opts);

bool fit_stage_new(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const fit_stage_opts& opts)
{
    //Just for timing things
    QTime timer_total;
    timer_total.start();
    QMap<QString, int> elapsed_times;

    //The timeseries data and the dimensions
    DiskReadMda X(timeseries_path);
    int M = X.N1();
    int N = X.N2();
    int T = opts.clip_size;

    //Read in the firings array
    Mda firingsA;
    firingsA.read(firings_path);
    Mda firings = sort_firings_by_time(firingsA);

    //These are the options for splitting into shells
    Define_Shells_Opts define_shells_opts;
    define_shells_opts.min_shell_size = opts.min_shell_size;
    define_shells_opts.shell_increment = opts.shell_increment;

    //Here we split into shells to handle amplitude variation
    Mda firings_split = split_into_shells(firings, define_shells_opts);

    //These are the templates corresponding to the sub-clusters (after shell splitting)
    Mda templates = compute_templates_0(X, firings_split, T); //MxTxK (wrong: MxNxK)

    //L is the number of events. Accumulate vectors of times and labels for convenience
    int L = firings.N2();
    QVector<double> times;
    QVector<int> labels;
    for (int j = 0; j < L; j++) {
        times << firings.value(1, j);
        labels << (int)firings_split.value(2, j); //these are labels of sub-clusters
    }

    //Now we do the processing in chunks
    int chunk_size = PROCESSING_CHUNK_SIZE;
    int overlap_size = PROCESSING_CHUNK_OVERLAP_SIZE;
    if (N < PROCESSING_CHUNK_SIZE) {
        chunk_size = N;
        overlap_size = 0;
    }

    QList<int> inds_to_use;

    printf("Starting fit stage\n");
    {
        QTime timer_status;
        timer_status.start();
        int num_timepoints_handled = 0;
#pragma omp parallel for
        for (int timepoint = 0; timepoint < N; timepoint += chunk_size) {
            QMap<QString, int> elapsed_times_local;
            Mda chunk; //this will be the chunk we are working on
            Mda local_templates; //just a local copy of the templates
            QVector<double> local_times; //the times that fall in this time range
            QVector<int> local_labels; //the corresponding labels
            QList<int> local_inds; //the corresponding event indices
            fit_stage_opts local_opts;
#pragma omp critical(lock1)
            {
                //build the variables above
                QTime timer;
                timer.start();
                local_templates = templates;
                local_opts = opts;
                X.readChunk(chunk, 0, timepoint - overlap_size, M, chunk_size + 2 * overlap_size);
                elapsed_times["readChunk"] += timer.elapsed();
                timer.start();
                for (int jj = 0; jj < L; jj++) {
                    if ((timepoint - overlap_size <= times[jj]) && (times[jj] < timepoint - overlap_size + chunk_size + 2 * overlap_size)) {
                        local_times << times[jj] - (timepoint - overlap_size);
                        local_labels << labels[jj];
                        local_inds << jj;
                    }
                }
                elapsed_times["set_local_data"] += timer.elapsed();
            }
            //Our real task is to decide which of these events to keep. Those will be stored in local_inds_to_use
            //"Local" means this chunk in this thread
            QList<int> local_inds_to_use;
            {
                QTime timer;
                timer.start();
                //This is the main kernel operation!!
                local_inds_to_use = fit_stage_kernel(chunk, local_templates, local_times, local_labels, local_opts);
                elapsed_times_local["fit_stage_kernel"] += timer.elapsed();
            }
#pragma omp critical(lock1)
            {
                elapsed_times["fit_stage_kernel"] += elapsed_times_local["fit_stage_kernel"];
                {
                    QTime timer;
                    timer.start();
                    for (int ii = 0; ii < local_inds_to_use.count(); ii++) {
                        int ind0 = local_inds[local_inds_to_use[ii]];
                        double t0 = times[ind0];
                        if ((timepoint <= t0) && (t0 < timepoint + chunk_size)) {
                            inds_to_use << ind0;
                        }
                    }
                    elapsed_times_local["get_local_data"] += timer.elapsed();
                }

                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer_status.elapsed() > 5000) || (num_timepoints_handled == N) || (timepoint == 0)) {
                    printf("%d/%d (%d%%) - Elapsed(s): RC:%g, SLD:%g, KERNEL:%g, GLD:%g, Total:%g, %d threads\n",
                        num_timepoints_handled, N,
                        (int)(num_timepoints_handled * 1.0 / N * 100),
                        elapsed_times["readChunk"] * 1.0 / 1000,
                        elapsed_times["set_local_data"] * 1.0 / 1000,
                        elapsed_times["fit_stage_kernel"] * 1.0 / 1000,
                        elapsed_times["get_local_data"] * 1.0 / 1000,
                        timer_total.elapsed() * 1.0 / 1000,
                        omp_get_num_threads());
                    timer_status.restart();
                }
            }
        }
    }

    qSort(inds_to_use);
    int num_to_use = inds_to_use.count();
    if (times.count()) {
        printf("using %d/%d events (%g%%)\n", num_to_use, (int)times.count(), num_to_use * 100.0 / times.count());
    }
    Mda firings_out(firings.N1(), num_to_use);
    for (int i = 0; i < num_to_use; i++) {
        for (int j = 0; j < firings.N1(); j++) {
            firings_out.set(firings.get(j, inds_to_use[i]), j, i);
        }
    }

    firings_out.write64(firings_out_path);

    return true;
}

typedef QList<int> IntList;

QList<int> get_channel_mask(Mda template0, int num)
{
    int M = template0.N1();
    int T = template0.N2();
    QVector<double> maxabs;
    for (int m = 0; m < M; m++) {
        double val = 0;
        for (int t = 0; t < T; t++) {
            val = qMax(val, qAbs(template0.value(m, t)));
        }
        maxabs << val;
    }
    QList<int> inds = get_sort_indices(maxabs);
    QList<int> ret;
    for (int i = 0; i < num; i++) {
        if (i < inds.count()) {
            ret << inds[inds.count() - 1 - i];
        }
    }
    return ret;
}

bool is_dirty(const Mda& dirty, int t0, const QList<int>& chmask)
{
    for (int i = 0; i < chmask.count(); i++) {
        if (dirty.value(chmask[i], t0))
            return true;
    }
    return false;
}

double compute_score(int N, double* X, double* template0)
{
    Mda resid(1, N);
    double* resid_ptr = resid.dataPtr();
    for (int i = 0; i < N; i++)
        resid_ptr[i] = X[i] - template0[i];
    double norm1 = MLCompute::norm(N, X);
    double norm2 = MLCompute::norm(N, resid_ptr);
    return norm1 * norm1 - norm2 * norm2;
}

double compute_score(int M, int T, double* X, double* template0, const QList<int>& chmask)
{
    double before_sumsqr = 0;
    double after_sumsqr = 0;
    for (int t = 0; t < T; t++) {
        for (int i = 0; i < chmask.count(); i++) {
            int m = chmask[i];
            double val = X[m + t * M];
            before_sumsqr += val * val;
            val -= template0[m + t * M];
            after_sumsqr += val * val;
        }
    }

    return before_sumsqr - after_sumsqr;
}

void subtract_scaled_template(int N, double* X, double* template0)
{
    double S12 = 0, S22 = 0;
    for (int i = 0; i < N; i++) {
        S22 += template0[i] * template0[i];
        S12 += X[i] * template0[i];
    }
    double alpha = 1;
    if (S22)
        alpha = S12 / S22;
    for (int i = 0; i < N; i++) {
        X[i] -= alpha * template0[i];
    }
}

void subtract_scaled_template(int M, int T, double* X, double* template0, const QList<int>& chmask)
{
    double S12 = 0, S22 = 0;
    for (int t = 0; t < T; t++) {
        for (int j = 0; j < chmask.count(); j++) {
            int m = chmask[j];
            int i = m + M * t;
            S22 += template0[i] * template0[i];
            S12 += X[i] * template0[i];
        }
    }
    double alpha = 1;
    if (S22)
        alpha = S12 / S22;
    for (int t = 0; t < T; t++) {
        for (int j = 0; j < chmask.count(); j++) {
            int m = chmask[j];
            int i = m + M * t;
            X[i] -= alpha * template0[i];
        }
    }
}

QList<int> fit_stage_kernel(Mda& X, Mda& templates, QVector<double>& times, QVector<int>& labels, const fit_stage_opts& opts)
{
    int M = X.N1(); //the number of dimensions
    int T = opts.clip_size; //the clip size
    int Tmid = (int)((T + 1) / 2) - 1; //the center timepoint in a clip (zero-indexed)
    int L = times.count(); //number of events we are looking at
    int K = MLCompute::max<int>(labels); //the maximum label number

    //compute the L2-norms of the templates ahead of time
    QVector<double> template_norms;
    template_norms << 0;
    for (int k = 1; k <= K; k++) {
        template_norms << MLCompute::norm(M * T, templates.dataPtr(0, 0, k - 1));
    }

    //keep passing through the data until nothing changes anymore
    bool something_changed = true;
    QVector<int> all_to_use; //a vector of 0's and 1's telling which events should be used
    for (int i = 0; i < L; i++)
        all_to_use << 0; //start out using none
    int num_passes = 0;
    //while ((something_changed)&&(num_passes<2)) {

    QVector<double> scores(L);
    Mda dirty(X.N1(), X.N2()); //timepoints/channels for which we need to recompute the score if a clip was centered here
    for (int ii = 0; ii < dirty.totalSize(); ii++) {
        dirty.setValue(1, ii);
    }

    QList<IntList> channel_mask;
    for (int i = 0; i < K; i++) {
        Mda template0;
        templates.getChunk(template0, 0, 0, i, M, T, 1);
        channel_mask << get_channel_mask(template0, 8); //use only the 8 channels with highest maxval
    }

    while (something_changed) {
        QTime timer0;
        timer0.start();
        num_passes++;
        QVector<double> scores_to_try;
        QVector<double> times_to_try;
        QVector<int> labels_to_try;
        QList<int> inds_to_try; //indices of the events to try on this pass
        //QVector<double> template_norms_to_try;
        for (int i = 0; i < L; i++) { //loop through the events
            if (all_to_use[i] == 0) { //if we are not yet using it...
                double t0 = times[i];
                int k0 = labels[i];
                if (k0 > 0) { //make sure we have a positive label (don't know why we wouldn't)
                    int tt = (int)(t0 - Tmid + 0.5); //start time of clip
                    double score0 = 0;
                    IntList chmask = channel_mask[k0 - 1];
                    if (!is_dirty(dirty, t0, chmask)) {
                        // we don't need to recompute the score
                        score0 = scores[i];
                    }
                    else {
                        //we do need to recompute it.
                        if ((tt >= 0) && (tt + T <= X.N2())) { //make sure we are in range
                            //The score will be how much something like the L2-norm is decreased
                            score0 = compute_score(M, T, X.dataPtr(0, tt), templates.dataPtr(0, 0, k0 - 1), chmask);
                        }
                        /*
                        if (score0 < template_norms[k0] * template_norms[k0] * 0.1)
                            score0 = 0; //the norm of the improvement needs to be at least 0.5 times the norm of the template
                            */
                    }

                    //the score needs to be at least as large as neglogprior in order to accept the spike
                    double neglogprior = 30;
                    if (score0 > neglogprior) {
                        //we are not committing to using this event yet... see below for next step
                        scores_to_try << score0;
                        times_to_try << t0;
                        labels_to_try << k0;
                        inds_to_try << i;
                    }
                    else {
                        //means we definitely aren't using it (so we will never get here again)
                        all_to_use[i] = -1; //signals not to try again
                    }
                }
            }
        }
        //Look at those events to try and see if we should use them
        QVector<int> to_use = find_events_to_use(times_to_try, scores_to_try, opts);

        //at this point, nothing is dirty
        for (int i = 0; i < dirty.totalSize(); i++) {
            dirty.set(0, i);
        }

        //for all those we are going to "use", we want to subtract out the corresponding templates from the timeseries data
        something_changed = false;
        int num_added = 0;
        for (int i = 0; i < to_use.count(); i++) {
            if (to_use[i] == 1) {
                IntList chmask = channel_mask[labels_to_try[i] - 1];
                something_changed = true;
                num_added++;
                bigint tt = (bigint)(times_to_try[i] - Tmid + 0.5);
                subtract_scaled_template(M, T, X.dataPtr(0, tt), templates.dataPtr(0, 0, labels_to_try[i] - 1), chmask);
                for (int aa = tt - T / 2 - 1; aa <= tt + T + T / 2 + 1; aa++) {
                    if ((aa >= 0) && (aa < X.N2())) {
                        for (int k = 0; k < chmask.count(); k++) {
                            dirty.setValue(1, chmask[k], aa);
                        }
                    }
                }
                all_to_use[inds_to_try[i]] = 1;
            }
        }
        //printf("pass %d: added %d events\n", num_passes, num_added);
        //printf("(%d: %d) ",num_added, timer0.elapsed());
    }

    QList<int> inds_to_use;
    for (int i = 0; i < L; i++) {
        if (all_to_use[i] == 1)
            inds_to_use << i;
    }

    return inds_to_use;
}

QList<int> fit_stage_kernel_old(Mda& X, Mda& templates, QVector<double>& times, QVector<int>& labels, const fit_stage_opts& opts)
{
    int M = X.N1(); //the number of dimensions
    int T = opts.clip_size; //the clip size
    int Tmid = (int)((T + 1) / 2) - 1; //the center timepoint in a clip (zero-indexed)
    int L = times.count(); //number of events we are looking at
    int K = MLCompute::max<int>(labels); //the maximum label number

    //compute the L2-norms of the templates ahead of time
    QVector<double> template_norms;
    template_norms << 0;
    for (int k = 1; k <= K; k++) {
        template_norms << MLCompute::norm(M * T, templates.dataPtr(0, 0, k - 1));
    }

    //keep passing through the data until nothing changes anymore
    bool something_changed = true;
    QVector<int> all_to_use; //a vector of 0's and 1's telling which events should be used
    for (int i = 0; i < L; i++)
        all_to_use << 0; //start out using none
    int num_passes = 0;
    //while ((something_changed)&&(num_passes<2)) {

    while (something_changed) {
        num_passes++;
        QVector<double> scores_to_try;
        QVector<double> times_to_try;
        QVector<int> labels_to_try;
        QList<int> inds_to_try; //indices of the events to try on this pass
        //QVector<double> template_norms_to_try;
        for (int i = 0; i < L; i++) { //loop through the events
            if (all_to_use[i] == 0) { //if we are not yet using it...
                double t0 = times[i];
                int k0 = labels[i];
                if (k0 > 0) { //make sure we have a positive label (don't know why we wouldn't)
                    int tt = (int)(t0 - Tmid + 0.5); //start time of clip
                    double score0 = 0;
                    if ((tt >= 0) && (tt + T <= X.N2())) { //make sure we are in range
                        //The score will be how much something like the L2-norm is decreased
                        score0 = compute_score(M * T, X.dataPtr(0, tt), templates.dataPtr(0, 0, k0 - 1));
                    }
                    /*
                    if (score0 < template_norms[k0] * template_norms[k0] * 0.1)
                        score0 = 0; //the norm of the improvement needs to be at least 0.5 times the norm of the template
                        */

                    //the score needs to be at least as large as neglogprior in order to accept the spike
                    double neglogprior = 30;
                    if (score0 > neglogprior) {
                        //we are not committing to using this event yet... see below for next step
                        scores_to_try << score0;
                        times_to_try << t0;
                        labels_to_try << k0;
                        inds_to_try << i;
                    }
                    else {
                        //means we definitely aren't using it (so we will never get here again)
                        all_to_use[i] = -1; //signals not to try again
                    }
                }
            }
        }
        //Look at those events to try and see if we should use them
        QVector<int> to_use = find_events_to_use(times_to_try, scores_to_try, opts);

        //for all those we are going to "use", we want to subtract out the corresponding templates from the timeseries data
        something_changed = false;
        int num_added = 0;
        for (int i = 0; i < to_use.count(); i++) {
            if (to_use[i] == 1) {
                something_changed = true;
                num_added++;
                bigint tt = (bigint)(times_to_try[i] - Tmid + 0.5);
                subtract_scaled_template(M * T, X.dataPtr(0, tt), templates.dataPtr(0, 0, labels_to_try[i] - 1));
                all_to_use[inds_to_try[i]] = 1;
            }
        }
        //printf("pass %d: added %d events\n", num_passes, num_added);
        //printf("(%d) ",num_added);
    }

    QList<int> inds_to_use;
    for (int i = 0; i < L; i++) {
        if (all_to_use[i] == 1)
            inds_to_use << i;
    }

    return inds_to_use;
}
