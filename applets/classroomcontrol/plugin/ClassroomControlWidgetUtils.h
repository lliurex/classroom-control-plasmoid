#ifndef PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H
#define PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDBusConnection>
#include <QMutex>
#include <QMutexLocker>

#include <n4d.hpp>

using namespace std;
using namespace edupals;
using namespace edupals::variant;


class ClassroomControlWidgetUtils : public QObject
{
    Q_OBJECT


public:
   

   explicit ClassroomControlWidgetUtils(QObject *parent = nullptr);

   void startWidget();
   bool isAdi();

   QVariantList getApplyChangesResult(QString stout,QString stderr);
   void getWidgetStatus();
   void getCurrentInfo();
   void reactivateControl(int cart);
   void automaticDeactivation();

   QString user;
   QString controlModeVar="/var/lib/n4d/variables/CLASSROOM";
   bool registeredService=true;


public slots:
    
    void cancelDeactivation();
    void launchDeactivation();

signals:
    
    void startWidgetFinished(bool startOk);
    void getWidgetStatusFinished (bool isEnabled, int deactivationTimeOut);
    void getCurrentInfoFinished (bool isAvailable, bool isEnabled, int cartConfigured, int maxNumCart);
    void automaticDeactivationFinished (bool result);
    void reactivateControlFinished (bool result);
    void closeWarningSignal();

private:    
     
    n4d::Client client;
    QMutex clientMutex;
    QFile TARGET_FILE;
    int maxNumCart=0;
    Variant cartInfo =Variant::create_array(0);
    QString natfreeServer="/usr/bin/natfree-adi";
    QString hideAppletVar="/var/lib/n4d/variables/HIDE_CLASSROOM_APPLET";
    QString automaticDeactivationConfig="/etc/classroom-plasmoid.cfg";
    int defaultDeactivationTimeOut=3600000;

    void cleanCache();
    QString getInstalledVersion();
    bool getHideAppletValue();
    bool registerService();
    QVariantList getCurrentCart();
    int getMaxNumCart();
    bool showWidget();
    bool isClassroomControlAvailable();
    int getDeactivationTimeOut();

signals:
    void cancelDeactivationSignal();
    void launchDeactivationSignal();

};

#endif // PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H
