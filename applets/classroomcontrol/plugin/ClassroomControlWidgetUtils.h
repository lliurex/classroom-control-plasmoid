#ifndef PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H
#define PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDBusConnection>

#include <n4d.hpp>
#include <variant.hpp>

using namespace std;
using namespace edupals;
using namespace edupals::variant;


class ClassroomControlWidgetUtils : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.classroomcontrol.Bridge")


public:
   

   ClassroomControlWidgetUtils(QObject *parent = nullptr);

   void cleanCache();
   QVariantList getCurrentCart();
   int getMaxNumCart();
   bool showWidget();
   bool isClassroomControlAvailable();
   QVariantList getApplyChangesResult(QString stout,QString stderr);
   bool isAdi();
   int getDeactivationTimeOut();
   bool automaticDeactivation();
   bool reactivateControl(int cart);


   QString user;
   int maxNumCart=0;
   Variant cartInfo =Variant::create_array(0);
   QString controlModeVar="/var/lib/n4d/variables/CLASSROOM";
   QString natfreeServer="/usr/bin/natfree-adi";
   QString hideAppletVar="/var/lib/n4d/variables/HIDE_CLASSROOM_APPLET";
   QString automaticDeactivationConfig="/etc/classroom-plasmoid.cfg";
   int defaultDeactivationTimeOut=3600000;


public slots:
    
    void cancelDeactivation();
    void launchDeactivation();

signals:

    void closeWarning();

private:    
     
    n4d::Client client;
    QFile TARGET_FILE;
    QString getInstalledVersion();
    bool getHideAppletValue();
    void registerService();

     
signals:
    void cancelDeactivationSignal();
    void launchDeactivationSignal();

};

#endif // PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H
