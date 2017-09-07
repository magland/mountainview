/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef isocluster_drift_v1_PROCESSOR_H
#define isocluster_drift_v1_PROCESSOR_H

#include "msprocessor.h"

class isocluster_drift_v1_ProcessorPrivate;
class isocluster_drift_v1_Processor : public MSProcessor {
public:
    friend class isocluster_drift_v1_ProcessorPrivate;
    isocluster_drift_v1_Processor();
    virtual ~isocluster_drift_v1_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    isocluster_drift_v1_ProcessorPrivate* d;
};

#endif // isocluster_drift_v1_PROCESSOR_H
