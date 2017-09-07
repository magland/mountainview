/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "extract_clips_aa_processor.h"

#include "extract_clips_aa_processor.h"
#include "extract_clips_aa.h"

class extract_clips_aa_ProcessorPrivate {
public:
    extract_clips_aa_Processor* q;
};

extract_clips_aa_Processor::extract_clips_aa_Processor()
{
    d = new extract_clips_aa_ProcessorPrivate;
    d->q = this;

    this->setName("extract_clips_aa");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "detect");
    this->setOutputFileParameters("clips_out", "detect_out");
    this->setRequiredParameters("clip_size", "central_channel");
    this->setOptionalParameters("channels", "t1", "t2");
}

extract_clips_aa_Processor::~extract_clips_aa_Processor()
{
    delete d;
}

bool extract_clips_aa_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

#include "mlcommon.h"
bool extract_clips_aa_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString detect_path = params["detect"].toString();
    QString clips_out_path = params["clips_out"].toString();
    QString detect_out_path = params["detect_out"].toString();
    QStringList channels_str = params.value("channels", "").toString().split(",");
    QList<int> channels = MLUtil::stringListToIntList(channels_str);
    int central_channel = params.value("central_channel").toInt();
    int clip_size = params["clip_size"].toInt();
    double t1 = params.value("t1", 0).toDouble();
    double t2 = params.value("t2", 0).toDouble();
    return extract_clips_aa(timeseries_path, detect_path, clips_out_path, detect_out_path, clip_size, central_channel, channels, t1, t2);
}
