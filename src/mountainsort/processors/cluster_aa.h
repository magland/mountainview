/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef cluster_aa_H
#define cluster_aa_H

#include <QString>
#include <mda32.h>

struct cluster_aa_opts {
    int num_features;
    int num_features2 = 0;
    double consolidation_factor;
    double isocut_threshold = 1.5;
    int K_init = 200;
};

bool cluster_aa(const QString& clips_path, const QString& detect_path, const QString& firings_out_path, const cluster_aa_opts& opts);

#endif // cluster_aa_H
