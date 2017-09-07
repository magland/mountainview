#include "branch_cluster_v2.h"
#include "diskreadmda.h"
#include <stdio.h>
#include "get_pca_features.h"
#include <math.h>
#include "isosplit2.h"
#include <QDebug>
#include <QTime>
#include "extract_clips.h"
#include "mlcommon.h"
#include "compute_templates_0.h"
#include "get_sort_indices.h"
#include "mlcommon.h"
#include "msmisc.h"
#include "diskreadmda32.h"

struct ClipsGroup {
    Mda32* clips; //MxTxL
    QList<int> inds;
    Mda32* features2; //FxL
};

QVector<int> do_branch_cluster_v2b(ClipsGroup clips, const Branch_Cluster_V2_Opts& opts, int channel_for_display);
QVector<double> compute_peaks_v2b(ClipsGroup clips, int ch);
QVector<int> consolidate_labels_v2b(DiskReadMda32& X, const QVector<double>& times, const QVector<int>& labels, int ch, int clip_size, int detect_interval, double consolidation_factor);
QList<int> get_sort_indices_b(const QVector<int>& channels, const QVector<double>& template_peaks);
QVector<int> split_clusters(ClipsGroup clips, const QVector<int>& original_labels, const Branch_Cluster_V2_Opts& opts, int channel_for_display);

//static QMap<QString,int> s_timers;

bool branch_cluster_v2b(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const Branch_Cluster_V2_Opts& opts)
{
    printf("Starting branch_cluster_v2 --------------------\n");
    DiskReadMda32 X;
    X.setPath(timeseries_path);
    int M = X.N1();

    /*
    s_timers["pca1"]=0;
    s_timers["pca2"]=0;
    s_timers["pca3"]=0;
    s_timers["isosplit2"]=0;
    */

    DiskReadMda detect;
    detect.setPath(detect_path);
    int L = detect.N2();
    printf("#events = %d\n", L);

    Mda AM;
    if (!adjacency_matrix_path.isEmpty()) {
        AM.read(adjacency_matrix_path);
    }
    else {
        AM.allocate(M, M);
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                AM.set(1, i, j);
            }
        }
    }

    if ((AM.N1() != M) || (AM.N2() != M)) {
        printf("Error: incompatible dimensions between AM and X %ldx%ld %d, %s\n", AM.N1(), AM.N2(), M, adjacency_matrix_path.toLatin1().data());
        return false;
    }

    Mda firings0;
    firings0.allocate(3, L); //L is the max it could be

    int jjjj = 0;
    int k_offset = 0;
#pragma omp parallel for
    for (int m = 0; m < M; m++) {
        Mda32 clips;
        QVector<double> times;
#pragma omp critical
        {
            QVector<int> neighborhood;
            neighborhood << m;
            for (int a = 0; a < M; a++)
                if ((AM.value(m, a)) && (a != m))
                    neighborhood << a;
            for (int i = 0; i < L; i++) {
                if (detect.value(0, i) == (m + 1)) {
                    times << detect.value(1, i) - 1; //convert to 0-based indexing
                }
            }
            clips = extract_clips(X, times, neighborhood, opts.clip_size);
        }
        Mda32 features2;
        if (opts.num_features2)
            features2 = compute_clips_features_per_channel(clips, opts.num_features2);
        ClipsGroup clips_group;
        clips_group.clips = &clips;
        clips_group.features2 = &features2;
        for (int i = 0; i < clips.N3(); i++)
            clips_group.inds << i;
        QVector<int> labels;
        if (!clips_group.inds.isEmpty())
            labels = do_branch_cluster_v2b(clips_group, opts, m);
        if (opts.split_clusters_at_end) {
            labels = split_clusters(clips_group, labels, opts, m);
        }
#pragma omp critical
        {
            labels = consolidate_labels_v2b(X, times, labels, m, opts.clip_size, opts.detect_interval, opts.consolidation_factor);
            QVector<double> peaks = compute_peaks_v2b(clips_group, 0);

            for (int i = 0; i < times.count(); i++) {
                if (labels[i]) {
                    firings0.setValue(m + 1, 0, jjjj); //channel
                    firings0.setValue(times[i] + 1, 1, jjjj); //times //convert back to 1-based indexing
                    firings0.setValue(labels[i] + k_offset, 2, jjjj); //labels
                    //firings0.setValue(peaks[i], 3, jjjj); //peaks
                    jjjj++;
                }
            }
            k_offset += MLCompute::max<int>(labels);
        }
    }

    int L_true = jjjj;
    Mda firings;
    firings.allocate(firings0.N1(), L_true);
    for (int i = 0; i < L_true; i++) {
        for (int j = 0; j < firings0.N1(); j++) {
            firings.setValue(firings0.value(j, i), j, i);
        }
    }

    //Now reorder the labels
    int K;
    {
        printf("Reordering labels...\n");
        QVector<int> labels;
        for (int i = 0; i < L; i++) {
            int k = (int)firings.value(2, i);
            labels << k;
        }
        K = MLCompute::max<int>(labels);
        QVector<int> channels(K, 0);
        for (int i = 0; i < L; i++) {
            int k = (int)firings.value(2, i);
            if (k >= 1) {
                channels[k - 1] = (int)firings.value(0, i);
            }
        }
        int T_for_peaks = 3;
        int Tmid_for_peaks = (int)((T_for_peaks + 1) / 2) - 1;
        Mda32 templates = compute_templates_0(X, firings, T_for_peaks); //MxTxK
        QVector<double> template_peaks;
        for (int k = 0; k < K; k++) {
            if (channels[k] >= 1) {
                template_peaks << templates.value(channels[k] - 1, Tmid_for_peaks, k);
            }
            else {
                template_peaks << 0;
            }
        }
        QList<int> sort_inds = get_sort_indices_b(channels, template_peaks);
        QVector<int> label_map(K + 1, 0);
        for (int j = 0; j < sort_inds.count(); j++)
            label_map[sort_inds[j] + 1] = j + 1;
        for (int i = 0; i < L; i++) {
            int k = (int)firings.value(2, i);
            if (k >= 1) {
                k = label_map[k];
                firings.setValue(k, 2, i);
            }
        }
    }

    firings.write64(output_firings_path);

    printf("Found %d clusters and %ld events\n", K, firings.N2());

    //QStringList keys=s_timers.keys();
    //foreach (QString key,keys) {
    //    qDebug().noquote() << "TIMER" << key << s_timers[key]*1.0/1000;
    //}

    return true;
}

Mda32 compute_clips_features_per_channel(const Mda32& X, int num_features_per_channel)
{
    int M = X.N1();
    int T = X.N2();
    int L = X.N3();

    /*
    Mda32 clips_reshaped(M * T, L);
    int iii = 0;
    for (int ii = 0; ii < L; ii++) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                clips_reshaped.set(X.value(m, t, ii), iii);
                iii++;
            }
        }
    }

    Mda32 CC,FF,sigma;
    QTime timerA; timerA.start();
    pca(CC, FF, sigma, clips_reshaped, num_features);
    s_timers["pca1"]+=timerA.elapsed();
    return FF;
    */

    Mda32 FF(M * num_features_per_channel, L);
    //QTime timerA; timerA.start();
    for (int m = 0; m < M; m++) {
        Mda32 tmp0(T, L);
        for (int i = 0; i < L; i++) {
            for (int t = 0; t < T; t++) {
                tmp0.set(X.value(m, t, i), t, i);
            }
        }
        Mda32 CC, FF0, sigma;
        pca(CC, FF0, sigma, tmp0, num_features_per_channel, false); //should we subtract the mean?
        for (int i = 0; i < L; i++) {
            for (int f = 0; f < num_features_per_channel; f++) {
                FF.setValue(FF0.value(f, i), m * M + f, i);
            }
        }
    }
    //s_timers["pca1"]+=timerA.elapsed();
    return FF;
}

struct template_comparer_struct {
    int channel;
    double template_peak;
    int index;
};
struct template_comparer {
    bool operator()(const template_comparer_struct& a, const template_comparer_struct& b) const
    {
        if (a.channel < b.channel)
            return true;
        else if (a.channel == b.channel) {
            if (a.template_peak < b.template_peak)
                return true;
            else if (a.template_peak == b.template_peak)
                return (a.index < b.index);
            else
                return false;
        }
        else
            return false;
    }
};

QList<int> get_sort_indices_b(const QVector<int>& channels, const QVector<double>& template_peaks)
{
    QList<template_comparer_struct> list;
    for (int i = 0; i < channels.count(); i++) {
        template_comparer_struct tmp;
        tmp.channel = channels[i];
        tmp.template_peak = template_peaks[i];
        tmp.index = i;
        list << tmp;
    }
    qSort(list.begin(), list.end(), template_comparer());
    QList<int> ret;
    for (int i = 0; i < list.count(); i++) {
        ret << list[i].index;
    }
    return ret;
}

QVector<int> consolidate_labels_v2b(DiskReadMda32& X, const QVector<double>& times, const QVector<int>& labels, int ch, int clip_size, int detect_interval, double consolidation_factor)
{
    int M = X.N1();
    int T = clip_size;
    int K = MLCompute::max<int>(labels);
    int Tmid = (int)((T + 1) / 2) - 1;
    QVector<int> all_channels;
    for (int m = 0; m < M; m++)
        all_channels << m;
    int label_mapping[K + 1];
    label_mapping[0] = 0;
    int kk = 1;
    for (int k = 1; k <= K; k++) {
        QVector<double> times_k;
        for (int i = 0; i < times.count(); i++) {
            if (labels[i] == k)
                times_k << times[i];
        }
        Mda32 clips_k = extract_clips(X, times_k, all_channels, clip_size);
        Mda32 template_k = compute_mean_clip(clips_k);
        QVector<double> energies;
        for (int m = 0; m < M; m++)
            energies << 0;
        double max_energy = 0;
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                double val = template_k.value(m, t);
                energies[m] += val * val;
                if ((m != ch) && (energies[m] > max_energy))
                    max_energy = energies[m];
            }
        }
        //double max_energy = MLCompute::max(energies);
        bool okay = true;
        if (energies[ch] < max_energy * consolidation_factor)
            okay = false;
        double abs_peak_val = 0;
        int abs_peak_ind = 0;
        for (int t = 0; t < T; t++) {
            double value = template_k.value(ch, t);
            if (fabs(value) > abs_peak_val) {
                abs_peak_val = fabs(value);
                abs_peak_ind = t;
            }
        }
        if (fabs(abs_peak_ind - Tmid) > detect_interval) {
            okay = false;
        }
        if (okay) {
            label_mapping[k] = kk;
            kk++;
        }
        else
            label_mapping[k] = 0;
    }
    QVector<int> ret;
    for (int i = 0; i < labels.count(); i++) {
        ret << label_mapping[labels[i]];
    }
    printf("Channel %d: Using %d of %d clusters.\n", ch + 1, MLCompute::max<int>(ret), K);
    return ret;
}

QVector<double> compute_peaks_v2b(ClipsGroup clips, int ch)
{
    int T = clips.clips->N2();
    int L = clips.inds.count();
    int t0 = (T + 1) / 2 - 1;
    QVector<double> ret;
    for (int i = 0; i < L; i++) {
        ret << clips.clips->value(ch, t0, clips.inds[i]);
    }
    return ret;
}

QVector<double> compute_abs_peaks_v2b(ClipsGroup clips, int ch)
{
    int T = clips.clips->N2();
    int L = clips.inds.count();
    int t0 = (T + 1) / 2 - 1;
    QVector<double> ret;
    for (int i = 0; i < L; i++) {
        ret << fabs(clips.clips->value(ch, t0, clips.inds[i]));
    }
    return ret;
}

QVector<int> find_peaks_below_threshold_v2b(QVector<double>& peaks, double threshold)
{
    QVector<int> ret;
    for (int i = 0; i < peaks.count(); i++) {
        if (peaks[i] < threshold)
            ret << i;
    }
    return ret;
}

QVector<int> find_peaks_above_threshold_v2b(QVector<double>& peaks, double threshold)
{
    QVector<int> ret;
    for (int i = 0; i < peaks.count(); i++) {
        if (peaks[i] >= threshold)
            ret << i;
    }
    return ret;
}

QVector<int> do_cluster_without_normalized_features_b(ClipsGroup clips, const Branch_Cluster_V2_Opts& opts)
{
    QTime timer;
    timer.start();
    int M = clips.clips->N1();
    int T = clips.clips->N2();
    int L = clips.inds.count();

    Mda32 CC, FF; // CC will be MTxK, FF will be KxL
    Mda32 sigma;

    //pca(CC,FF,sigma,*clips.features2,opts.num_features2);

    if (!opts.num_features2) {
        //do this inside a block so memory gets released
        Mda32 clips_reshaped(M * T, L);
        int iii = 0;
        for (int ii = 0; ii < L; ii++) {
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    clips_reshaped.set(clips.clips->value(m, t, clips.inds[ii]), iii);
                    iii++;
                }
            }
        }

        //QTime timerA; timerA.start();
        pca(CC, FF, sigma, clips_reshaped, opts.num_features, false); //should we subtract the mean?
        //s_timers["pca2"]+=timerA.elapsed();
    }
    else {
        Mda32 features2_subset(clips.features2->N1(), clips.inds.count());
        for (int ii = 0; ii < clips.inds.count(); ii++) {
            for (int f = 0; f < clips.features2->N1(); f++) {
                features2_subset.set(clips.features2->get(f, clips.inds[ii]), f, ii);
            }
        }
        //QTime timerA; timerA.start();
        pca(CC, FF, sigma, features2_subset, opts.num_features, false); //should we subtract the mean?
        //s_timers["pca3"]+=timerA.elapsed();
    }

    //normalize_features(FF);
    //QTime timerA; timerA.start();
    QVector<int> ret = isosplit2(FF, opts.isocut_threshold);
    //s_timers["isosplit2"]+=timerA.elapsed();
    return ret;
}

QVector<double> compute_dists_from_template_b(ClipsGroup clips, Mda32& template0)
{
    int M = clips.clips->N1();
    int T = clips.clips->N2();
    int L = clips.inds.count();
    dtype32* ptr1 = clips.clips->dataPtr();
    dtype32* ptr2 = template0.dataPtr();
    QVector<double> ret;
    for (int i = 0; i < L; i++) {
        int aaa = clips.inds[i] * M * T;
        int bbb = 0;
        double sumsqr = 0;
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                double diff0 = ptr1[aaa] - ptr2[bbb];
                sumsqr += diff0 * diff0;
                aaa++;
                bbb++;
            }
        }
        ret << sqrt(sumsqr);
    }
    return ret;
}

Mda32 compute_mean_clip_b(ClipsGroup clips)
{
    int M = clips.clips->N1();
    int T = clips.clips->N2();
    int L = clips.inds.count();
    Mda32 ret;
    ret.allocate(M, T);
    for (int i = 0; i < L; i++) {
        int aaa = clips.inds[i] * M * T;
        int bbb = 0;
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(bbb) + clips.clips->get(aaa), bbb);
                aaa++;
                bbb++;
            }
        }
    }
    if (L) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(m, t) / L, m, t);
            }
        }
    }
    return ret;
}

ClipsGroup grab_clips_subset(ClipsGroup clips, const QVector<int>& inds)
{
    ClipsGroup ret;
    ret.clips = clips.clips;
    ret.features2 = clips.features2;
    for (int i = 0; i < inds.count(); i++) {
        ret.inds << clips.inds[inds[i]];
    }
    return ret;
}

QVector<int> split_clusters(ClipsGroup clips, const QVector<int>& original_labels, const Branch_Cluster_V2_Opts& opts, int channel_for_display)
{
    printf("Splitting clusters for channel %d\n", channel_for_display + 1);
    int K = MLCompute::max(original_labels);
    QVector<int> new_labels(original_labels.count());
    int k_offset = 0;
    for (int k = 1; k <= K; k++) {
        QVector<int> inds_k;
        for (int a = 0; a < original_labels.count(); a++) {
            if (original_labels[a] == k)
                inds_k << a;
        }
        ClipsGroup clips_k = grab_clips_subset(clips, inds_k);
        QVector<int> labels0 = do_cluster_without_normalized_features_b(clips_k, opts);
        int K0 = MLCompute::max(labels0);
        for (int ii = 0; ii < inds_k.count(); ii++) {
            if (labels0[ii]) {
                new_labels[inds_k[ii]] = k_offset + labels0[ii];
            }
        }
        k_offset += K0;
    }
    printf("Split clusters for channel %d: %d->%d clusters\n", channel_for_display + 1, K, MLCompute::max(new_labels));
    return new_labels;
}

QVector<int> do_branch_cluster_v2b(ClipsGroup clips, const Branch_Cluster_V2_Opts& opts, int channel_for_display)
{
    printf("do_branch_cluster_v2 %ldx%ldx%d (channel %d)\n", clips.clips->N1(), clips.clips->N2(), clips.inds.count(), channel_for_display + 1);
    int M = clips.clips->N1();
    int T = clips.clips->N2();
    int L = clips.inds.count();
    QVector<double> peaks = compute_peaks_v2b(clips, 0);
    QVector<double> abs_peaks = compute_abs_peaks_v2b(clips, 0);

    //In the case we have both positive and negative peaks, just split into two tasks!
    double min0 = MLCompute::min(peaks);
    double max0 = MLCompute::max(peaks);
    if ((min0 < 0) && (max0 >= 0)) {
        //find the event inds corresponding to negative and positive peaks
        QVector<int> inds_neg, inds_pos;
        for (int i = 0; i < L; i++) {
            if (peaks[i] < 0)
                inds_neg << i;
            else
                inds_pos << i;
        }

        //grab the negative and positive clips
        ClipsGroup clips_neg = grab_clips_subset(clips, inds_neg);
        ClipsGroup clips_pos = grab_clips_subset(clips, inds_pos);

        QVector<int> labels_neg, labels_pos;
        //cluster the negatives and positives separately
        if (!inds_neg.isEmpty()) {
            printf("Channel %d: NEGATIVES (%d)\n", channel_for_display + 1, inds_neg.count());
            labels_neg = do_branch_cluster_v2b(clips_neg, opts, channel_for_display);
        }
        if (!inds_pos.isEmpty()) {
            printf("Channel %d: POSITIVES (%d)\n", channel_for_display + 1, inds_pos.count());
            labels_pos = do_branch_cluster_v2b(clips_pos, opts, channel_for_display);
        }

        //Combine them together
        int K_neg = MLCompute::max<int>(labels_neg);
        QVector<int> labels;
        for (int i = 0; i < L; i++)
            labels << 0;
        for (int i = 0; i < inds_neg.count(); i++) {
            labels[inds_neg[i]] = labels_neg[i];
        }
        for (int i = 0; i < inds_pos.count(); i++) {
            if (labels_pos[i])
                labels[inds_pos[i]] = labels_pos[i] + K_neg;
            else
                labels[inds_pos[i]] = 0;
        }
        return labels;
    }

    //First we simply cluster all of the events
    //QVector<int> labels0=do_cluster_with_normalized_features(clips,opts);
    QTime timer;
    timer.start();
    QVector<int> labels0 = do_cluster_without_normalized_features_b(clips, opts);
    int K0 = MLCompute::max<int>(labels0);

    if (K0 > 1) {
        //if we found more than one cluster, then we should divide and conquer
        //we apply the same procedure to each cluster and then combine all of the clusters together.
        printf("Channel %d: K=%d\n", channel_for_display + 1, K0);
        QVector<int> labels;
        for (int i = 0; i < L; i++)
            labels << 0;
        int kk_offset = 0;
        for (int k = 1; k <= K0; k++) {
            QVector<int> inds_k;
            for (int a = 0; a < L; a++) {
                if (labels0[a] == k)
                    inds_k << a;
            }
            if (!inds_k.isEmpty()) {
                ClipsGroup clips_k = grab_clips_subset(clips, inds_k);
                QVector<int> labels_k = do_branch_cluster_v2b(clips_k, opts, channel_for_display);
                for (int a = 0; a < inds_k.count(); a++) {
                    labels[inds_k[a]] = labels_k[a] + kk_offset;
                }
                kk_offset += MLCompute::max<int>(labels_k);
            }
        }
        return labels;
    }
    else {
        if (opts.shell_increment == 0) {
            //in this case we are not going to increment at all
            QVector<int> labels;
            for (int i = 0; i < L; i++)
                labels << 1;
            return labels;
        }

        //otherwise, we have only one cluster
        //so we need to increase the threshold to see if we can get things to split at higher amplitude
        double abs_peak_threshold = 0;
        double max_abs_peak = MLCompute::max(abs_peaks);

        //increase abs_peak_threshold by opts.shell_increment until we have at least opts.min_shell_size below and above the threshold
        while (true) {
            QVector<int> inds_below = find_peaks_below_threshold_v2b(abs_peaks, abs_peak_threshold);
            if ((inds_below.count() >= opts.min_shell_size) && (L - inds_below.count() >= opts.min_shell_size)) {
                break;
            }
            if (abs_peak_threshold > max_abs_peak) {
                break;
            }
            abs_peak_threshold += opts.shell_increment;
        }
        if (abs_peak_threshold > max_abs_peak) {
            //we couldn't split it. So fine, we'll just say there is only one cluster
            QVector<int> labels;
            for (int i = 0; i < L; i++)
                labels << 1;
            return labels;
        }
        else {
            //we now split things into two categories based on abs_peak_threshold
            QVector<int> inds_below = find_peaks_below_threshold_v2b(abs_peaks, abs_peak_threshold);
            QVector<int> inds_above = find_peaks_above_threshold_v2b(abs_peaks, abs_peak_threshold);
            ClipsGroup clips_above = grab_clips_subset(clips, inds_above);
            ClipsGroup clips_below = grab_clips_subset(clips, inds_below);

            //Apply the procedure to the events above the threshold
            QVector<int> labels_above;
            if (!inds_above.isEmpty())
                labels_above = do_branch_cluster_v2b(clips_above, opts, channel_for_display);
            int K_above = MLCompute::max<int>(labels_above);

            if (K_above <= 1) {
                //there is really only one cluster
                QVector<int> labels;
                for (int i = 0; i < L; i++)
                    labels << 1;
                return labels;
            }
            else {
                //there is more than one cluster. Let's divide up the based on the nearest
                //let's consider only the next shell above
                QVector<double> abs_peaks_above;
                for (int i = 0; i < inds_above.count(); i++)
                    abs_peaks_above << abs_peaks[inds_above[i]];
                QVector<int> inds_next_shell = find_peaks_below_threshold_v2b(abs_peaks_above, abs_peak_threshold + opts.shell_increment);
                ClipsGroup clips_next_shell = grab_clips_subset(clips_above, inds_next_shell);
                QVector<int> labels_next_shell;
                for (int i = 0; i < inds_next_shell.count(); i++)
                    labels_next_shell << labels_above[inds_next_shell[i]];

                //compute the centroids for the next shell above
                Mda32 centroids;
                centroids.allocate(M, T, K_above);
                for (int kk = 1; kk <= K_above; kk++) {
                    QVector<int> inds_kk;
                    for (int i = 0; i < labels_next_shell.count(); i++) {
                        if (labels_next_shell[i] == kk)
                            inds_kk << i;
                    }
                    ClipsGroup clips_kk = grab_clips_subset(clips_next_shell, inds_kk);
                    Mda32 centroid0 = compute_mean_clip_b(clips_kk);
                    for (int t = 0; t < T; t++) {
                        for (int m = 0; m < M; m++) {
                            centroids.setValue(centroid0.value(m, t), m, t, kk - 1);
                        }
                    }
                }

                //set the labels for all of the inds above
                QVector<int> labels;
                for (int i = 0; i < L; i++)
                    labels << 0;
                for (int i = 0; i < inds_above.count(); i++) {
                    labels[inds_above[i]] = labels_above[i];
                }

                //for the events below, compute the distances to all the centroids of the next shell above
                Mda distances;
                distances.allocate(inds_below.count(), K_above);
                for (int k = 1; k <= K_above; k++) {
                    QVector<int> tmp;
                    tmp << k - 1;
                    Mda32 centroid0 = grab_clips_subset(centroids, tmp);
                    QVector<double> dists = compute_dists_from_template_b(clips_below, centroid0);
                    for (int i = 0; i < inds_below.count(); i++) {
                        distances.setValue(dists[i], i, k - 1);
                    }
                }

                //label the events below based on distance to threshold
                for (int i = 0; i < inds_below.count(); i++) {
                    int best_k = 0;
                    double best_dist = distances.value(i, 0L);
                    for (int k = 0; k < K_above; k++) {
                        double dist0 = distances.value(i, k);
                        if (dist0 < best_dist) {
                            best_dist = dist0;
                            best_k = k;
                        }
                    }
                    labels[inds_below[i]] = best_k + 1; //convert back to 1-based indexing
                }
                return labels;
            }
        }
    }
}
