/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef ISOCLUSTER_DRIFT_V1_H
#define ISOCLUSTER_DRIFT_V1_H

#include <QString>
#include <mda32.h>

struct isocluster_drift_v1_opts {
    int clip_size;
    int num_features;
    int num_features2 = 0;
    int detect_interval;
    double consolidation_factor;
    double isocut_threshold = 1.5;
    int K_init = 200;
    int segment_size = 30000 * 60 * 5;
};

namespace Isocluster_drift_v1 {
bool isocluster_drift_v1(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const isocluster_drift_v1_opts& opts);
}

#endif // ISOCLUSTER_DRIFT_V1_H
