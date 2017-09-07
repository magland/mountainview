QT += core gui network qml widgets concurrent

DEFINES += MOUNTAINLAB_SRC_PATH=\\\"$$PWD/../../mountainlab\\\"


CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

#TODO: Do we need openmp for mountainview?
CONFIG += mlcommon mvcommon openmp taskprogress

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainview
TEMPLATE = app

RESOURCES += mountainview.qrc

#######################################################################################################
INCLUDEPATH += mlcommon/include
VPATH += mlcommon/include
VPATH += mlcommon/src
HEADERS += mlcommon.h sumit.h \
    mlcommon/include/mda/mda32.h \
    mlcommon/include/mda/diskreadmda32.h \
    mlcommon/include/mda/mda_p.h \
    mlcommon/include/mliterator.h \
    mlcommon/include/mda/mdareader.h \
    mlcommon/include/mda/mdareader_p.h \
    mlcommon/include/objectregistry.h \
    mlcommon/include/mlprivate.h \
    mlcommon/include/icounter.h \
    mlcommon/include/qprocessmanager.h \
    mlcommon/include/signalhandler.h \
    mlcommon/include/mllog.h \
    mlcommon/include/tracing/tracing.h \
    mlcommon/include/mlvector.h \
    mlcommon/include/get_sort_indices.h

SOURCES += \
    mlcommon.cpp sumit.cpp \
    mda/mda32.cpp \
    mda/diskreadmda32.cpp \
    objectregistry.cpp \
    mda/mdareader.cpp \
    componentmanager/icomponent.cpp \
    icounter.cpp \
    qprocessmanager.cpp \
    signalhandler.cpp \
    mllog.cpp \
    tracing/tracing.cpp \
    mlvector.cpp \
    get_sort_indices.cpp

INCLUDEPATH += mlcommon/include/mda
VPATH += mlcommon/include/mda
VPATH += mlcommon/src/mda
HEADERS += diskreadmda.h diskwritemda.h mda.h mdaio.h remotereadmda.h usagetracking.h
SOURCES += diskreadmda.cpp diskwritemda.cpp mda.cpp mdaio.cpp remotereadmda.cpp usagetracking.cpp

INCLUDEPATH += mlcommon/include/cachemanager
VPATH += mlcommon/include/cachemanager
VPATH += mlcommon/src/cachemanager
HEADERS += cachemanager.h
SOURCES += cachemanager.cpp

INCLUDEPATH += mlcommon/include/taskprogress
VPATH += mlcommon/include/taskprogress
VPATH += mlcommon/src/taskprogress
HEADERS += taskprogress.h
SOURCES += taskprogress.cpp

INCLUDEPATH += mlcommon/include/mlnetwork
VPATH += mlcommon/include/mlnetwork
VPATH += mlcommon/src/mlnetwork
HEADERS += mlnetwork.h
SOURCES += mlnetwork.cpp

INCLUDEPATH += mlcommon/include/componentmanager
VPATH += mlcommon/include/componentmanager
VPATH += mlcommon/src/componentmanager
HEADERS += componentmanager.h icomponent.h
SOURCES += componentmanager.cpp


###################################################################################################

INCLUDEPATH += mvcommon/include
VPATH += mvcommon/include mvcommon/src
HEADERS += prvmanagerdialog.h resolveprvsdialog.h
SOURCES += prvmanagerdialog.cpp resolveprvsdialog.cpp

FORMS += prvmanagerdialog.ui resolveprvsdialog.ui

INCLUDEPATH += mvcommon/include/misc
VPATH += mvcommon/src/misc mvcommon/include/misc
HEADERS += \
clustermerge.h \
mvmisc.h mvutils.h paintlayer.h paintlayerstack.h renderablewidget.h jscounter.h
SOURCES += \
clustermerge.cpp \
mvmisc.cpp mvutils.cpp paintlayer.cpp paintlayerstack.cpp renderablewidget.cpp jscounter.cpp

INCLUDEPATH += mvcommon/include/core
VPATH += mvcommon/src/core mvcommon/include/core
HEADERS += \
closemehandler.h flowlayout.h imagesavedialog.h \
mountainprocessrunner.h mvabstractcontextmenuhandler.h \
mvabstractcontrol.h mvabstractview.h mvabstractviewfactory.h \
mvcontrolpanel2.h mvstatusbar.h \
tabber.h tabberframe.h taskprogressview.h actionfactory.h mvabstractplugin.h \
mvabstractcontext.h mvmainwindow.h

SOURCES += \
closemehandler.cpp flowlayout.cpp imagesavedialog.cpp \
mountainprocessrunner.cpp mvabstractcontextmenuhandler.cpp \
mvabstractcontrol.cpp mvabstractview.cpp mvabstractviewfactory.cpp \
mvcontrolpanel2.cpp mvstatusbar.cpp \
tabber.cpp tabberframe.cpp taskprogressview.cpp actionfactory.cpp mvabstractplugin.cpp \
mvabstractcontext.cpp mvmainwindow.cpp

INCLUDEPATH += mvcommon/3rdparty/qaccordion/include
VPATH += mvcommon/3rdparty/qaccordion/include
VPATH += mvcommon/3rdparty/qaccordion/src
HEADERS += qAccordion/qaccordion.h qAccordion/contentpane.h qAccordion/clickableframe.h
SOURCES += qaccordion.cpp contentpane.cpp clickableframe.cpp

RESOURCES += mvcommon.qrc \
    mvcommon/3rdparty/qaccordion/icons/qaccordionicons.qrc







####################################################################################################

INCLUDEPATH += msv/plugins msv/views
VPATH += msv/plugins msv/views

HEADERS += clusterdetailplugin.h clusterdetailview.h \
    clusterdetailviewpropertiesdialog.h \
    msv/views/matrixview.h \
    msv/views/isolationmatrixview.h \
    msv/plugins/isolationmatrixplugin.h \
    msv/views/clustermetricsview.h msv/views/clusterpairmetricsview.h \
    msv/plugins/clustermetricsplugin.h \
    msv/views/curationprogramview.h \
    msv/plugins/curationprogramplugin.h \
    msv/views/curationprogramcontroller.h \
    views/mvgridview.h \
    views/mvgridviewpropertiesdialog.h \
    controlwidgets/createtimeseriesdialog.h
SOURCES += clusterdetailplugin.cpp clusterdetailview.cpp \
    clusterdetailviewpropertiesdialog.cpp \
    msv/views/matrixview.cpp \
    msv/views/isolationmatrixview.cpp \
    msv/plugins/isolationmatrixplugin.cpp \
    msv/views/clustermetricsview.cpp msv/views/clusterpairmetricsview.cpp \
    msv/plugins/clustermetricsplugin.cpp \
    msv/views/curationprogramview.cpp \
    msv/plugins/curationprogramplugin.cpp \
    msv/views/curationprogramcontroller.cpp \
    views/mvgridview.cpp \
    views/mvgridviewpropertiesdialog.cpp \
    controlwidgets/createtimeseriesdialog.cpp
FORMS += clusterdetailviewpropertiesdialog.ui \
    controlwidgets/createtimeseriesdialog.ui

HEADERS += clipsviewplugin.h mvclipsview.h
SOURCES += clipsviewplugin.cpp mvclipsview.cpp

HEADERS += clustercontextmenuplugin.h
SOURCES += clustercontextmenuplugin.cpp


SOURCES += mountainviewmain.cpp \
    #guides/individualmergedecisionpage.cpp \
    views/mvspikespraypanel.cpp \
    views/firetrackview.cpp \
    views/ftelectrodearrayview.cpp \
    controlwidgets/mvmergecontrol.cpp \
    controlwidgets/mvprefscontrol.cpp \
    views/mvpanelwidget.cpp views/mvpanelwidget2.cpp \
    views/mvtemplatesview2.cpp \
    views/mvtemplatesview3.cpp \
    views/mvtemplatesview2panel.cpp \


HEADERS += mvcontext.h
SOURCES += mvcontext.cpp

INCLUDEPATH += multiscaletimeseries
VPATH += multiscaletimeseries
HEADERS += multiscaletimeseries.h
SOURCES += multiscaletimeseries.cpp

HEADERS += \
    #guides/individualmergedecisionpage.h \
    views/mvspikespraypanel.h \
    views/firetrackview.h \
    views/ftelectrodearrayview.h \
    controlwidgets/mvmergecontrol.h \
    controlwidgets/mvprefscontrol.h \
    views/mvpanelwidget.h views/mvpanelwidget2.h \
    views/mvtemplatesview2.h \
    views/mvtemplatesview3.h \
    views/mvtemplatesview2panel.h \


# to remove
#HEADERS += computationthread.h
#SOURCES += computationthread.cpp

#HEADERS += guides/guidev1.h guides/guidev2.h
#SOURCES += guides/guidev1.cpp guides/guidev2.cpp

INCLUDEPATH += views
VPATH += views
HEADERS += \
correlationmatrixview.h histogramview.h mvamphistview2.h mvamphistview3.h histogramlayer.h \
mvclipswidget.h \
mvclusterview.h mvclusterwidget.h mvcrosscorrelogramswidget3.h \
mvdiscrimhistview.h mvfiringeventview2.h mvhistogramgrid.h \
mvspikesprayview.h mvtimeseriesrendermanager.h mvtimeseriesview2.h \
mvtimeseriesviewbase.h spikespywidget.h \
#mvdiscrimhistview_guide.h \
mvclusterlegend.h
SOURCES += \
correlationmatrixview.cpp histogramview.cpp mvamphistview2.cpp mvamphistview3.cpp histogramlayer.cpp \
mvclipswidget.cpp \
mvclusterview.cpp mvclusterwidget.cpp mvcrosscorrelogramswidget3.cpp \
mvdiscrimhistview.cpp mvfiringeventview2.cpp mvhistogramgrid.cpp \
mvspikesprayview.cpp mvtimeseriesrendermanager.cpp mvtimeseriesview2.cpp \
mvtimeseriesviewbase.cpp spikespywidget.cpp \
#mvdiscrimhistview_guide.cpp \
mvclusterlegend.cpp

INCLUDEPATH += controlwidgets
VPATH += controlwidgets
HEADERS += \
mvclustervisibilitycontrol.h \
mvgeneralcontrol.h mvtimeseriescontrol.h mvopenviewscontrol.h mvclusterordercontrol.h
SOURCES += \
mvclustervisibilitycontrol.cpp \
mvgeneralcontrol.cpp mvtimeseriescontrol.cpp mvopenviewscontrol.cpp mvclusterordercontrol.cpp

INCLUDEPATH += controlwidgets/mvexportcontrol
VPATH += controlwidgets/mvexportcontrol
HEADERS += mvexportcontrol.h
SOURCES += mvexportcontrol.cpp

#INCLUDEPATH += guides
#VPATH += guides
#HEADERS += clustercurationguide.h
#SOURCES += clustercurationguide.cpp

## TODO: REMOVE THIS:
HEADERS += computationthread.h
SOURCES += computationthread.cpp

INCLUDEPATH += msv/contextmenuhandlers
VPATH += msv/contextmenuhandlers
HEADERS += clustercontextmenuhandler.h clusterpaircontextmenuhandler.h
SOURCES += clustercontextmenuhandler.cpp clusterpaircontextmenuhandler.cpp

INCLUDEPATH += mountainsort/utils
DEPENDPATH += mountainsort/utils
VPATH += mountainsort/utils
HEADERS += msmisc.h
SOURCES += msmisc.cpp
HEADERS += get_pca_features.h get_principal_components.h eigenvalue_decomposition.h
SOURCES += get_pca_features.cpp get_principal_components.cpp eigenvalue_decomposition.cpp
HEADERS += affinetransformation.h
SOURCES += affinetransformation.cpp
HEADERS += compute_templates_0.h
SOURCES += compute_templates_0.cpp

INCLUDEPATH += mountainsort/processors
DEPENDPATH += mountainsort/processors
VPATH += mountainsort/processors
HEADERS += extract_clips.h
SOURCES += extract_clips.cpp

#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

FORMS += \
    views/mvgridviewpropertiesdialog.ui

DISTFILES += \
    msv/views/curationprogram.js


