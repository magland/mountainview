#include "extract_clips_aa.h"

Mda32 extract_clips_aa(const DiskReadMda32& X, const QVector<double>& times, const QVector<int>& channels, int clip_size)
{
    int M = X.N1();
    int N = X.N2();
    int M0 = channels.count();
    int T = clip_size;
    int L = times.count();
    int Tmid = (int)((T + 1) / 2) - 1;
    Mda32 clips(M0, T, L);
    for (int i = 0; i < L; i++) {
        int t1 = (int)times[i] - Tmid;
        int t2 = t1 + T - 1;
        if ((t1 >= 0) && (t2 < N)) {
            Mda32 tmp;
            X.readChunk(tmp, 0, t1, M, T);
            for (int t = 0; t < T; t++) {
                for (int m0 = 0; m0 < M0; m0++) {
                    clips.set(tmp.get(channels[m0], t), m0, t, i);
                }
            }
        }
    }
    return clips;
}

bool extract_clips_aa(const QString& timeseries_path, const QString& detect_path, const QString& clips_out_path, const QString& detect_out_path, int clip_size, int central_channel, const QList<int>& channels_in, double t1, double t2)
{
    QVector<int> channels;
    for (int i = 0; i < channels_in.count(); i++)
        channels << channels_in[i] - 1;
    DiskReadMda32 X(timeseries_path);
    DiskReadMda D(detect_path);
    QVector<double> times;
    QVector<int> event_indices;
    for (int j = 0; j < D.N2(); j++) {
        if (D.value(0, j) == central_channel) {
            double t0 = D.value(1, j);
            if ((t2 == 0) || ((t1 <= t0) && (t0 <= t2))) {
                times << D.value(1, j);
                event_indices << j;
            }
        }
    }
    Mda32 clips = extract_clips_aa(X, times, channels, clip_size);
    clips.write32(clips_out_path);

    int R = D.N1();
    if (R < 4)
        R = 4;
    Mda detect_out(R, event_indices.count());
    for (int i = 0; i < event_indices.count(); i++) {
        for (int r = 0; r < R; r++) {
            detect_out.setValue(D.value(r, event_indices[i]), r, i);
        }
        detect_out.setValue(event_indices[i], 3, i);
    }
    detect_out.write64(detect_out_path);
    return true;
}
