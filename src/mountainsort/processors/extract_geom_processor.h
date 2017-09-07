#ifndef EXTRACT_GEOM_PROCESSOR_H
#define EXTRACT_GEOM_PROCESSOR_H

#include "msprocessor.h"

class extract_geom_ProcessorPrivate;
class extract_geom_Processor : public MSProcessor {
public:
    friend class extract_geom_ProcessorPrivate;
    extract_geom_Processor();
    virtual ~extract_geom_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    extract_geom_ProcessorPrivate* d;
};

#endif // EXTRACT_GEOM_PROCESSOR_H
