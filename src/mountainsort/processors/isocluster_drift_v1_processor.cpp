#include "isocluster_drift_v1_processor.h"
#include "isocluster_drift_v1.h"
#include "omp.h"

class isocluster_drift_v1_ProcessorPrivate {
public:
    isocluster_drift_v1_Processor* q;
};

isocluster_drift_v1_Processor::isocluster_drift_v1_Processor()
{
    d = new isocluster_drift_v1_ProcessorPrivate;
    d->q = this;

    this->setName("isocluster_drift_v1");
    this->setVersion("0.2.7");
    this->setInputFileParameters("timeseries", "detect", "adjacency_matrix");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("clip_size", "num_features", "num_features2");
    this->setRequiredParameters("detect_interval");
    this->setOptionalParameters("consolidation_factor");
    this->setOptionalParameters("num_features2");
    this->setOptionalParameters("isocut_threshold");
    this->setOptionalParameters("K_init");
    this->setRequiredParameters("segment_size");
}

isocluster_drift_v1_Processor::~isocluster_drift_v1_Processor()
{
    delete d;
}

bool isocluster_drift_v1_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool isocluster_drift_v1_Processor::run(const QMap<QString, QVariant>& params)
{
    isocluster_drift_v1_opts opts;

    printf("isocluster_drift_v1_Processor...\n");

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
    opts.segment_size = (int)params.value("segment_size", 0).toDouble(); //important to go to double first because of scientific notation

    return Isocluster_drift_v1::isocluster_drift_v1(timeseries_path, detect_path, adjacency_matrix_path, firings_path, opts);
}
