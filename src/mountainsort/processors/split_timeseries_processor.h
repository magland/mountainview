#ifndef SPLIT_TIMESERIES_PROCESSOR_H
#define SPLIT_TIMESERIES_PROCESSOR_H

#include "msprocessor.h"

class concat_timeseries_ProcessorPrivate;
class concat_timeseries_Processor : public MSProcessor {
public:
    friend class concat_timeseries_ProcessorPrivate;
    concat_timeseries_Processor();
    virtual ~concat_timeseries_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    concat_timeseries_ProcessorPrivate* d;
};

class split_timeseries_ProcessorPrivate;
class split_timeseries_Processor : public MSProcessor {
public:
    friend class split_timeseries_ProcessorPrivate;
    split_timeseries_Processor();
    virtual ~split_timeseries_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    split_timeseries_ProcessorPrivate* d;
};

class split_firings_ProcessorPrivate;
class split_firings_Processor : public MSProcessor {
public:
    friend class split_firings_ProcessorPrivate;
    split_firings_Processor();
    virtual ~split_firings_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    split_firings_ProcessorPrivate* d;
};

#endif // SPLIT_TIMESERIES_PROCESSOR_H
