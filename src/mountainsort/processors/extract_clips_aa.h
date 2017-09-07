/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef EXTRACT_CLIPS_aa_H
#define EXTRACT_CLIPS_aa_H

#include "mda.h"
#include "diskreadmda.h"
#include "diskreadmda32.h"

bool extract_clips_aa(const QString& timeseries_path, const QString& detect_path, const QString& clips_out_path, const QString& detect_out_path, int clip_size, int central_channel, const QList<int>& channels, double t1, double t2);

#endif // EXTRACT_CLIPS_aa_H
