/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef MVFIRINGEVENTVIEW2_H
#define MVFIRINGEVENTVIEW2_H

#include <QWidget>
#include <diskreadmda.h>
#include <paintlayer.h>
#include "mvabstractviewfactory.h"
#include "mvtimeseriesviewbase.h"

class MVFiringEventView2Private;
class MVFiringEventView2 : public MVTimeSeriesViewBase {
    Q_OBJECT
public:
    friend class MVFiringEventView2Private;
    MVFiringEventView2(MVAbstractContext* context);
    virtual ~MVFiringEventView2();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setLabelsToUse(const QSet<int>& labels_to_use);

    void paintContent(QPainter* painter);

    void setAmplitudeRange(MVRange range);
    void autoSetAmplitudeRange();

protected:
    void mouseMoveEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void resizeEvent(QResizeEvent* evt);

private slots:
    void slot_update_image();

private:
    MVFiringEventView2Private* d;
};

class MVFiringEventsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVFiringEventsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
};

class FiringEventImageCalculator;
class FiringEventContentLayer : public PaintLayer {
    Q_OBJECT
public:
    FiringEventContentLayer();
    virtual ~FiringEventContentLayer();

    void paint(QPainter* painter) Q_DECL_OVERRIDE;
    void setWindowSize(QSize size) Q_DECL_OVERRIDE;

    FiringEventImageCalculator* calculator;

    //void updateImage();

    QImage output_image;

private slots:
    void slot_calculator_finished();
};

#endif // MVFiringEventView2_H
