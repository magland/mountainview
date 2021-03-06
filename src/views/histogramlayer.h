/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef HISTOGRAMLAYER_H
#define HISTOGRAMLAYER_H

#include "mvabstractcontext.h"
#include "paintlayer.h"

#include <QObject>

/** \class HistogramLayer
 *  \brief Layer of a histogram from an arbitrary data array
 */
class HistogramLayerPrivate;
class HistogramLayer : public PaintLayer {
    Q_OBJECT
public:
    enum TimeScaleMode {
        Uniform,
        Log,
        Cubic
    };

    friend class HistogramLayerPrivate;
    explicit HistogramLayer(QObject* parent = 0);
    virtual ~HistogramLayer();

    void setData(const QVector<double>& values); // The data to view
    void setSecondData(const QVector<double>& values);
    void setBinInfo(double bin_min, double bin_max, int num_bins); //Set evenly spaced bins
    void setFillColor(const QColor& col); // The color for filling the histogram bars
    void setLineColor(const QColor& col); // The edge color for the bars
    void setTitle(const QString& title);
    void setCaption(const QString& caption);
    void setColors(const QMap<QString, QColor>& colors); // Controls background and highlighting colors. For consistent app look.
    void setTimeScaleMode(TimeScaleMode mode);
    void setTimeConstant(double timepoints);

    MVRange xRange() const;
    void setXRange(MVRange range);
    void autoCenterXRange(); //centers range based on data
    void setDrawVerticalAxisAtZero(bool val);
    void setVerticalLines(const QList<double>& vals);
    void setTickMarks(const QList<double>& vals);

    void setCurrent(bool val); // Set this as the current histogram (affects highlighting)
    void setSelected(bool val); // Set this as among the selected histograms (affects highlighting)

    void paint(QPainter* painter) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;

private:
    HistogramLayerPrivate* d;
};

#endif // HISTOGRAMLAYER_H
