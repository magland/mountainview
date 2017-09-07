#include "cluster_aa.h"

#include <mda.h>
#include "pca.h"
#include "isosplit5.h"

bool cluster_aa(const QString& clips_path, const QString& detect_path, const QString& firings_out_path, const cluster_aa_opts& opts)
{
    Mda32 clips(clips_path);

    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();

    Mda32 FF;
    {
        // do this inside a code block so memory gets released
        Mda32 clips_reshaped(M * T, L);
        int iii = 0;
        for (int ii = 0; ii < L; ii++) {
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    clips_reshaped.set(clips.value(m, t, ii), iii);
                    iii++;
                }
            }
        }

        Mda32 CC, sigma;
        pca(CC, FF, sigma, clips_reshaped, opts.num_features, false); //should we subtract the mean?
    }

    isosplit5_opts i5_opts;
    i5_opts.isocut_threshold = opts.isocut_threshold;
    i5_opts.K_init = opts.K_init;

    QVector<int> labels(L);
    isosplit5(labels.data(), opts.num_features, L, FF.dataPtr(), i5_opts);

    Mda detect(detect_path);
    int R = detect.N1();
    if (R < 3)
        R = 3;
    Mda firings(R, L);
    for (int i = 0; i < L; i++) {
        for (int r = 0; r < R; r++) {
            firings.setValue(detect.value(r, i), r, i); //important to use .value() here because otherwise it will be out of range
        }
        firings.setValue(labels[i], 2, i);
    }

    return firings.write64(firings_out_path);
}
