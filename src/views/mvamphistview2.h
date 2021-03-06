#ifndef MVAMPHISTVIEW2_H
#define MVAMPHISTVIEW2_H

#include "mvhistogramgrid.h"
#include "mvabstractviewfactory.h"

class MVAmpHistView2Private;
class MVAmpHistView2 : public MVHistogramGrid {
    Q_OBJECT
public:
    friend class MVAmpHistView2Private;
    MVAmpHistView2(MVAbstractContext* context);
    virtual ~MVAmpHistView2();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void wheelEvent(QWheelEvent* evt);

signals:

private slots:
    void slot_zoom_in_horizontal(double factor = 1.2);
    void slot_zoom_out_horizontal(double factor = 1.2);
    void slot_pan_left(double units = 0.1);
    void slot_pan_right(double units = 0.1);

private:
    MVAmpHistView2Private* d;
};

class MVAmplitudeHistogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVAmplitudeHistogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    void slot_amplitude_histogram_activated();
};

#endif // MVAMPHISTVIEW2_H
