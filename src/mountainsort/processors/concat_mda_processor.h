#ifndef CONCAT_MDA_PROCESSOR_H
#define CONCAT_MDA_PROCESSOR_H

#include "msprocessor.h"

class concat_mda_ProcessorPrivate;
class concat_mda_Processor : public MSProcessor {
public:
    friend class concat_mda_ProcessorPrivate;
    concat_mda_Processor();
    virtual ~concat_mda_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    concat_mda_ProcessorPrivate* d;
};

#endif // CONCAT_MDA_PROCESSOR_H
