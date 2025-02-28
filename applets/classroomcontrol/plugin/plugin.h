#ifndef CLASSROOM_CONTROL_PLUGIN_H
#define CLASSROOM_CONTROL_PLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class ClassroomControlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
/*    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")*/
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) Q_DECL_OVERRIDE;
};

#endif // LLIUREX_QUOTA_WATCH_PLUGIN_H
