#include "concat_mda_processor.h"
#include "mlcommon.h"

#include <diskreadmda.h>
#include <diskwritemda.h>

class concat_mda_ProcessorPrivate {
public:
    concat_mda_Processor* q;
};

concat_mda_Processor::concat_mda_Processor()
{
    d = new concat_mda_ProcessorPrivate;
    d->q = this;

    this->setName("concat_mda");
    this->setVersion("0.11");
    this->setInputFileParameters("inputs");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("dimension");
}

concat_mda_Processor::~concat_mda_Processor()
{
    delete d;
}

bool concat_mda_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool concat_mda_Processor::run(const QMap<QString, QVariant>& params)
{
    int dimension = params["dimension"].toInt();
    QStringList input_paths = MLUtil::toStringList(params["inputs"]);
    if (input_paths.count() < 1) {
        qWarning() << "At least one input file required.";
        return false;
    }

    QString output_path = params["output"].toString();

    QString path0 = input_paths[0];
    DiskReadMda X0(path0);
    int DIMS[4];
    for (int dd = 1; dd <= 3; dd++) {
        if (dd != dimension) {
            DIMS[dd] = X0.N(dd);
        }
        else {
            DIMS[dd] = 0;
        }
    }
    for (int i = 0; i < input_paths.count(); i++) {
        QString path1 = input_paths[i];
        DiskReadMda X1(path1);
        DIMS[dimension] += X1.N(dimension);
        for (int dd = 1; dd <= 3; dd++) {
            if (dd != dimension) {
                if (DIMS[dd] != X1.N(dd)) {
                    qWarning() << "Incompatible dimensions for concatenation.";
                    return false;
                }
            }
        }
        printf("INPUT: %ld x %ld x %ld\n", X1.N1(), X1.N2(), X1.N3());
    }

    printf("OUTPUT:  %d x %d x %d\n", DIMS[1], DIMS[2], DIMS[3]);
    DiskWriteMda Y(X0.mdaioHeader().data_type, output_path, DIMS[1], DIMS[2], DIMS[3]);
    int offset = 0;
    for (int i = 0; i < input_paths.count(); i++) {
        QString path1 = input_paths[i];
        DiskReadMda X1(path1);
        Mda chunk;
        printf("Reading chunk...\n");
        if (!X1.readChunk(chunk, 0, 0, 0, X1.N1(), X1.N2(), X1.N3()))
            return false;
        int ind1 = 0;
        int ind2 = 0;
        int ind3 = 0;
        if (dimension == 1) {
            ind1 = offset;
            offset += X1.N1();
        }
        else if (dimension == 2) {
            ind2 = offset;
            offset += X1.N2();
        }
        else if (dimension == 3) {
            ind3 = offset;
            offset += X1.N3();
        }
        printf("Writing chunk...\n");
        if (!Y.writeChunk(chunk, ind1, ind2, ind3))
            return false;
    }
    printf("Done.\n");

    return true;
}
