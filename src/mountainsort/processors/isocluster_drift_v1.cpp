#include "isocluster_drift_v1.h"
#include "diskreadmda.h"
#include <stdio.h>
#include "get_pca_features.h"
#include <math.h>
#include "isosplit2.h"
#include <QDebug>
#include <QTime>
#include <diskwritemda.h>
#include "extract_clips.h"
#include "mlcommon.h"
#include "compute_templates_0.h"
#include "get_sort_indices.h"
#include "mlcommon.h"
#include "msmisc.h"
#include "diskreadmda32.h"
#include "isosplit5.h"
#include <QFile>

namespace Isocluster_drift_v1 {

struct ClipsGroup {
    Mda32* clips; //MxTxL
    QList<int> inds;
    Mda32* features2; //FxL
};

Mda32 compute_clips_features_per_channel(const Mda32& clips, const QVector<int>& inds, int num_features_per_channel)
{
    int M = clips.N1();
    int T = clips.N2();
    //int L = clips.N3();
    int LL = inds.count();

    Mda32 FF(M * num_features_per_channel, LL);

    for (int m = 0; m < M; m++) {
        Mda32 tmp0(T, LL);
        for (int i = 0; i < LL; i++) {
            for (int t = 0; t < T; t++) {
                tmp0.set(clips.value(m, t, inds[i]), t, i);
            }
        }
        Mda32 CC, FF0, sigma;
        pca(CC, FF0, sigma, tmp0, num_features_per_channel, false); //should we subtract the mean?
        for (int i = 0; i < LL; i++) {
            for (int f = 0; f < num_features_per_channel; f++) {
                FF.setValue(FF0.value(f, i), m * M + f, i);
            }
        }
    }
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

QVector<int> consolidate_labels(const DiskReadMda32& X, const QVector<double>& times, const QVector<int>& labels, int ch, int clip_size, int detect_interval, double consolidation_factor)
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
    //printf("Channel %d: Using %d of %d clusters.\n", ch + 1, MLCompute::max<int>(ret), K);
    return ret;
}

QVector<double> compute_peaks(const Mda32& clips, int ch)
{
    int T = clips.N2();
    int L = clips.N3();
    int t0 = (T + 1) / 2 - 1;
    QVector<double> ret;
    for (int i = 0; i < L; i++) {
        ret << clips.value(ch, t0, i);
    }
    return ret;
}

/*
QVector<double> compute_abs_peaks(ClipsGroup clips, int ch)
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
*/

QVector<int> find_peaks_below_threshold(QVector<double>& peaks, double threshold)
{
    QVector<int> ret;
    for (int i = 0; i < peaks.count(); i++) {
        if (peaks[i] < threshold)
            ret << i;
    }
    return ret;
}

QVector<int> find_peaks_above_threshold(QVector<double>& peaks, double threshold)
{
    QVector<int> ret;
    for (int i = 0; i < peaks.count(); i++) {
        if (peaks[i] >= threshold)
            ret << i;
    }
    return ret;
}

QVector<int> do_cluster_2(const Mda32& clips, const QVector<int>& inds, const isocluster_drift_v1_opts& opts)
{
    int M = clips.N1();
    int T = clips.N2();
    //int L = clips.N3();
    int LL = inds.count();

    Mda32 CC, FF; // CC will be MTxK, FF will be KxL
    Mda32 sigma;

    //pca(CC,FF,sigma,*clips.features2,opts.num_features2);

    if (!opts.num_features2) {
        //do this inside a block so memory gets released
        Mda32 clips_reshaped(M * T, LL);
        int iii = 0;
        for (int ii = 0; ii < LL; ii++) {
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    clips_reshaped.setValue(clips.value(m, t, inds[ii]), iii);
                    iii++;
                }
            }
        }

        pca(CC, FF, sigma, clips_reshaped, opts.num_features, false); //should we subtract the mean?
    }
    else {
        Mda32 features2 = compute_clips_features_per_channel(clips, inds, opts.num_features2);
        pca(CC, FF, sigma, features2, opts.num_features, false); //should we subtract the mean?
    }

    isosplit5_opts oo5;
    oo5.isocut_threshold = opts.isocut_threshold;
    oo5.K_init = opts.K_init;
    oo5.refine_clusters = false;
    QVector<int> labels(FF.N2());
    isosplit5(labels.data(), FF.N1(), FF.N2(), FF.dataPtr(), oo5);

    // Check for further splits while re-computing the PCA features (branch method)
    int K = MLCompute::max(labels);
    //if (K>1) {
    if (0) {
        // More than one cluster, so we proceed with the branch method
        QVector<int> new_labels(LL);
        for (int jj = 0; jj < LL; jj++)
            new_labels[jj] = 0;
        int k_offset = 0;
        for (int k = 1; k <= K; k++) {
            QVector<int> inds_k, inds_k_b;
            for (int i = 0; i < LL; i++) {
                if (labels[i] == k) {
                    inds_k << inds[i];
                    inds_k_b << i;
                }
            }
            if (!inds_k.isEmpty()) {
                QVector<int> labels_k = do_cluster_2(clips, inds_k, opts);
                for (int jj = 0; jj < inds_k.count(); jj++) {
                    new_labels[inds_k_b[jj]] = k_offset + labels_k[jj];
                }
                k_offset += MLCompute::max(labels_k);
            }
        }
        return new_labels;
    }
    else {
        // Only one cluster, we are done.
        return labels;
    }
}

QVector<int> do_cluster(const Mda32& clips, const isocluster_drift_v1_opts& opts)
{
    //int M = clips.N1();
    //int T = clips.N2();
    int L = clips.N3();

    //split into positive and negative peaks
    QVector<double> peaks = compute_peaks(clips, 0);
    //double min0 = MLCompute::min(peaks);
    //double max0 = MLCompute::max(peaks);

    //find the event inds corresponding to negative and positive peaks
    QVector<int> inds_neg, inds_pos;
    for (int i = 0; i < L; i++) {
        if (peaks[i] < 0)
            inds_neg << i;
        else
            inds_pos << i;
    }

    QVector<int> labels_neg, labels_pos;
    //cluster the negatives and positives separately
    if (!inds_neg.isEmpty()) {
        labels_neg = do_cluster_2(clips, inds_neg, opts);
    }
    if (!inds_pos.isEmpty()) {
        labels_pos = do_cluster_2(clips, inds_pos, opts);
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
            labels[inds_pos[i]] = 0; //make sure we don't assign 0 -> K_neg+0
    }
    return labels;
}

QVector<int> remove_empty_labels(const QVector<int>& labels)
{
    int K = MLCompute::max(labels);
    QVector<int> counts(K + 1);
    for (int i = 0; i < labels.count(); i++) {
        counts[labels[i]]++;
    }
    QVector<int> label_map(K + 1);
    label_map[0] = 0;
    int kk = 1;
    for (int i = 1; i < counts.count(); i++) {
        if (counts[i]) {
            label_map[i] = kk;
            kk++;
        }
        else
            label_map[i] = 0;
    }
    QVector<int> ret(labels.count());
    for (int i = 0; i < labels.count(); i++) {
        ret[i] = label_map[labels[i]];
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////
class Cluster {
public:
    void addEvent(double t0, const Mda32& clip);
    double computeDistanceToClip(const Mda32& clip);

private:
    QVector<double> m_times;
    QList<Mda32> m_clips;
    int m_num_recent_events = 100;
};

void Cluster::addEvent(double t0, const Mda32& clip)
{
    m_times << t0;
    m_clips << clip;
    if (m_times.count() > m_num_recent_events) {
        m_times = m_times.mid(m_times.count() - m_num_recent_events);
        m_clips = m_clips.mid(m_clips.count() - m_num_recent_events);
    }
}

double compute_distance_between_clips(const Mda32& X, const Mda32& Y)
{
    double sumsqr = 0;
    int N = X.totalSize();
    for (int ii = 0; ii < N; ii++) {
        double val = X.get(ii) - Y.get(ii);
        sumsqr += val * val;
    }
    return sqrt(sumsqr);
}

double Cluster::computeDistanceToClip(const Mda32& clip)
{
    double best_dist = -1;
    for (int i = 0; i < m_clips.count(); i++) {
        double dist0 = compute_distance_between_clips(m_clips[i], clip);
        if ((best_dist < 0) || (dist0 < best_dist))
            best_dist = dist0;
    }
    return best_dist;
}

//////////////////////////////////////////////////////////////////////
class ClusterList {
public:
    ~ClusterList()
    {
        qDeleteAll(m_clusters);
    }
    void addEvent(double t0, int k0, const Mda32& clip);
    int classifyEvent(const Mda32& clip);
    int numClusters() { return m_clusters.count(); }

private:
    QMap<int, Cluster*> m_clusters;
};

void ClusterList::addEvent(double t0, int k0, const Mda32& clip)
{
    if (!m_clusters.contains(k0)) {
        m_clusters[k0] = new Cluster;
    }
    m_clusters[k0]->addEvent(t0, clip);
}

int ClusterList::classifyEvent(const Mda32& clip)
{

    double best_distance = -1;
    int best_k = 0;
    QList<int> k_s = m_clusters.keys();
    foreach (int k, k_s) {
        double dist0 = m_clusters[k]->computeDistanceToClip(clip);
        if ((best_distance < 0) || (dist0 < best_distance)) {
            best_distance = dist0;
            best_k = k;
        }
    }
    return best_k;
}

/*
Mda32 grab_clips_subset(const Mda32& clips, const QVector<int>& inds)
{
    int M = clips.N1();
    int T = clips.N2();
    int LLL = inds.count();
    Mda32 ret;
    ret.allocate(M, T, LLL);
    for (int i = 0; i < LLL; i++) {
        int aaa = i * M * T;
        int bbb = inds[i] * M * T;
        for (int k = 0; k < M * T; k++) {
            ret.set(clips.get(bbb), aaa);
            aaa++;
            bbb++;
        }
    }
    return ret;
}
*/

Mda32 compute_sliding_average(const Mda32& clips, int radius)
{
    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();
    Mda32 ret(M, T, L);
    Mda sum(M, T);
    int count0 = 0;
    for (int i = 0; (i <= radius - 1) && (i < L); i++) {
        for (int t = 0; t < T; t++)
            for (int m = 0; m < M; m++)
                sum.set(sum.get(m, t) + clips.get(m, t, i), m, t);
        count0++;
    }
    for (int i = 0; i < L; i++) {
        if (i - radius - 1 >= 0) {
            for (int t = 0; t < T; t++)
                for (int m = 0; m < M; m++)
                    sum.set(sum.get(m, t) - clips.get(m, t, i - radius - 1), m, t);
            count0--;
        }
        if (i + radius < L) {
            for (int t = 0; t < T; t++)
                for (int m = 0; m < M; m++)
                    sum.set(sum.get(m, t) + clips.get(m, t, i + radius), m, t);
            count0++;
        }
        {
            for (int t = 0; t < T; t++)
                for (int m = 0; m < M; m++)
                    ret.set(sum.get(m, t) / count0, m, t);
        }
    }
    return ret;
}

void drift_correct_clips_in_single_cluster(Mda32& clips)
{
    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();
    int sliding_avg_radius = 100;
    Mda32 avg = compute_sliding_average(clips, sliding_avg_radius);
    Mda32 last;
    avg.getChunk(last, 0, 0, L - 1, M, T, 1);
    for (int i = 0; i < clips.N3(); i++) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                clips.setValue(clips.value(m, t, i) + last.value(m, t) - avg.value(m, t, i), m, t, i); //do the drift correction
            }
        }
    }
}

void drift_correct_clips(Mda32& clips, const QVector<double>& times, const QVector<int>& labels)
{
    int M = clips.N1();
    int T = clips.N2();
    QList<int> time_sort_inds = get_sort_indices(times);
    int K = MLCompute::max(labels);
    for (int k = 1; k <= K; k++) {
        QVector<int> inds_k;
        for (int jj = 0; jj < time_sort_inds.count(); jj++) {
            if (labels[time_sort_inds[jj]] == k) {
                inds_k << time_sort_inds[jj];
            }
        }
        if (!inds_k.isEmpty()) {
            Mda32 clips_k = grab_clips_subset(clips, inds_k);
            drift_correct_clips_in_single_cluster(clips_k);
            for (int ii = 0; ii < inds_k.count(); ii++) {
                Mda32 tmp;
                clips_k.getChunk(tmp, 0, 0, ii, M, T, 1);
                clips.setChunk(tmp, 0, 0, inds_k[ii]);
            }
        }
    }
}

struct Segment {
    QVector<int> indices1;
    QVector<int> indices2;

    QVector<int> labels1;
    QVector<int> labels2;
};

void assess_cluster_agreements(QMap<int, int>& agreement_map, QMap<int, double>& agreement_scores, const QVector<int>& labels, const QVector<int>& labels_prev)
{

    QMap<int, double> counts, counts_prev;
    for (int jj = 0; jj < labels.count(); jj++)
        counts[labels[jj]]++;
    for (int jj = 0; jj < labels_prev.count(); jj++)
        counts_prev[labels_prev[jj]]++;
    QMap<int, QMap<int, double> > pair_counts;
    for (int jj = 0; jj < labels.count(); jj++) {
        pair_counts[labels[jj]][labels_prev[jj]]++;
    }

    QSet<int> labels_set, labels_prev_set;
    foreach (int k, labels)
        labels_set.insert(k);
    foreach (int k, labels_prev)
        labels_prev_set.insert(k);
    foreach (int k, labels_set) {
        double best_score = 0;
        int best_match = 0;
        foreach (int k_prev, labels_prev_set) {
            double numer0 = pair_counts[k][k_prev];
            double denom0 = counts[k] + counts_prev[k_prev] - pair_counts[k][k_prev];
            if (!denom0)
                denom0 = 1;
            double score0 = numer0 / denom0;
            if (score0 > best_score) {
                best_score = score0;
                best_match = k_prev;
            }
        }
        agreement_map[k] = best_match;
        agreement_scores[k] = best_score;
    }
}

QVector<int> cluster_in_neighborhood(const DiskReadMda32& X, const QVector<int>& neighborhood_channels, const QVector<double>& times, const isocluster_drift_v1_opts& opts)
{
    printf("cluster_in_neighborhood (Channel %d, %d events)...\n", neighborhood_channels[0] + 1, times.count());
    int N = X.N2();
    int L = times.count();

    QList<Segment> segments;
    int jj = 0;
    while (1) {
        int tt1 = jj * opts.segment_size / 2;
        int tt2 = (jj + 1) * opts.segment_size / 2;
        if (tt2 >= N)
            break;
        int tt3 = (jj + 2) * opts.segment_size / 2;
        if (tt3 > N)
            tt3 = N;
        Segment S;
        for (int ii = 0; ii < L; ii++) {
            if ((tt1 <= times[ii]) && (times[ii] < tt2)) {
                S.indices1 << ii;
            }
            else if ((tt2 <= times[ii]) && (times[ii] < tt3)) {
                S.indices2 << ii;
            }
        }
        segments << S;
        jj++;
    }

    int k_offset = 0;
    for (int i = 0; i < segments.count(); i++) {
        QVector<int> indices_seg = segments[i].indices1;
        indices_seg.append(segments[i].indices2);
        printf("Channel %d: Clustering %d events in segment %d of %d\n", neighborhood_channels[0] + 1, indices_seg.count(), i + 1, segments.count());
        QVector<double> times_seg;
        for (int jj = 0; jj < indices_seg.count(); jj++)
            times_seg << times[indices_seg[jj]];
        Mda32 clips_seg = extract_clips(X, times_seg, neighborhood_channels, opts.clip_size);
        QVector<int> labels_seg = do_cluster(clips_seg, opts);
        printf("Channel %d: Found %d clusters in segment.\n", neighborhood_channels[0] + 1, MLCompute::max(labels_seg));
        for (int jj = 0; jj < segments[i].indices1.count(); jj++) {
            int label0 = labels_seg[jj];
            if (label0 > 0)
                segments[i].labels1 << k_offset + label0;
            else
                segments[i].labels1 << 0;
        }
        for (int jj = 0; jj < segments[i].indices2.count(); jj++) {
            int label0 = labels_seg[segments[i].indices1.count() + jj];
            if (label0 > 0)
                segments[i].labels2 << k_offset + label0;
            else
                segments[i].labels2 << 0;
        }
        k_offset += MLCompute::max(labels_seg);
    }

    double agreement_threshold = 0.8;

    QVector<int> output_labels(L);
    for (int i = 0; i < L; i++)
        output_labels[i] = 0;
    for (int i = 0; i < segments.count(); i++) {
        Segment* S = &segments[i];
        if (i == 0) {
            for (int jj = 0; jj < S->indices1.count(); jj++) {
                output_labels[S->indices1[jj]] = S->labels1[jj];
            }
            for (int jj = 0; jj < S->indices2.count(); jj++) {
                output_labels[S->indices2[jj]] = S->labels2[jj];
            }
        }
        else {
            Segment* Sprev = &segments[i - 1];
            if (Sprev->indices2.count() != S->indices1.count()) {
                qCritical() << "Unexpected mismatch of cluster sizes." << Sprev->indices2.count() << S->indices1.count();
                abort();
            }
            QMap<int, int> agreement_map;
            QMap<int, double> agreement_scores;
            printf("Assessing agreements for segment %d of %d\n", i + 1, segments.count());
            assess_cluster_agreements(agreement_map, agreement_scores, S->labels1, Sprev->labels2);
            for (int jj = 0; jj < S->indices1.count(); jj++) {
                int label0 = S->labels1[jj];
                if ((agreement_map.contains(label0)) && (agreement_scores[label0] > agreement_threshold)) {
                    label0 = agreement_map[label0];
                }
                S->labels1[jj] = label0;
            }
            for (int jj = 0; jj < S->indices2.count(); jj++) {
                int label0 = S->labels2[jj];
                if ((agreement_map.contains(label0)) && (agreement_scores[label0] > agreement_threshold)) {
                    label0 = agreement_map[label0];
                }
                S->labels2[jj] = label0;
                output_labels[S->indices2[jj]] = label0;
            }
        }
    }

    remove_empty_labels(output_labels);

    printf("Consolidating labels...\n");
    // Here's the critical step of discarding all clusters that are deemed redundant because they have higher energy on a channel other than the one they were identified on
    output_labels = consolidate_labels(X, times, output_labels, neighborhood_channels[0], opts.clip_size, opts.detect_interval, opts.consolidation_factor);

    printf("Found %d clusters in neighborhood of channel %d\n", MLCompute::max(output_labels), neighborhood_channels[0] + 1);

    printf(".\n");
    return output_labels;
}

struct ChannelResult {
    QVector<double> times;
    QVector<int> labels;
};

bool isocluster_drift_v1(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const isocluster_drift_v1_opts& opts)
{
    printf("Starting isocluster_drift_v1 --------------------\n");
    DiskReadMda32 X;
    X.setPath(timeseries_path);
    int M = X.N1();

    // Set up the input array (detected events)
    DiskReadMda detect;
    detect.setPath(detect_path);

    int L = detect.N2();
    printf("#events = %d\n", L);

    // Read the adjacency matrix, or set it to all ones
    Mda AM;
    if (!adjacency_matrix_path.isEmpty()) {
        AM.readCsv(adjacency_matrix_path);
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

    QVector<ChannelResult> channel_results(M);

#pragma omp parallel for
    for (int m = 0; m < M; m++) {
        //for (int m = 0; m <= 0; m++) {
        QVector<double> times_m;
        QVector<int> neighborhood_channels;
#pragma omp critical
        {
            for (int i = 0; i < L; i++) {
                int central_channel = detect.value(0, i);
                if ((central_channel == m + 1) || ((central_channel == 0) && (m == 0))) {
                    times_m << detect.value(1, i) - 1; //convert to 0-based indexing
                }
            }
            neighborhood_channels << m;
            for (int a = 0; a < M; a++)
                if ((AM.value(m, a)) && (a != m))
                    neighborhood_channels << a;
        }

        DiskReadMda32 X_m = X;
        QVector<int> labels_m = cluster_in_neighborhood(X_m, neighborhood_channels, times_m, opts);

#pragma omp critical
        {
            channel_results[m].times = times_m;
            channel_results[m].labels = labels_m;
        }
    }

    QVector<double> output_times;
    QVector<int> output_chans;
    QVector<int> output_labels;
    int k_offset = 0;
    for (int m = 0; m < M; m++) {
        int K_m = MLCompute::max(channel_results[m].labels);
        for (int jj = 0; jj < channel_results[m].times.count(); jj++) {
            output_times << channel_results[m].times[jj];
            output_labels << k_offset + channel_results[m].labels[jj];
            output_chans << m + 1;
        }
        k_offset += K_m;
    }

    int L_true = output_times.count();
    QList<int> sort_inds = get_sort_indices(output_times);

    Mda firings;
    firings.allocate(3, L_true);
    for (int i = 0; i < L_true; i++) {
        double time0 = output_times[sort_inds[i]];
        int label0 = output_labels[sort_inds[i]];
        int chan0 = output_chans[sort_inds[i]];
        firings.setValue(chan0, 0, i);
        firings.setValue(time0, 1, i);
        firings.setValue(label0, 2, i);
    }

    //Now reorder the labels, so they are sorted nicely by channel and amplitude (good for display)
    int K; // this will be the number of clusters
    {
        //printf("Reordering labels...\n");
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
        QList<int> sort_inds = Isocluster_drift_v1::get_sort_indices_b(channels, template_peaks);
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

    // write the output
    firings.write64(output_firings_path);

    //printf("Found %d clusters and %d events\n", K, firings.N2());

    return true;
}
}
