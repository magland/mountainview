/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef extract_clips_aa_PROCESSOR_H
#define extract_clips_aa_PROCESSOR_H

#include "msprocessor.h"

class extract_clips_aa_ProcessorPrivate;
class extract_clips_aa_Processor : public MSProcessor {
public:
    friend class extract_clips_aa_ProcessorPrivate;
    extract_clips_aa_Processor();
    virtual ~extract_clips_aa_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    extract_clips_aa_ProcessorPrivate* d;
};

#endif // extract_clips_aa_PROCESSOR_H
