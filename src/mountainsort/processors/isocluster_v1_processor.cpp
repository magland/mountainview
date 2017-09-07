#include "isocluster_v1_processor.h"
#include "isocluster_v1.h"
#include "omp.h"

class isocluster_v1_ProcessorPrivate {
public:
    isocluster_v1_Processor* q;
};

isocluster_v1_Processor::isocluster_v1_Processor()
{
    d = new isocluster_v1_ProcessorPrivate;
    d->q = this;

    this->setName("isocluster_v1");
    this->setVersion("0.12");
    this->setInputFileParameters("timeseries", "detect", "adjacency_matrix");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("clip_size", "num_features", "num_features2");
    this->setRequiredParameters("detect_interval");
    this->setOptionalParameters("consolidation_factor");
    this->setOptionalParameters("num_features2");
    this->setOptionalParameters("isocut_threshold");
    this->setOptionalParameters("K_init");
}

isocluster_v1_Processor::~isocluster_v1_Processor()
{
    delete d;
}

bool isocluster_v1_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool isocluster_v1_Processor::run(const QMap<QString, QVariant>& params)
{
    isocluster_v1_opts opts;

    QString timeseries_path = params["timeseries"].toString();
    QString detect_path = params["detect"].toString();
    QString adjacency_matrix_path = params["adjacency_matrix"].toString();
    QString firings_path = params["firings_out"].toString();
    opts.clip_size = params["clip_size"].toInt();
    opts.num_features = params["num_features"].toInt();
    opts.num_features2 = params.value("num_features2", 0).toInt();
    opts.detect_interval = params["detect_interval"].toInt();
    opts.consolidation_factor = params.value("consolidation_factor", 0.9).toDouble();
    opts.isocut_threshold = params.value("isocut_threshold", 1).toDouble();
    opts.K_init = params.value("K_init", 200).toInt();

    return isocluster_v1(timeseries_path, detect_path, adjacency_matrix_path, firings_path, opts);
}
