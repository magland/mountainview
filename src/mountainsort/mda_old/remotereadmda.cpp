/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/2/2016
*******************************************************/

#include "remotereadmda.h"
#include "mlcommon.h"
#include "diskreadmda.h"
#include <QStringList>
#include <QDir>
#include <QDateTime>
#include "../cachemanager/mscachemanager.h"

#define REMOTE_READ_MDA_CHUNK_SIZE 5e5

struct RemoteReadMdaInfo {
    int N1, N2, N3;
    QString checksum;
    QDateTime file_last_modified;
};

class RemoteReadMdaPrivate {
public:
    RemoteReadMda* q;

    QString m_path;
    RemoteReadMdaInfo m_info;
    bool m_info_downloaded;

    void download_info_if_needed();
    QString download_chunk_at_index(int ii);
};

RemoteReadMda::RemoteReadMda(const QString& path)
{
    d = new RemoteReadMdaPrivate;
    d->q = this;
    this->setPath(path);
}

RemoteReadMda::RemoteReadMda(const RemoteReadMda& other)
{
    d = new RemoteReadMdaPrivate;
    d->q = this;
    this->setPath(other.d->m_path);
}

void RemoteReadMda::operator=(const RemoteReadMda& other)
{
    this->setPath(other.d->m_path);
}

RemoteReadMda::~RemoteReadMda()
{
    delete d;
}

void RemoteReadMda::setPath(const QString& path)
{
    d->m_info.N1 = d->m_info.N2 = d->m_info.N3 = 0;
    d->m_info_downloaded = false;
    d->m_path = path;
}

QString RemoteReadMda::path() const
{
    return d->m_path;
}

int RemoteReadMda::N1()
{
    d->download_info_if_needed();
    return d->m_info.N1;
}

int RemoteReadMda::N2()
{
    d->download_info_if_needed();
    return d->m_info.N2;
}

int RemoteReadMda::N3()
{
    d->download_info_if_needed();
    return d->m_info.N3;
}

QDateTime RemoteReadMda::fileLastModified()
{
    d->download_info_if_needed();
    return d->m_info.file_last_modified;
}

bool RemoteReadMda::readChunk(Mda& X, int i, int size) const
{
    X.allocate(size, 1);
    double* Xptr = X.dataPtr();
    int ii1 = i;
    int ii2 = i + size - 1;
    int jj1 = ii1 / REMOTE_READ_MDA_CHUNK_SIZE;
    int jj2 = ii2 / REMOTE_READ_MDA_CHUNK_SIZE;
    if (jj1 == jj2) {
        QString fname = d->download_chunk_at_index(jj1);
        if (fname.isEmpty())
            return false;
        DiskReadMda A(fname);
        A.readChunk(X, ii1 - jj1 * REMOTE_READ_MDA_CHUNK_SIZE, size);
        return true;
    }
    else {
        for (int jj = jj1; jj <= jj2; jj++) {
            QString fname = d->download_chunk_at_index(jj);
            if (fname.isEmpty())
                return false;
            DiskReadMda A(fname);
            if (jj == jj1) {
                Mda tmp;
                int size0 = (jj1 + 1) * REMOTE_READ_MDA_CHUNK_SIZE - ii1;
                A.readChunk(tmp, ii1 - jj1 * REMOTE_READ_MDA_CHUNK_SIZE, size0);
                double* tmp_ptr = tmp.dataPtr();
                int b = 0;
                for (int a = 0; a < size0; a++) {
                    Xptr[b] = tmp_ptr[a];
                    b++;
                }
            }
            else if (jj == jj2) {
                Mda tmp;
                int size0 = ii2 + 1 - jj2 * REMOTE_READ_MDA_CHUNK_SIZE;
                A.readChunk(tmp, REMOTE_READ_MDA_CHUNK_SIZE - size0, size0);
                double* tmp_ptr = tmp.dataPtr();
                int b = jj2 * REMOTE_READ_MDA_CHUNK_SIZE - ii1;
                for (int a = 0; a < size0; a++) {
                    Xptr[b] = tmp_ptr[a];
                    b++;
                }
            }
            else {
                Mda tmp;
                A.readChunk(tmp, 0, REMOTE_READ_MDA_CHUNK_SIZE);
                double* tmp_ptr = tmp.dataPtr();
                int b = jj * REMOTE_READ_MDA_CHUNK_SIZE - ii1;
                for (int a = 0; a < REMOTE_READ_MDA_CHUNK_SIZE; a++) {
                    Xptr[b] = tmp_ptr[a];
                    b++;
                }
            }
        }
        return true;
    }
}

void RemoteReadMdaPrivate::download_info_if_needed()
{
    if (m_info_downloaded)
        return;
    m_info_downloaded = true;
    //QString url=file_url_for_remote_path(m_path);
    QString url = m_path;
    QString url2 = url + "?a=info&output=text";
    QString txt = MLNetwork::httpGetText(url2);
    QStringList lines = txt.split("\n");
    QStringList sizes = lines.value(0).split(",");
    m_info.N1 = sizes.value(0).toLong();
    m_info.N2 = sizes.value(1).toLong();
    m_info.N3 = sizes.value(2).toLong();
    m_info.checksum = lines.value(1);
    m_info.file_last_modified = QDateTime::fromMSecsSinceEpoch(lines.value(2).toLong());
}

QString RemoteReadMdaPrivate::download_chunk_at_index(int ii)
{
    int Ntot = m_info.N1 * m_info.N2 * m_info.N3;
    int size = REMOTE_READ_MDA_CHUNK_SIZE;
    if (ii * REMOTE_READ_MDA_CHUNK_SIZE + size > Ntot) {
        size = Ntot - ii * REMOTE_READ_MDA_CHUNK_SIZE;
    }
    if (size <= 0)
        return "";
    if (m_info.checksum.isEmpty())
        return "";
    QString file_name = m_info.checksum + "-" + QString("%1-%2").arg(REMOTE_READ_MDA_CHUNK_SIZE).arg(ii);
    QString fname = MSCacheManager::globalInstance()->makeLocalFile(file_name, MSCacheManager::ShortTerm);
    if (QFile::exists(fname))
        return fname;
    //QString url=file_url_for_remote_path(m_path);
    QString url = m_path;
    QString url0 = url + QString("?a=readChunk&output=text&index=%1&size=%2&datatype=float64").arg((int)(ii * REMOTE_READ_MDA_CHUNK_SIZE)).arg(size);
    QString binary_url = MLNetwork::httpGetText(url0).trimmed();
    if (binary_url.isEmpty())
        return "";
    QString mda_fname = MLNetwork::httpGetBinaryFile(binary_url);
    if (mda_fname.isEmpty())
        return "";
    DiskReadMda tmp(mda_fname);
    if (tmp.totalSize() != size) {
        qWarning() << "Unexpected total size problem: " << tmp.totalSize() << size;
        QFile::remove(mda_fname);
        return "";
    }
    QFile::rename(mda_fname, fname);
    return fname;
}

void unit_test_remote_read_mda()
{
    //removed by jfm 5/2/16
}

#include "diskreadmda.h"
void unit_test_remote_read_mda_2(const QString& path)
{
    // run this test by calling
    // > mountainview unit_test remotereadmda2

    DiskReadMda X(path);
    for (int j = 0; j < 20; j++) {
        printf("%d: ", j);
        for (int i = 0; i < X.N1(); i++) {
            printf("%g, ", X.value(i, j));
        }
        printf("\n");
    }
}
