#include "ClassroomControlWidget.h"
#include "ClassroomControlWidgetUtils.h"

#include <KLocalizedString>
#include <KFormat>
#include <KNotification>
#include <QTimer>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QThread>
#include <QtCore/QStringList>
#include <QJsonObject>
#include <variant.hpp>
#include <json.hpp>
#include <QDebug>
#include <KIO/CommandLauncherJob>

using namespace edupals;
using namespace std;
using namespace edupals::variant;


ClassroomControlWidget::ClassroomControlWidget(QObject *parent)
    : QObject(parent)
    , m_utils(new ClassroomControlWidgetUtils(this))
    , m_applyChanges(new QProcess(this))
   
{
    m_utils->cleanCache();
    notificationTitle=i18n("Mobile Classroom Control");
    TARGET_FILE.setFileName(m_utils->controlModeVar);

    connect(m_applyChanges, (void (QProcess::*)(int, QProcess::ExitStatus))&QProcess::finished,
            this, &ClassroomControlWidget::applyChangesFinished);

    setSubToolTip(notificationTitle);
    plasmoidMode();

}  

void ClassroomControlWidget::plasmoidMode(){

    if (m_utils->showWidget()){
        if (TARGET_FILE.exists()){
            createWatcher();
        }
        if (m_utils->isClassroomControlAvailable()){
            updateInfo();
    	}else{
            disableApplet();
    	}
    }else{
        setCanEdit(false);
        setCurrentStackIndex(0);
        changeTryIconState(1);
    }

}

void ClassroomControlWidget::createWatcher(){

    watcher=new QFileSystemWatcher(this);

    if (TARGET_FILE.exists()){
        if (!createFileWatcher){
            createFileWatcher=true;
            connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(updateInfo()));
            watcher->addPath(m_utils->controlModeVar);
        }
    }else{
        createFileWatcher=false;
    }
} 

void ClassroomControlWidget::updateInfo(){

    if (!isWorking){
        qDebug()<<"[CLASSROOM_CONTROL]: Detecting changed in n4d vars directory";
        isWorking=true;
        bool enable=false;
        bool disable=false;
       
        if (TARGET_FILE.exists()){
            qDebug()<<"[CLASSROOM_CONTROL]: Updating info...";
            QVariantList ret=m_utils->getCurrentCart();
            initCart=ret[1].toInt();
            m_maxNumCart=m_utils->getMaxNumCart();
        	
            if (initCart==0){
                disable=true;
            }else{
                if (initCart>0){
                    enable=true;
                }
                createWatcher();
            }
        }else{
            disable=true;
        }

        if (disable){
            disableApplet();
            isWorking=false;
        }else{
            if (enable){
                cartControlEnabled=true;
                title=i18n("Classroom control activated");
                setCurrentCartIndex(initCart-1);
                QString cart=QString::number(initCart);
                setIsCartControlEnabled(true);
                QString tmpIcon="classroom_control_cart_";
                tmpIcon.append(QString("%1").arg(cart));
                setIconName(tmpIcon);
                setIconNamePh("classroom_control");
                notificationBody=i18n("Controlling the cart number: ")+cart;
                setSubToolTip(title+'\n'+notificationBody); 
                if (showNotification){
                    m_notification = new KNotification(QStringLiteral("Set"),KNotification::CloseOnTimeout,this);
                    m_notification->setComponentName(QStringLiteral("classroomcontrol"));
                    m_notification->setTitle(title);
                    m_notification->setText(notificationBody);
                    m_notification->setIconName(tmpIcon);
                    m_notification->sendEvent();
                }         
            
            }else{
                cartControlEnabled=false;
                title=i18n("Classroom control disabled");
                setCurrentCart(1);
                setCurrentCartIndex(0);
                setIsCartControlEnabled(false);
                setIconName("classroom_control_off");
                setIconNamePh("classroom_control_off");
                setSubToolTip(title);
                if (showNotification){
                    m_notification = new KNotification(QStringLiteral("Unset"),KNotification::CloseOnTimeout,this);
                    m_notification->setComponentName(QStringLiteral("classroomcontrol"));
                    m_notification->setTitle(title);
                    m_notification->setText("");
                    m_notification->setIconName("classroom_control_off");
                    m_notification->sendEvent();
                }
            }
            changeTryIconState(0);
            setCanEdit(true);
            showNotification=true;
            isWorking=false;
        }
    }
}

void ClassroomControlWidget::disableApplet(){

    notificationBody=i18n("Mobile Classroom Control not available in this computer");
   
    setCanEdit(false);
    setIconName("classroom_control");
    setIconNamePh("classroom_control");
    setSubToolTip(notificationBody);
    setCurrentStackIndex(0);
    changeTryIconState(1);

}

ClassroomControlWidget::TrayStatus ClassroomControlWidget::status() const
{
    return m_status;
}

void ClassroomControlWidget::changeTryIconState(int state){

    if (state==0){
    	setStatus(ActiveStatus);
        setToolTip(notificationTitle);
    }else{
        setStatus(PassiveStatus);
    }

}

void ClassroomControlWidget::changeControlMode(bool isCartControlEnabled){

    if (cartControlEnabled!=isCartControlEnabled){
        cartControlEnabled=isCartControlEnabled;
        setArePendingChanges(true);
    }else{
        setArePendingChanges(false);
    }
}

void ClassroomControlWidget::changeCart(int newCart){

    if (newCart!=initCart){
        setArePendingChanges(true);
    }else{
        setArePendingChanges(false);
    }

   setCurrentCart(newCart);
   setCurrentCartIndex(newCart-1);
}

void ClassroomControlWidget::applyChanges(){

    setShowError(false);
    setShowWaitMsg(true);
    setMsgCode(2);

    QString newCart="";
    QString cmd="";

    if (m_notification){
        m_notification->close();
    }

    if (m_applyChanges->state() != QProcess::NotRunning) {
        m_applyChanges->kill();
    }

    if (cartControlEnabled){
        newCart=QString::number(m_currentCart);
        qDebug()<<"[CLASSROOM_CONTROL]: Apply changes. New Cart: "<<newCart;
        cmd="pkexec natfree-adi CONFIGURE "+newCart;
    }else{
        qDebug()<<"[CLASSROOM_CONTROL]: Apply changes. Disable classroom control";
        cmd="pkexec natfree-adi UNSET";
    
    }
    m_applyChanges->start("/bin/sh", QStringList()<< "-c" 
                       << cmd,QIODevice::ReadOnly);

}

void ClassroomControlWidget::applyChangesFinished(int exitCode, QProcess::ExitStatus exitStatus){    
   
    Q_UNUSED(exitCode);
    bool isError=false;
    int code=0;

    showNotification=true;
    
    if (exitStatus!=QProcess::NormalExit){
        isError=true;
        code=-6;
        notificationBody=i18n("Unable to configure classroom control");

    }else{
        QString stdout=QString::fromLocal8Bit(m_applyChanges->readAllStandardOutput());
        QString stderr=QString::fromLocal8Bit(m_applyChanges->readAllStandardError());

        QVariantList ret=m_utils->getApplyChangesResult(stdout, stderr);
        isError=ret[0].toBool();
        code=ret[1].toInt();
        notificationBody=ret[2].toString();
    }
    
    if (isError){
        if (code==-7){  
            cancelChanges();
        }else{
            qDebug()<<"[CLASSROOM_CONTROL]: Apply changes with error. Code: "<<code;
            setShowWaitMsg(false);
            setMsgCode(0);
            setArePendingChanges(false);
            setShowError(true);
            setErrorCode(code);
            setIconName("classroom_control_error");
            setIconNamePh("classroom_control_error");
            title=i18n("Error configuring classroom control");
            setSubToolTip(title+'\n'+notificationBody);
            if (showNotification){
                m_notification = new KNotification(QStringLiteral("Error"),KNotification::CloseOnTimeout,this);
                m_notification->setComponentName(QStringLiteral("classroomcontrol"));
                m_notification->setTitle(title);
                m_notification->setText(notificationBody);
                m_notification->setIconName("classroom_control_error");
                m_notification->sendEvent();
            }
        }
    
    }else{
        if (!createFileWatcher){
            if (!isWorking){
                updateInfo();
            }
        }
        setShowWaitMsg(false);
        setMsgCode(0);
        setArePendingChanges(false);
        setCurrentStackIndex(0);
    }
   
}

void ClassroomControlWidget::cancelChanges(){

    qDebug()<<"[CLASSROOM_CONTROL]: Cancel changes ...";

    if (m_notification){
        m_notification->close();
    }

    setShowError(false);
    setShowWaitMsg(true);
    setMsgCode(3);
    
    showNotification=false;
    
    if (!isWorking){
        updateInfo();
        setShowWaitMsg(false);
        setMsgCode(0);
        setArePendingChanges(false);
        setCurrentStackIndex(0);
    }
}

void ClassroomControlWidget::unlockCart(){

    qDebug()<<"[CLASSROOM_CONTROL]: Unlock cart ...";

    if (m_notification){
        m_notification->close();
    }

    setShowError(false);
    setShowWaitMsg(true);
    setMsgCode(2);

    QString cmd="pkexec natfree-adi -v natfree00 UNSET ";
    m_applyChanges->start("/bin/sh", QStringList()<< "-c" 
                       << cmd,QIODevice::ReadOnly);

}

void ClassroomControlWidget::manageNavigation(int stackIndex)
{
    if (stackIndex==0 && m_arePendingChanges){
        if (!m_showError){
            cancelChanges();
        }
    }else{
        setCurrentStackIndex(stackIndex);
    }
}

void ClassroomControlWidget::openHelp(){

    QString command="xdg-open https://wiki.edu.gva.es/lliurex/tiki-index.php?page=Configuración-de-los-equipos-del-aula-móvil";
    KIO::CommandLauncherJob *job = nullptr;
    job = new KIO::CommandLauncherJob(command);
    job->start();
}

void ClassroomControlWidget::setStatus(ClassroomControlWidget::TrayStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

QString ClassroomControlWidget::iconName() const
{
    return m_iconName;
}

void ClassroomControlWidget::setIconName(const QString &name)
{
    if (m_iconName != name) {
        m_iconName = name;
        emit iconNameChanged();
    }
}

QString ClassroomControlWidget::iconNamePh() const
{
    return m_iconNamePh;
}

void ClassroomControlWidget::setIconNamePh(const QString &name)
{
    if (m_iconNamePh != name) {
        m_iconNamePh = name;
        emit iconNamePhChanged();
    }
}

QString ClassroomControlWidget::toolTip() const
{
    return m_toolTip;
}

void ClassroomControlWidget::setToolTip(const QString &toolTip)
{
    if (m_toolTip != toolTip) {
        m_toolTip = toolTip;
        emit toolTipChanged();
    }
}

QString ClassroomControlWidget::subToolTip() const
{
    return m_subToolTip;
}

void ClassroomControlWidget::setSubToolTip(const QString &subToolTip)
{
    if (m_subToolTip != subToolTip) {
        m_subToolTip = subToolTip;
        emit subToolTipChanged();
    }
}

int ClassroomControlWidget::currentStackIndex(){

    return m_currentStackIndex;
}

void ClassroomControlWidget::setCurrentStackIndex(int currentStackIndex){

    if (m_currentStackIndex!=currentStackIndex){
        m_currentStackIndex=currentStackIndex;
        emit currentStackIndexChanged();
    }
}

int ClassroomControlWidget::currentCart(){

    return m_currentCart;
}

void ClassroomControlWidget::setCurrentCart(int newCart){

    if (m_currentCart!=newCart){
        m_currentCart=newCart;
        emit currentCartChanged();
    }
}

int ClassroomControlWidget::currentCartIndex(){

    return m_currentCartIndex;
}

void ClassroomControlWidget::setCurrentCartIndex(int newCartIndex){

    if (m_currentCartIndex!=newCartIndex){
        m_currentCartIndex=newCartIndex;
        emit currentCartIndexChanged();
    }
}

bool ClassroomControlWidget::isCartControlEnabled(){

    return m_isCartControlEnabled;
}

void ClassroomControlWidget::setIsCartControlEnabled(bool isCartControlEnabled){

    if (m_isCartControlEnabled!=isCartControlEnabled){
        m_isCartControlEnabled=isCartControlEnabled;
        emit isCartControlEnabledChanged();
    }
}

bool ClassroomControlWidget::arePendingChanges(){

    return m_arePendingChanges;
}

void ClassroomControlWidget::setArePendingChanges(bool arePendingChanges){

    if (m_arePendingChanges!=arePendingChanges){
        m_arePendingChanges=arePendingChanges;
        emit arePendingChangesChanged();
    }
}

bool ClassroomControlWidget::showError(){

    return m_showError;
}

void ClassroomControlWidget::setShowError(bool showError){

    if (m_showError!=showError){
        m_showError=showError;
        emit showErrorChanged();
    }
}

int ClassroomControlWidget::errorCode(){

    return m_errorCode;
}

void ClassroomControlWidget::setErrorCode(int errorCode){

    if (m_errorCode!=errorCode){
        m_errorCode=errorCode;
        emit errorCodeChanged();
    }

}

bool ClassroomControlWidget::canEdit(){

    return m_canEdit;
}

void ClassroomControlWidget::setCanEdit(bool canEdit){

    if (m_canEdit!=canEdit){
        m_canEdit=canEdit;
        emit canEditChanged();
    }

}

int ClassroomControlWidget::maxNumCart(){

    return m_maxNumCart;
}

bool ClassroomControlWidget::showWaitMsg(){

    return m_showWaitMsg;
}

void ClassroomControlWidget::setShowWaitMsg(bool showWaitMsg){

    if (m_showWaitMsg!=showWaitMsg){
        m_showWaitMsg=showWaitMsg;
        emit showWaitMsgChanged();
    }
}

int ClassroomControlWidget::msgCode(){

    return m_msgCode;
}

void ClassroomControlWidget::setMsgCode(int msgCode){

    if (m_msgCode!=msgCode){
        m_msgCode=msgCode;
        emit msgCodeChanged();
    }
}

