/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "extract_clips_processor.h"

#include "extract_clips_processor.h"
#include "extract_clips.h"

class extract_clips_ProcessorPrivate {
public:
    extract_clips_Processor* q;
};

extract_clips_Processor::extract_clips_Processor()
{
    d = new extract_clips_ProcessorPrivate;
    d->q = this;

    this->setName("extract_clips");
    this->setVersion("0.13");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("clips");
    this->setRequiredParameters("clip_size");
    this->setOptionalParameters("channels", "t1", "t2");
}

extract_clips_Processor::~extract_clips_Processor()
{
    delete d;
}

bool extract_clips_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

#include "mlcommon.h"
bool extract_clips_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString clips_path = params["clips"].toString();
    QStringList channels_str = params.value("channels", "").toString().split(",", QString::SkipEmptyParts);
    QList<int> channels = MLUtil::stringListToIntList(channels_str);
    int clip_size = params["clip_size"].toInt();
    double t1 = params.value("t1", 0).toDouble();
    double t2 = params.value("t2", 0).toDouble();
    return extract_clips(timeseries_path, firings_path, clips_path, clip_size, channels, t1, t2);
}
