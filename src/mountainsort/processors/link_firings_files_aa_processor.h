/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef link_firings_files_aa_PROCESSOR_H
#define link_firings_files_aa_PROCESSOR_H

#include "msprocessor.h"

class link_firings_files_aa_ProcessorPrivate;
class link_firings_files_aa_Processor : public MSProcessor {
public:
    friend class link_firings_files_aa_ProcessorPrivate;
    link_firings_files_aa_Processor();
    virtual ~link_firings_files_aa_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    link_firings_files_aa_ProcessorPrivate* d;
};

#endif // link_firings_files_aa_PROCESSOR_H
