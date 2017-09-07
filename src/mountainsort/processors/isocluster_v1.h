/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef ISOCLUSTER_V1_H
#define ISOCLUSTER_V1_H

#include <QString>
#include <mda32.h>

struct isocluster_v1_opts {
    int clip_size;
    int num_features;
    int num_features2 = 0;
    int detect_interval;
    double consolidation_factor;
    bool split_clusters_at_end = false;
    double isocut_threshold = 1.5;
    int K_init = 200;
};

bool isocluster_v1(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const isocluster_v1_opts& opts);

Mda32 compute_clips_features_per_channel(const Mda32& X, int num_features);

#endif // ISOCLUSTER_V1_H
