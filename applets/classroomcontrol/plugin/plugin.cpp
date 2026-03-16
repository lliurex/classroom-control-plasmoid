#include "plugin.h"
#include "ClassroomControlWidget.h"

#include <QtQml>

void ClassroomControlPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.classroomcontrol"));
    qmlRegisterType<ClassroomControlWidget>(uri, 2, 0, "ClassroomControlWidget");
}
