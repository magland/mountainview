/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef DISKWRITEMDA_H
#define DISKWRITEMDA_H

#include "mda.h"

#include <QString>
#include <mdaio.h>

class DiskWriteMdaPrivate;
class DiskWriteMda {
public:
    friend class DiskWriteMdaPrivate;
    DiskWriteMda();
    DiskWriteMda(int data_type, const QString& path, int N1, int N2, int N3 = 1, int N4 = 1, int N5 = 1, int N6 = 1);
    virtual ~DiskWriteMda();
    bool open(int data_type, const QString& path, int N1, int N2, int N3 = 1, int N4 = 1, int N5 = 1, int N6 = 1);
    void close();

    int N1();
    int N2();
    int N3();
    int N4();
    int N5();
    int N6();
    int totalSize();

    void writeChunk(Mda& X, int i);
    void writeChunk(Mda& X, int i1, int i2);
    void writeChunk(Mda& X, int i1, int i2, int i3);

private:
    DiskWriteMdaPrivate* d;
};

#endif // DISKWRITEMDA_H
