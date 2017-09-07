/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef isosplit2_h
#define isosplit2_h

#include "mda32.h"
#include <QList>

QVector<int> isosplit2(Mda32& X, float isocut_threshold = 1.5, int K_init = 30, bool verbose = false);
void test_isosplit2_routines();

QList<int> find_inds(const QVector<int>& labels, int k);

#endif
