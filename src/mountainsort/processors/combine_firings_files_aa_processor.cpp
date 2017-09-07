/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "combine_firings_files_aa_processor.h"

#include "combine_firings_files_aa_processor.h"
#include "combine_firings_files_aa.h"

class combine_firings_files_aa_ProcessorPrivate {
public:
    combine_firings_files_aa_Processor* q;
};

combine_firings_files_aa_Processor::combine_firings_files_aa_Processor()
{
    d = new combine_firings_files_aa_ProcessorPrivate;
    d->q = this;

    this->setName("combine_firings_files_aa");
    this->setVersion("0.1");
    this->setInputFileParameters("firings_list");
    this->setOutputFileParameters("firings_out");
}

combine_firings_files_aa_Processor::~combine_firings_files_aa_Processor()
{
    delete d;
}

bool combine_firings_files_aa_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

#include "mlcommon.h"
bool combine_firings_files_aa_Processor::run(const QMap<QString, QVariant>& params)
{
    QStringList firings_path_list = MLUtil::toStringList(params["firings_list"]);
    QString firings_out_path = params["firings_out"].toString();
    return combine_firings_files_aa::run(firings_path_list, firings_out_path);
}
