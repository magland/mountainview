/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef ISOCLUSTER_V2_PROCESSOR_H
#define ISOCLUSTER_V2_PROCESSOR_H

#include "msprocessor.h"

class isocluster_v2_ProcessorPrivate;
class isocluster_v2_Processor : public MSProcessor {
public:
    friend class isocluster_v2_ProcessorPrivate;
    isocluster_v2_Processor();
    virtual ~isocluster_v2_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    isocluster_v2_ProcessorPrivate* d;
};

#endif // ISOCLUSTER_V2_PROCESSOR_H
