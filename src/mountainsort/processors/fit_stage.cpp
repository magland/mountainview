/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#include "fit_stage.h"
#include <QList>
#include "mlcommon.h"
#include "mlcommon.h"
#include "diskreadmda.h"
#include <math.h>
#include "compute_templates_0.h"
#include "compute_detectability_scores.h"
#include "get_sort_indices.h"

/*
bool fit_stage(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const fit_stage_opts& opts)
{
    Mda X(timeseries_path);
    Mda firingsA;
    firingsA.read(firings_path);
    Mda firings = sort_firings_by_time(firingsA);

    int M = X.N1();
    int T = opts.clip_size;
    int Tmid = (int)((T + 1) / 2) - 1;
    int L = firings.N2();

    Define_Shells_Opts define_shells_opts;
    define_shells_opts.min_shell_size = opts.min_shell_size;
    define_shells_opts.shell_increment = opts.shell_increment;
    Mda firings_split = split_into_shells(firings, define_shells_opts);
    //Mda firings_split = firings;

    QVector<double> times;
    QVector<int> labels;
    for (int i = 0; i < L; i++) {
        times << firings_split.value(1, i);
        labels << (int)firings_split.value(2, i);
    }
    int K = MLCompute::max<int>(labels);

    DiskReadMda X0(timeseries_path);
    Mda templates = compute_templates_0(X0, firings_split, T); //MxNxK

    QVector<double> template_norms;
    template_norms << 0;
    for (int k = 1; k <= K; k++) {
        template_norms << MLCompute::norm(M * T, templates.dataPtr(0, 0, k - 1));
    }

    bool something_changed = true;
    QVector<int> all_to_use;
    for (int i = 0; i < L; i++)
        all_to_use << 0;
    int num_passes = 0;
    //while ((something_changed)&&(num_passes<2)) {
    while (something_changed) {
        num_passes++;
        printf("pass %d... ", num_passes);
        QVector<double> scores_to_try;
        QVector<double> times_to_try;
        QVector<int> labels_to_try;
        QList<int> inds_to_try; //indices of the events to try on this pass
        //QVector<double> template_norms_to_try;
        for (int i = 0; i < L; i++) {
            if (all_to_use[i] == 0) {
                double t0 = times[i];
                int k0 = labels[i];
                if (k0 > 0) {
                    int tt = (int)(t0 - Tmid + 0.5);
                    double score0 = compute_score(M, T, X.dataPtr(0, tt), templates.dataPtr(0, 0, k0 - 1), );

                    double neglogprior = opts.neglogprior;
                    if (score0 > neglogprior) {
                        scores_to_try << score0;
                        times_to_try << t0;
                        labels_to_try << k0;
                        inds_to_try << i;
                    }
                    else {
                        all_to_use[i] = -1; //means we definitely aren't using it (so we will never get here again)
                    }
                }
            }
        }
        QVector<int> to_use = find_events_to_use(times_to_try, scores_to_try, opts);

        {
            X.write32(QString("/tmp/debug-X-%1.mda").arg(num_passes));
            QList<int> debug_inds;
            for (int i = 0; i < to_use.count(); i++) {
                if (to_use[i] == 1) {
                    debug_inds << inds_to_try[i];
                }
            }
            Mda debug_firings(firings.N1(), debug_inds.count());
            for (int a = 0; a < debug_inds.count(); a++) {
                for (int b = 0; b < firings.N1(); b++) {
                    debug_firings.setValue(firings.value(b, debug_inds[a]), b, a);
                }
            }
            debug_firings.write64(QString("/tmp/debug-firings-%1.mda").arg(num_passes));
        }

        something_changed = false;
        int num_added = 0;
        for (int i = 0; i < to_use.count(); i++) {
            if (to_use[i] == 1) {
                something_changed = true;
                num_added++;
                int tt = (int)(times_to_try[i] - Tmid + 0.5);
                subtract_scaled_template(M * T, X.dataPtr(0, tt), templates.dataPtr(0, 0, labels_to_try[i] - 1));
                all_to_use[inds_to_try[i]] = 1;
            }
        }
        printf("added %d events\n", num_added);
    }

    int num_to_use = 0;
    for (int i = 0; i < L; i++) {
        if (all_to_use[i] == 1)
            num_to_use++;
    }
    if (times.count()) {
        printf("using %d/%d events (%g%%)\n", num_to_use, (int)times.count(), num_to_use * 100.0 / times.count());
    }
    Mda firings_out(firings.N1(), num_to_use);
    int aa = 0;
    for (int i = 0; i < L; i++) {
        if (all_to_use[i] == 1) {
            for (int j = 0; j < firings.N1(); j++) {
                firings_out.set(firings.get(j, i), j, aa);
            }
            aa++;
        }
    }

    firings_out.write64(firings_out_path);

    return true;
}
*/

QVector<int> find_events_to_use(const QVector<double>& times, const QVector<double>& scores, const fit_stage_opts& opts)
{
    QVector<int> to_use;
    int L = times.count();
    for (int i = 0; i < L; i++)
        to_use << 0; //start out not using any
    for (int i = 0; i < L; i++) {
        if (scores[i] > 0) { //score has to at least be positive
            to_use[i] = 1; //for now we say we are using it
            {
                // but let's check nearby events that may have a larger score
                int j = i;
                while ((j >= 0) && (times[j] >= times[i] - opts.clip_size)) {
                    if ((i != j) && (scores[j] >= scores[i]))
                        to_use[i] = 0; //actually not using it because there is something bigger to the left
                    j--;
                }
            }
            {
                int j = i;
                while ((j < times.count()) && (times[j] <= times[i] + opts.clip_size)) {
                    if (scores[j] > scores[i])
                        to_use[i] = 0; //actually not using it because there is something bigger to the right
                    j++;
                }
            }
        }
    }
    return to_use;
}

Mda split_into_shells(const Mda& firings, Define_Shells_Opts opts)
{
    if (!opts.shell_increment)
        return firings;
    //The input is firings the output is the same except with new labels corresponding to subclusters
    QVector<int> labels, labels_new;
    for (int j = 0; j < firings.N2(); j++) {
        labels << (int)firings.value(2, j); //the old labels extracted out of the original firings array
        labels_new << 0; //the new labels (start with all zeros)
    }
    int K = MLCompute::max<int>(labels); //K is the max original label
    int k2 = 1; //this is the next sub-cluster label we will use
    for (int k = 1; k <= K; k++) {
        QVector<int> inds_k = find_label_inds(labels, k); //find all event indices corresponding to label k
        QVector<double> peaks; //make a vector of the peaks corresponding to the events in this (original) cluster k
        for (int j = 0; j < inds_k.count(); j++) {
            peaks << firings.value(3, inds_k[j]);
        }
        //define the shells (based on amplitude and #constraints of the opts)
        QList<Shell> shells = define_shells(peaks, opts);
        //step through the shells
        for (int a = 0; a < shells.count(); a++) {
            QList<int> s_inds = shells[a].inds; //get the inds associated with this shell
            for (int b = 0; b < s_inds.count(); b++) {
                labels_new[inds_k[s_inds[b]]] = k2; //set the sub-cluster label
            }
            k2++;
        }
    }

    Mda firings_ret = firings;
    for (int j = 0; j < firings.N2(); j++) {
        firings_ret.setValue(labels_new[j], 2, j);
    }
    return firings_ret;
}

Mda sort_firings_by_time(const Mda& firings)
{
    QVector<double> times;
    for (int i = 0; i < firings.N2(); i++) {
        times << firings.value(1, i);
    }
    QList<int> sort_inds = get_sort_indices(times);

    Mda F(firings.N1(), firings.N2());
    for (int i = 0; i < firings.N2(); i++) {
        for (int j = 0; j < firings.N1(); j++) {
            F.setValue(firings.value(j, sort_inds[i]), j, i);
        }
    }

    return F;
}
