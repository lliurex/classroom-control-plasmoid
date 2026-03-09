#include "ClassroomControlWidgetAdaptor.h"
#include "ClassroomControlWidgetUtils.h"


ClassroomControlWidgetAdaptor::ClassroomControlWidgetAdaptor(ClassroomControlWidgetUtils *parent)
    : QDBusAbstractAdaptor(parent)
{
    connect(parent,&ClassroomControlWidgetUtils::closeWarningSignal,this,&ClassroomControlWidgetAdaptor::closeWarningSignal);
    connect(parent,&ClassroomControlWidgetUtils::cancelDeactivationSignal,this,&ClassroomControlWidgetAdaptor::cancelDeactivationSignal);
    connect(parent,&ClassroomControlWidgetUtils::launchDeactivationSignal,this,&ClassroomControlWidgetAdaptor::launchDeactivationSignal);

}
void ClassroomControlWidgetAdaptor::cancelDeactivation(){

    static_cast<ClassroomControlWidgetUtils*>(parent())->cancelDeactivation();

}

void ClassroomControlWidgetAdaptor::launchDeactivation(){

    static_cast<ClassroomControlWidgetUtils*>(parent())->launchDeactivation();

}

