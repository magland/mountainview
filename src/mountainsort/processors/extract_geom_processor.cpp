#include "extract_geom_processor.h"

#include <mda.h>
#include "extract_raw_processor.h" //for str_to_intlist()

class extract_geom_ProcessorPrivate {
public:
    extract_geom_Processor* q;
};

extract_geom_Processor::extract_geom_Processor()
{
    d = new extract_geom_ProcessorPrivate;
    d->q = this;

    this->setName("extract_geom");
    this->setVersion("0.13");
    this->setInputFileParameters("input");
    this->setOutputFileParameters("output");
    this->setOptionalParameters("channels");
}

extract_geom_Processor::~extract_geom_Processor()
{
    delete d;
}

bool extract_geom_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool extract_geom_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input = params["input"].toString();
    QString output = params["output"].toString();
    QString channels_str = params["channels"].toString();
    QVector<int> channels = str_to_intlist(channels_str);

    Mda X;
    X.readCsv(input);
    int N = X.N1(); // note transposed rel to appearance in CSV
    int M = X.N2();

    if (channels.isEmpty()) {
        for (int m = 1; m <= M; m++)
            channels << m;
    }

    Mda Y(N, channels.count());
    for (int mm = 0; mm < channels.count(); mm++) {
        for (int nn = 0; nn < N; nn++) {
            Y.setValue(X.value(nn, channels[mm] - 1), nn, mm);
        }
    }

    return Y.writeCsv(output);
}
