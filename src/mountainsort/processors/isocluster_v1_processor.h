/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef ISOCLUSTER_V1_PROCESSOR_H
#define ISOCLUSTER_V1_PROCESSOR_H

#include "msprocessor.h"

class isocluster_v1_ProcessorPrivate;
class isocluster_v1_Processor : public MSProcessor {
public:
    friend class isocluster_v1_ProcessorPrivate;
    isocluster_v1_Processor();
    virtual ~isocluster_v1_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    isocluster_v1_ProcessorPrivate* d;
};

#endif // ISOCLUSTER_V1_PROCESSOR_H
