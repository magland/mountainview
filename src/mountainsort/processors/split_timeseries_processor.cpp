#include "split_timeseries_processor.h"

#include "mlcommon.h"

#include <diskreadmda.h>
#include <diskreadmda32.h>
#include <diskwritemda.h>

class concat_timeseries_ProcessorPrivate {
public:
    concat_timeseries_Processor* q;
};

concat_timeseries_Processor::concat_timeseries_Processor()
{
    d = new concat_timeseries_ProcessorPrivate;
    d->q = this;

    this->setName("concat_timeseries");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("timeseries_concat");
}

concat_timeseries_Processor::~concat_timeseries_Processor()
{
    delete d;
}

bool concat_timeseries_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool concat_timeseries_Processor::run(const QMap<QString, QVariant>& params)
{
    QStringList timeseries = MLUtil::toStringList(params["timeseries"]);
    QString timeseries_concat = params["timeseries_concat"].toString();

    if (timeseries.count() < 1) {
        qWarning() << "Need at least one timeseries.";
        return false;
    }

    DiskReadMda32 X0(timeseries[0]);
    int M = X0.N1();
    int N = 0;

    for (int i = 0; i < timeseries.count(); i++) {
        DiskReadMda32 X1(timeseries[i]);
        if (X1.N1() != M) {
            qWarning() << "Dimension mismatch.";
            return false;
        }
        N += X1.N2();
    }

    DiskWriteMda Y(X0.mdaioHeader().data_type, timeseries_concat, M, N);

    int N_offset = 0;
    for (int i = 0; i < timeseries.count(); i++) {
        DiskReadMda32 X1(timeseries[i]);
        Mda32 chunk;
        printf("Reading %d x %ld\n", M, X1.N2());
        if (!X1.readChunk(chunk, 0, 0, M, X1.N2())) {
            qWarning() << "Error reading chunk.";
            return false;
        }
        printf("Writing %d x %ld\n", M, X1.N2());
        if (!Y.writeChunk(chunk, 0, N_offset)) {
            qWarning() << "Error writing chunk.";
            return false;
        }
        N_offset += X1.N2();
    }

    return true;
}

class split_timeseries_ProcessorPrivate {
public:
    split_timeseries_Processor* q;
};

split_timeseries_Processor::split_timeseries_Processor()
{
    d = new split_timeseries_ProcessorPrivate;
    d->q = this;

    this->setName("split_timeseries");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "raw_timeseries");
    this->setOutputFileParameters("timeseries_split");
}

split_timeseries_Processor::~split_timeseries_Processor()
{
    delete d;
}

bool split_timeseries_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool split_timeseries_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries = params["timeseries"].toString();
    QStringList raw_timeseries = MLUtil::toStringList(params["raw_timeseries"]);
    QStringList timeseries_split = MLUtil::toStringList(params["timeseries_split"]);

    if (raw_timeseries.count() != timeseries_split.count()) {
        qWarning() << "Size mismatch between raw_timeseries and timeseries_split" << raw_timeseries.count() << timeseries_split.count();
        return false;
    }

    DiskReadMda32 X(timeseries);
    int M = X.N1();
    int N_offset = 0;
    for (int i = 0; i < raw_timeseries.count(); i++) {
        DiskReadMda R(raw_timeseries[i]);
        if (R.N1() != M) {
            qWarning() << "Dimension mismatch" << R.N1() << M;
            return false;
        }
        Mda32 chunk;
        printf("Reading %d x %ld...\n", M, R.N2());
        if (!X.readChunk(chunk, 0, N_offset, M, R.N2())) {
            qWarning() << "Problem reading chunk.";
            return false;
        }
        printf("Writing %d x %ld...\n", M, R.N2());
        if (!chunk.write32(timeseries_split[i])) {
            qWarning() << "Problem writing chunk.";
            return false;
        }
        N_offset += R.N2();
    }

    return true;
}

class split_firings_ProcessorPrivate {
public:
    split_firings_Processor* q;
};

split_firings_Processor::split_firings_Processor()
{
    d = new split_firings_ProcessorPrivate;
    d->q = this;

    this->setName("split_firings");
    this->setVersion("0.12");
    this->setInputFileParameters("firings", "raw_timeseries");
    this->setOutputFileParameters("firings_split");
}

split_firings_Processor::~split_firings_Processor()
{
    delete d;
}

bool split_firings_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool split_firings_Processor::run(const QMap<QString, QVariant>& params)
{
    QString firings = params["firings"].toString();
    QStringList raw_timeseries = MLUtil::toStringList(params["raw_timeseries"]);
    QStringList firings_split = MLUtil::toStringList(params["firings_split"]);

    if (raw_timeseries.count() != firings_split.count()) {
        qWarning() << "Size mismatch between raw_timeseries and firings_split" << raw_timeseries.count() << firings_split.count();
        return false;
    }

    Mda F(firings);
    QVector<int> rounded_timepoints;
    for (int i = 0; i < F.N2(); i++) {
        rounded_timepoints << (int)F.value(1, i);
    }
    int N_offset = 0;
    for (int i = 0; i < raw_timeseries.count(); i++) {
        DiskReadMda R(raw_timeseries[i]);
        int min_timepoint = N_offset + 1;
        int max_timepoint = N_offset + R.N2();

        QVector<int> inds_to_use;
        for (int j = 0; j < rounded_timepoints.count(); j++) {
            if ((rounded_timepoints[j] >= min_timepoint) && (rounded_timepoints[j] <= max_timepoint)) {
                inds_to_use << j;
            }
        }
        Mda F2(F.N1(), inds_to_use.count());
        for (int j = 0; j < inds_to_use.count(); j++) {
            for (int k = 0; k < F.N1(); k++) {
                F2.setValue(F.value(k, inds_to_use[j]), k, j);
            }
            F2.setValue(F2.value(1, j) - N_offset, 1, j);
        }
        printf("Writing %ld x %ld\n", F2.N1(), F2.N2());
        F2.write64(firings_split[i]);

        N_offset += R.N2();
    }

    return true;
}
