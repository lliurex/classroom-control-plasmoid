#ifndef CLASSROOM_CONTROL_PLUGIN_H
#define CLASSROOM_CONTROL_PLUGIN_H

#include <QQmlEngineExtensionPlugin>

class ClassroomControlPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
    using QQmlEngineExtensionPlugin::QQmlEngineExtensionPlugin;
};
#endif // CLASSROOM_CONTROL_PLUGIN_H
