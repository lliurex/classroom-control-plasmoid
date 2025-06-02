#ifndef PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H
#define PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QDir>

#include <n4d.hpp>
#include <variant.hpp>

using namespace std;
using namespace edupals;
using namespace edupals::variant;


class ClassroomControlWidgetUtils : public QObject
{
    Q_OBJECT


public:
   

   ClassroomControlWidgetUtils(QObject *parent = nullptr);

   void cleanCache();
   QVariantList getCurrentCart();
   int getMaxNumCart();
   bool showWidget();
   bool isClassroomControlAvailable();
   QVariantList getApplyChangesResult(QString stout,QString stderr);

   QString user;
   int maxNumCart=0;
   Variant cartInfo =Variant::create_array(0);
   QString controlModeVar="/var/lib/n4d/variables/CLASSROOM";
   QString natfreeServer="/usr/bin/natfree-adi";
   QString hideAppletVar="/var/lib/n4d/variables/HIDE_CLASSROOM_APPLET";

private:    
     
    n4d::Client client;
    QFile TARGET_FILE;
    QString getInstalledVersion();
    bool getHideAppletValue();

     
};



#endif // PLASMA_CLASSROOM_CONTROL_WIDGET_UTILS_H
