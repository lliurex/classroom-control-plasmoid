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

bool ClassroomControlWidgetUtils::isClassroomControlAvailable(){

    TARGET_FILE.setFileName("/usr/bin/natfree-server");
    bool isAvailable=false;

    if (TARGET_FILE.exists()){
        TARGET_FILE.setFileName(controlModeVar);
        if (TARGET_FILE.exists()){
            auto[isError,cart]=getCurrentCart();
            if (!isError){
                if (cart==0){
                    isAvailable=false;
                }else{
                    isAvailable=true;
                }
            }
        }
    }
    
    qDebug()<<"[CLASSROOM_CONTROL]: Classroom Control Available: "<<isAvailable;
    return isAvailable;

}

std::tuple<bool, int> ClassroomControlWidgetUtils::getCurrentCart(){

    bool isError=false;
    QString currentCart="0";

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
   return {isError,numCart};

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
std::tuple<bool, int> ClassroomControlWidgetUtils::getApplyChangesResult(QString stdout,QString stderr){

    bool isError=false;
    int errorCode=0;

    QStringList lstout=stdout.split("\n");
    QStringList lsterr=stderr.split("\n");
   
    qDebug()<<"[CLASSROOM_CONTROL]: Updating CART Output: "<<lstout;
    qDebug()<<"[CLASSROOM_CONTROL]: Updating CART Error: "<<lsterr;

    if (lsterr[0]!=""){
        isError=true;
        if (lsterr[0].contains("ip") || lsterr[0].contains("hosts")){
            errorCode=-1;
        }else if (lsterr[0].contains("Mask")){
            errorCode=-2;
        }else if (lsterr[0].contains("Cart")){
            errorCode=-3;
        }else if (lsterr[0].contains("authorized")){
            isError=false;
        }
    }else{
        isError=false;
    }

    return {isError,errorCode};

}
