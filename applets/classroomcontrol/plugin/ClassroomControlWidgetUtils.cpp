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
#include <sys/types.h>

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
    n4d::Client client;
    client=n4d::Client("https://127.0.0.1:9779");
  
}

void ClassroomControlWidgetUtils::cleanCache(){

    QFile CURRENT_VERSION_TOKEN;
    QDir cacheDir("/home/"+user+"/.cache/plasmashell/qmlcache");
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

int ClassroomControlWidgetUtils::getCurrentCart(){

    try{
        variant::Variant modeInfo = client.get_variable("CLASSROOM",true);
        currentCart=modeInfo["value"]; 
    }catch (...){
        qDebug()<<"error";

    } 
   qDebug()<<"CLASROOM"<<currentCart;
   
   return currentCart;

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
        maxNumCart=lst[0].toInt();
    }

    return maxNumCart;
}

void ClassroomControlWidgetUtils::updateCart(int newCart){

    /*
    try{
        client.call("BellSchedulerManager","stop_bell");
    }catch(...){
        
    }
    */
    currentCart=newCart;
    qDebug()<<"Cambiar modo"<<currentCart;

    
}
