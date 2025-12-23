#include "ClassroomControlWidgetUtils.h"

#include <QFile>
#include <QDateTime>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDebug>
#include <QTextStream>
#include <QJsonObject>
#include <QList>
#include <KLocalizedString>
#include <sys/types.h>
#include <QDBusConnection>

#include <grp.h>
#include <pwd.h>
#include <n4d.hpp>
#include <variant.hpp>
#include <json.hpp>

#include <tuple>
#include <sys/types.h>
#include <QDebug>

using namespace edupals;
using namespace std;
using namespace edupals::variant;


ClassroomControlWidgetUtils::ClassroomControlWidgetUtils(QObject *parent)
    : QObject(parent)
       
{
    user=qgetenv("USER");
    
    n4d::Client tmpClient=n4d::Client("https://127.0.0.1:9779",user.toStdString(),"");
    n4d::Ticket ticket=tmpClient.create_ticket();
    tmpClient=n4d::Client(ticket);
    client=tmpClient;
    registerService();
}

void ClassroomControlWidgetUtils::cleanCache(){

    QFile CURRENT_VERSION_TOKEN;
    QDir cacheDir("/home/"+user+"/.cache/plasmashell/qmlcache");
    QDir warningCache("/home/"+user+"/.cache/classroom-control-dialog.py");
    QString currentVersion="";
    bool clear=false;

    CURRENT_VERSION_TOKEN.setFileName("/home/"+user+"/.config/classroom-control-widget.conf");
    QString installedVersion=getInstalledVersion();

    if (!CURRENT_VERSION_TOKEN.exists()){
        if (CURRENT_VERSION_TOKEN.open(QIODevice::WriteOnly)){
            QTextStream data(&CURRENT_VERSION_TOKEN);
            data<<installedVersion;
            CURRENT_VERSION_TOKEN.close();
            clear=true;
        }
    }else{
        if (CURRENT_VERSION_TOKEN.open(QIODevice::ReadOnly)){
            QTextStream content(&CURRENT_VERSION_TOKEN);
            currentVersion=content.readLine();
            CURRENT_VERSION_TOKEN.close();
        }

        if (currentVersion!=installedVersion){
            if (CURRENT_VERSION_TOKEN.open(QIODevice::WriteOnly)){
                QTextStream data(&CURRENT_VERSION_TOKEN);
                data<<installedVersion;
                CURRENT_VERSION_TOKEN.close();
                clear=true;
            }
        }
    } 
    if (clear){
        if (cacheDir.exists()){
            cacheDir.removeRecursively();
        }

        if (warningCache.exists()){
            warningCache.removeRecursively();
        }
    }   

}

QString ClassroomControlWidgetUtils::getInstalledVersion(){

    QFile INSTALLED_VERSION_TOKEN;
    QString installedVersion="";
    
    INSTALLED_VERSION_TOKEN.setFileName("/var/lib/classroom-control-plasmoid/version");

    if (INSTALLED_VERSION_TOKEN.exists()){
        if (INSTALLED_VERSION_TOKEN.open(QIODevice::ReadOnly)){
            QTextStream content(&INSTALLED_VERSION_TOKEN);
            installedVersion=content.readLine();
            INSTALLED_VERSION_TOKEN.close();
        }
    }
    return installedVersion;

}  

void ClassroomControlWidgetUtils::registerService(){

    QDBusConnection bus=QDBusConnection::sessionBus();
    bus.registerService("com.classroomcontrol.DeactivationWarning");
    bus.registerObject("/DeactivationWarning",this,QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
}


bool ClassroomControlWidgetUtils::showWidget(){

    int j, ngroups=32;
    gid_t groups[32];
    struct passwd *pw;
    struct group *gr;

    QByteArray uname = user.toLocal8Bit();
    const char *username = uname.data();
    pw=getpwnam(username);
    getgrouplist(username, pw->pw_gid, groups, &ngroups);
    for (j = 0; j < ngroups; j++) {
        gr = getgrgid(groups[j]);
        if (gr != NULL){
            if ((strcmp(gr->gr_name,"adm")==0)||(strcmp(gr->gr_name,"admins")==0)||(strcmp(gr->gr_name,"teachers")==0)){
                 return true;
            } 
        }  
    }

    return false; 
}   

bool ClassroomControlWidgetUtils::isClassroomControlAvailable(){

    TARGET_FILE.setFileName(natfreeServer);
    bool isAvailable=false;

    if (isAdi()){
        if (TARGET_FILE.exists()){
            if (!getHideAppletValue()){
                isAvailable=true;
            }
        }
    }
    
    qDebug()<<"[CLASSROOM_CONTROL]: Classroom Control Available: "<<isAvailable;
    return isAvailable;

}

QVariantList ClassroomControlWidgetUtils::getCurrentCart(){

    bool isError=false;
    QString currentCart="-1";
    QVariantList result;

    try{
        variant::Variant cartInfo = client.get_variable("CLASSROOM",true);
        auto tmpCart=cartInfo["value"];
        
        if (tmpCart.size()>0){
            currentCart=QString::fromStdString(cartInfo["value"].get_string());
            qDebug()<<"[CLASSROOM_CONTROL]: Reading CLASSROOM var: "<<currentCart;
        }else{
            currentCart="-1";
            qDebug()<<"[CLASSROOM_CONTROL]: Reading CLASSROOM var: ''";

        }
    }catch (std::exception& e){
        qDebug()<<"[CLASSROOM_CONTROL]: Error reading CLASSROOM var: " <<e.what();
        isError=true;

    } 
   
   int numCart=currentCart.toInt();
   
   result.append(isError);
   result.append(numCart);
   return result;

}

int ClassroomControlWidgetUtils::getMaxNumCart(){

    QProcess process;
    QString cmd="nfctl get MAX_NUM_CART_DISPLAY";
    process.start("/bin/sh", QStringList()<< "-c" 
                       << cmd);
    process.waitForFinished(-1);
    QString stdout=process.readAllStandardOutput();
    QString stderr=process.readAllStandardError();
    QStringList lst=stdout.split("\n");

    if (lst.length()>0){
        maxNumCart=lst[0].toInt()-1;
    }

    return maxNumCart;
}
QVariantList ClassroomControlWidgetUtils::getApplyChangesResult(QString stdout,QString stderr){

    bool isError=false;
    int errorCode=0;
    QString msgText="";
    QVariantList result;

    QStringList lstout=stdout.split("\n");
    QStringList lsterr=stderr.split("\n");
   
    qDebug()<<"[CLASSROOM_CONTROL]: Updating CART Output: "<<lstout;
    qDebug()<<"[CLASSROOM_CONTROL]: Updating CART Error: "<<lsterr;

    if (lsterr[0]!=""){
        isError=true;
        if (lsterr[0].contains("ip")){
            errorCode=-1;
            msgText=i18n("Unable to get ip from interface");
        }else if (lsterr[0].contains("Mask")){
            errorCode=-2;
            msgText=i18n("Mask value from interface is wrong");
        }else if (lsterr[0].contains("Cart")){
            errorCode=-3;
            msgText=i18n("The selected cart is already beaing controlled by another computer");
        }else if (lsterr[0].contains("hosts")){
            errorCode=-4;
            msgText=i18n("Insufficient number of hosts in subnet");
        }else if (lsterr[0].contains("Virtual")){
            errorCode=-5;
            msgText=i18n("Virtual interface not created");
        }else if (lsterr[0].contains("authorized")){
            errorCode=-7;
        }else{
            errorCode=-6;
            msgText=i18n("Unable to configure classroom control");
        }
    }else{
        isError=false;
    }
    result.append(isError);
    result.append(errorCode);
    result.append(msgText);
    
    return result;

}

bool ClassroomControlWidgetUtils::getHideAppletValue(){

    bool hideApplet=false;

    TARGET_FILE.setFileName(hideAppletVar);

    if (TARGET_FILE.exists()){

        try{
            variant::Variant appletInfo = client.get_variable("HIDE_CLASSROOM_APPLET",true);
            hideApplet=appletInfo["value"];
            qDebug()<<"[CLASSROOM_CONTROL]: Reading HIDE_CLASSROOM_APPLET var: "<<QString::number(hideApplet);
        }catch (std::exception& e){
            qDebug()<<"[CLASSROOM_CONTROL]: Error reading HIDE_CLASSROOM_APPLET var: " <<e.what();
        } 
   }
   
   return hideApplet;


}

bool ClassroomControlWidgetUtils::isAdi(){

    bool matchAdi=false;
    QProcess process;
    QStringList flavours;
    QString cmd="lliurex-version -v";
    process.start("/bin/sh",QStringList()<<"-c"<<cmd);
    process.waitForFinished(-1);
    QString stdout=process.readAllStandardOutput();
    QString stderr=process.readAllStandardError();
    flavours=stdout.split('\n');


    for (int i=0;i<flavours.count();i++){
        if (flavours[i].contains("adi")){
            matchAdi=true;
            break;
        }
    }

    return matchAdi;

}

int ClassroomControlWidgetUtils::getDeactivationTimeOut(){

    QFile CUSTOM_DEACTIVATION_TIMEOUT;
    QString customTimeOut;
    int deactivationTimeOut=defaultDeactivationTimeOut;
    
    CUSTOM_DEACTIVATION_TIMEOUT.setFileName(automaticDeactivationConfig);

    if (CUSTOM_DEACTIVATION_TIMEOUT.exists()){
        if (CUSTOM_DEACTIVATION_TIMEOUT.open(QIODevice::ReadOnly)){
            QTextStream content(&CUSTOM_DEACTIVATION_TIMEOUT);
            customTimeOut=content.readLine();
            CUSTOM_DEACTIVATION_TIMEOUT.close();
            try{
                deactivationTimeOut=customTimeOut.toInt()*60*1000;
            }catch(std::exception& e){
                qDebug()<<"[CLASSROOM_CONTROL]: getDeactivationTimeOut. Error: "<<e.what();
            }
        }
    }
    if (deactivationTimeOut==0){
        deactivationTimeOut=defaultDeactivationTimeOut;
    }
    return deactivationTimeOut;

}

bool ClassroomControlWidgetUtils::automaticDeactivation(){

    bool result=false;
    try{
        variant::Variant ret=client.call("NatfreeADI","unset");
        result=ret;
        qDebug()<<"[CLASSROOM_CONTROL]: Automatic deactivation. Result: "<<result;
    }catch(std::exception& e){
        qDebug()<<"[CLASSROOM_CONTROL]: Automatic deactivation. Error: "<<e.what();
    }

    return result;
}

bool ClassroomControlWidgetUtils::reactivateControl(int cart){

    bool result=false;
    try{
        vector<variant::Variant>params={cart};
        variant::Variant ret=client.call("NatfreeADI","set",params);
        result=ret;
        qDebug()<<"[CLASSROOM_CONTROL]: Reactivation control. Result: "<<result;
    }catch(std::exception& e){
        qDebug()<<"[CLASSROOM_CONTROL]: Reactivation control. Error: "<<e.what();
    }

    return result;
}

void ClassroomControlWidgetUtils::cancelDeactivation(){

    emit cancelDeactivationSignal();
}

void ClassroomControlWidgetUtils::launchDeactivation(){

    emit launchDeactivationSignal();
}

