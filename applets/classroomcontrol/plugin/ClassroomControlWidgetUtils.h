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


public:
   

   explicit ClassroomControlWidgetUtils(QObject *parent = nullptr);

   void cleanCache();
   bool isAdi();

   QVariantList getApplyChangesResult(QString stout,QString stderr);
   bool reactivateControl(int cart);
   void getWidgetStatus();
   void getCurrentInfo();
   bool automaticDeactivation();



   QString user;
   QString controlModeVar="/var/lib/n4d/variables/CLASSROOM";


public slots:
    
    void cancelDeactivation();
    void launchDeactivation();

signals:
    
    void getWidgetStatusFinished (bool isEnabled, int deactivationTimeOut);
    void getCurrentInfoFinished (bool isAvailable, bool isEnabled, int cartConfigured, int maxNumCart);
    void closeWarningSignal();

private:    
     
    n4d::Client client;
    QFile TARGET_FILE;
    int maxNumCart=0;
    Variant cartInfo =Variant::create_array(0);
    QString natfreeServer="/usr/bin/natfree-adi";
    QString hideAppletVar="/var/lib/n4d/variables/HIDE_CLASSROOM_APPLET";
    QString automaticDeactivationConfig="/etc/classroom-plasmoid.cfg";
    int defaultDeactivationTimeOut=3600000;

    QString getInstalledVersion();
    bool getHideAppletValue();
    void registerService();
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
