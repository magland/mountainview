/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef combine_firings_files_aa_PROCESSOR_H
#define combine_firings_files_aa_PROCESSOR_H

#include "msprocessor.h"

class combine_firings_files_aa_ProcessorPrivate;
class combine_firings_files_aa_Processor : public MSProcessor {
public:
    friend class combine_firings_files_aa_ProcessorPrivate;
    combine_firings_files_aa_Processor();
    virtual ~combine_firings_files_aa_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    combine_firings_files_aa_ProcessorPrivate* d;
};

#endif // combine_firings_files_aa_PROCESSOR_H
