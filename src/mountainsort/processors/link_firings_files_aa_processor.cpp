/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "link_firings_files_aa_processor.h"

#include "link_firings_files_aa_processor.h"
#include "link_firings_files_aa.h"

class link_firings_files_aa_ProcessorPrivate {
public:
    link_firings_files_aa_Processor* q;
};

link_firings_files_aa_Processor::link_firings_files_aa_Processor()
{
    d = new link_firings_files_aa_ProcessorPrivate;
    d->q = this;

    this->setName("link_firings_files_aa");
    this->setVersion("0.1");
    this->setInputFileParameters("firings_list");
    this->setOutputFileParameters("firings_out");
}

link_firings_files_aa_Processor::~link_firings_files_aa_Processor()
{
    delete d;
}

bool link_firings_files_aa_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

#include "mlcommon.h"
bool link_firings_files_aa_Processor::run(const QMap<QString, QVariant>& params)
{
    QStringList firings_path_list = MLUtil::toStringList(params["firings_list"]);
    QString firings_out_path = params["firings_out"].toString();
    return link_firings_files_aa(firings_path_list, firings_out_path);
}
