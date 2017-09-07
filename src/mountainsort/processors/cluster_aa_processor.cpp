#include "cluster_aa_processor.h"
#include "cluster_aa.h"
#include "omp.h"

class cluster_aa_ProcessorPrivate {
public:
    cluster_aa_Processor* q;
};

cluster_aa_Processor::cluster_aa_Processor()
{
    d = new cluster_aa_ProcessorPrivate;
    d->q = this;

    this->setName("cluster_aa");
    this->setVersion("0.1");
    this->setInputFileParameters("clips", "detect");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("num_features", "num_features2");
    this->setOptionalParameters("consolidation_factor");
    this->setOptionalParameters("isocut_threshold");
    this->setOptionalParameters("K_init");
}

cluster_aa_Processor::~cluster_aa_Processor()
{
    delete d;
}

bool cluster_aa_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool cluster_aa_Processor::run(const QMap<QString, QVariant>& params)
{
    cluster_aa_opts opts;

    QString clips_path = params["clips"].toString();
    QString detect_path = params["detect"].toString();
    QString firings_out_path = params["firings_out"].toString();
    opts.num_features = params["num_features"].toInt();
    opts.num_features2 = params.value("num_features2", 0).toInt();
    opts.consolidation_factor = params.value("consolidation_factor", 0.9).toDouble();
    opts.isocut_threshold = params.value("isocut_threshold", 1).toDouble();
    opts.K_init = params.value("K_init", 200).toInt();

    return cluster_aa(clips_path, detect_path, firings_out_path, opts);
}
