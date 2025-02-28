#include "plugin.h"
#include "ClassroomControlWidget.h"
#include "ClassroomControlWidgetUtils.h"

#include <QtQml>

void ClassroomControlPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.classroomcontrol"));
    qmlRegisterType<ClassroomControlWidget>(uri, 1, 0, "ClassroomControlWidget");
    qmlRegisterType<ClassroomControlWidgetUtils>(uri, 1, 0, "ClassroomControlWidgetUtils");
}
