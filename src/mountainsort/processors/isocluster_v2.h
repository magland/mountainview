/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef ISOCLUSTER_V2_H
#define ISOCLUSTER_V2_H

#include <QString>
#include <mda32.h>

struct isocluster_v2_opts {
    int clip_size;
    int num_features;
    int num_features2 = 0;
    int detect_interval;
    double consolidation_factor;
    bool split_clusters_at_end = false;
    double isocut_threshold = 1.5;
    int K_init = 200;
    int time_segment_size = 0;

    int _internal_time_segment_t1 = 0;
    int _internal_time_segment_t2 = 0;
    bool _internal_verbose = true;
};

bool isocluster_v2(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const isocluster_v2_opts& opts);

Mda32 compute_clips_features_per_channel(const Mda32& X, int num_features);

#endif // ISOCLUSTER_V2_H
