/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef BRANCH_CLUSTER_V2_H
#define BRANCH_CLUSTER_V2_H

#include <QString>
#include <mda32.h>

struct Branch_Cluster_V2_Opts {
    int clip_size;
    int min_shell_size;
    double shell_increment;
    int num_features;
    int num_features2 = 0;
    int detect_interval;
    int num_pca_representatives;
    double consolidation_factor;
    bool split_clusters_at_end = true;
    double isocut_threshold = 1.5;
};

bool branch_cluster_v2(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const Branch_Cluster_V2_Opts& opts);
bool branch_cluster_v2b(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const Branch_Cluster_V2_Opts& opts);
bool branch_cluster_v2c(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const Branch_Cluster_V2_Opts& opts);

Mda32 compute_clips_features_per_channel(const Mda32& X, int num_features);

#endif // BRANCH_CLUSTER_V2_H
