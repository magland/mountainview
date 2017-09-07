/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef cluster_aa_PROCESSOR_H
#define cluster_aa_PROCESSOR_H

#include "msprocessor.h"

class cluster_aa_ProcessorPrivate;
class cluster_aa_Processor : public MSProcessor {
public:
    friend class cluster_aa_ProcessorPrivate;
    cluster_aa_Processor();
    virtual ~cluster_aa_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    cluster_aa_ProcessorPrivate* d;
};

#endif // cluster_aa_PROCESSOR_H
