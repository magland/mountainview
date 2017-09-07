/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MDA_H
#define MDA_H

#ifdef QT_CORE_LIB
#include <QString>
#include <QDebug>
#endif

class MdaPrivate;
/** \class Mda - a multi-dimensional array corresponding to the .mda file format
 * @brief The Mda class
 *
 * An object of type Mda is a multi-dimensional array, with up to 6 dimensions. All indexing is 0-based.
 */
class Mda {
public:
    friend class MdaPrivate;
    ///Construct an array of size N1xN2x...xN6
    Mda(int N1 = 1, int N2 = 1, int N3 = 1, int N4 = 1, int N5 = 1, int N6 = 1);
    ///Construct an array and read the .mda file
    Mda(const QString mda_filename);
    ///Copy constructor
    Mda(const Mda& other);
    ///Assignment operator
    void operator=(const Mda& other);
    ///Destructor
    virtual ~Mda();
    ///Allocate an array of size N1xN2x...xN6
    bool allocate(int N1, int N2, int N3 = 1, int N4 = 1, int N5 = 1, int N6 = 1);
#ifdef QT_CORE_LIB
    ///Create an array with content read from the .mda file specified by path
    bool read(const QString& path);
    ///Write the array to the .mda file specified by path, with file format 32-bit float
    bool write32(const QString& path) const;
    ///Write the array to the .mda file specified by path, with file format 64-bit float
    bool write64(const QString& path) const;
#endif
    ///Create an array with content read from the .mda file specified by path
    bool read(const char* path);
    ///Write the array to the .mda file specified by path, with file format 32-bit float
    bool write32(const char* path) const;
    ///Write the array to the .mda file specified by path, with file format 64-bit float
    bool write64(const char* path) const;

    ///The number of dimensions. This will be between 2 and 6. It will be 3 if N3()>1 and N4()...N6() are all 1. And so on.
    int ndims() const;
    ///The size of the first dimension
    int N1() const;
    ///The size of the second dimension
    int N2() const;
    ///The size of the third dimension
    int N3() const;
    ///The size of the fourth dimension
    int N4() const;
    ///The size of the fifth dimension
    int N5() const;
    ///The size of the sixth dimension
    int N6() const;
    ///The product of N1() through N6()
    int totalSize() const;
    int size(int dimension_index); //zero-based

    ///The value of the ith entry of the vectorized array. For example get(3+N1()*4)==get(3,4). Use the slower value(i) to safely return 0 when i is out of bounds.
    double get(int i) const;
    ///The value of the (i1,i2) entry of the array. Use the slower value(i1,i2) when either of the indices are out of bounds.
    double get(int i1, int i2) const;
    ///The value of the (i1,i2,i3) entry of the array. Use the slower value(i1,i2,i3) when any of the indices are out of bounds.
    double get(int i1, int i2, int i3) const;
    ///The value of the (i1,i2,...,i6) entry of the array. Use the slower value(i1,i2,...,i6) when any of the indices are out of bounds.
    double get(int i1, int i2, int i3, int i4, int i5 = 0, int i6 = 0) const;

    ///Set the value of the ith entry of the vectorized array to val. For example set(0.4,3+N1()*4) is the same as set(0.4,3,4). Use the slower setValue(val,i) to safely handle the case when i is out of bounds.
    void set(double val, int i);
    ///Set the value of the (i1,i2) entry of the array to val. Use the slower setValue(val,i1,i2) to safely handle the case when either of the indices are out of bounds.
    void set(double val, int i1, int i2);
    ///Set the value of the (i1,i2,i3) entry of the array to val. Use the slower setValue(val,i1,i2,i3) to safely handle the case when any of the indices are out of bounds.
    void set(double val, int i1, int i2, int i3);
    ///Set the value of the (i1,i2,...,i6) entry of the array to val. Use the slower setValue(val,i1,i2,...,i6) to safely handle the case when any of the indices are out of bounds.
    void set(double val, int i1, int i2, int i3, int i4, int i5 = 0, int i6 = 0);

    ///Slower version of get(i), safely returning 0 when i is out of bounds.
    double value(int i) const;
    ///Slower version of get(i1,i2), safely returning 0 when either of the indices are out of bounds.
    double value(int i1, int i2) const;
    ///Slower version of get(i1,i2,i3), safely returning 0 when any of the indices are out of bounds.
    double value(int i1, int i2, int i3) const;
    ///Slower version of get(i1,i2,...,i6), safely returning 0 when any of the indices are out of bounds.
    double value(int i1, int i2, int i3, int i4, int i5 = 0, int i6 = 0) const;

    ///Slower version of set(val,i), safely doing nothing when i is out of bounds.
    void setValue(double val, int i);
    ///Slower version of set(val,i1,i2), safely doing nothing when either of the indices are out of bounds.
    void setValue(double val, int i1, int i2);
    ///Slower version of set(val,i1,i2,i3), safely doing nothing when any of the indices are out of bounds.
    void setValue(double val, int i1, int i2, int i3);
    ///Slower version of set(val,i1,i2,...,i6), safely doing nothing when any of the indices are out of bounds.
    void setValue(double val, int i1, int i2, int i3, int i4, int i5 = 0, int i6 = 0);

    ///Return a pointer to the 1D raw data. The internal data may be efficiently read/written.
    double* dataPtr();
    ///Return a pointer to the 1D raw data at the vectorized location i. The internal data may be efficiently read/written.
    double* dataPtr(int i);
    ///Return a pointer to the 1D raw data at the the location (i1,i2). The internal data may be efficiently read/written.
    double* dataPtr(int i1, int i2);
    ///Return a pointer to the 1D raw data at the the location (i1,i2,i3). The internal data may be efficiently read/written.
    double* dataPtr(int i1, int i2, int i3);
    ///Return a pointer to the 1D raw data at the the location (i1,i2,...,i6). The internal data may be efficiently read/written.
    double* dataPtr(int i1, int i2, int i3, int i4, int i5 = 0, int i6 = 0);

    ///Retrieve a chunk of the vectorized data of size 1xN starting at position i
    void getChunk(Mda& ret, int i, int N);
    ///Retrieve a chunk of the vectorized data of size N1xN2 starting at position (i1,i2)
    void getChunk(Mda& ret, int i1, int i2, int N1, int N2);
    ///Retrieve a chunk of the vectorized data of size N1xN2xN3 starting at position (i1,i2,i3)
    void getChunk(Mda& ret, int i1, int i2, int i3, int size1, int size2, int size3);

    ///Set a chunk of the vectorized data starting at position i
    void setChunk(Mda& X, int i);
    ///Set a chunk of the vectorized data starting at position (i1,i2)
    void setChunk(Mda& X, int i1, int i2);
    ///Set a chunk of the vectorized data of size N1xN2xN3 starting at position (i1,i2,i3)
    void setChunk(Mda& X, int i1, int i2, int i3);

    void reshape(int N1b, int N2b, int N3b = 1, int N4b = 1, int N5b = 1, int N6b = 1);

private:
    MdaPrivate* d;
};

#endif // MDA_H
