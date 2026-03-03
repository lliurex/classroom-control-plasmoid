#ifndef PLASMA_CLASSROOM_CONTROL_WIDGET_ADAPTOR_H
#define PLASMA_CLASSROOM_CONTROL_WIDGET_ADAPTOR_H

#include <QDBusAbstractAdaptor>
#include "ClassroomControlWidgetUtils.h"


class ClassroomControlWidgetAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.classroomcontrol.Bridge")


public:

   explicit ClassroomControlWidgetAdaptor(ClassroomControlWidgetUtils *parent);

  
public slots:
    
    void cancelDeactivation();
    void launchDeactivation();

signals:
    
    void cancelDeactivationSignal();
    void launchDeactivationSignal();
    void closeWarningSignal();
};

#endif // PLASMA_CLASSROOM_CONTROL_WIDGET_ADAPTOR_H
