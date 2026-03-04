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
#include <QtConcurrent>

using namespace edupals;
using namespace std;
using namespace edupals::variant;


ClassroomControlWidget::ClassroomControlWidget(QObject *parent)
    : QObject(parent)
    , m_utils(new ClassroomControlWidgetUtils(this))
    , m_applyChanges(new QProcess(this))
    , m_timer_deactivation(new QTimer(this))
{
    m_utils->cleanCache();
    notificationTitle=i18n("Mobile Classroom Control");
    TARGET_VAR_FILE.setFileName(m_utils->controlModeVar);
    TARGET_DIR_N4DVARS.setPath(n4dVarPath);

    connect(m_utils,&ClassroomControlWidgetUtils::getWidgetStatusFinished,this,&ClassroomControlWidget::initPlasmoid);
    connect(m_timer_deactivation, &QTimer::timeout, this, &ClassroomControlWidget::showDeactivationWarning);
    connect(m_utils,&ClassroomControlWidgetUtils::getCurrentInfoFinished,this,&ClassroomControlWidget::updateInfo);
    connect(m_applyChanges, (void (QProcess::*)(int, QProcess::ExitStatus))&QProcess::finished,
            this, &ClassroomControlWidget::applyChangesFinished);
    connect(&m_changesWatcher,&QFutureWatcher<QVariantList>::finished,this,&ClassroomControlWidget::handleProcessingFinished);               
    connect(&m_deactivationWatcher,&QFutureWatcher<bool>::finished,this,&ClassroomControlWidget::handleDeactivationFinished);               
    connect(&m_reactivationWatcher,&QFutureWatcher<bool>::finished,this,&ClassroomControlWidget::handleReactivationFinished);               
    connect(m_utils,&ClassroomControlWidgetUtils::cancelDeactivationSignal,this,&ClassroomControlWidget::stopDeactivation);
    connect(m_utils,&ClassroomControlWidgetUtils::launchDeactivationSignal,this,&ClassroomControlWidget::launchAutomaticDeactivation);
    setSubToolTip(notificationTitle);
    plasmoidMode();

}  

void ClassroomControlWidget::plasmoidMode(){

    m_utils->getWidgetStatus();
  
}

void ClassroomControlWidget::initPlasmoid(bool isEnabled, int timeOut){

    if (isEnabled){
        deactivationTimeOut=timeOut;
        qDebug()<<"[CLASSROOM_CONTROL]: TimeOut for automatic deactivation (in ms): "<<deactivationTimeOut;
        createWatcher();
        getInfo();
    }else{
        disableApplet();
    }

}

void ClassroomControlWidget::createWatcher(){

    watcher=new QFileSystemWatcher(this);

    if (TARGET_DIR_N4DVARS.exists()){
        if (!createDirectoryN4dWatcher){
            createDirectoryN4dWatcher=true;
            connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(getInfo()));
            watcher->addPath(n4dVarPath);
        }
        if (TARGET_VAR_FILE.exists()){
            if (!createFileVarWatcher){
                createFileVarWatcher=true;
                connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(getInfo()));
                watcher->addPath(m_utils->controlModeVar);
            }

        }
    }
    
} 

void ClassroomControlWidget::getInfo(){

    if (!isWorking){
        qDebug()<<"[CLASSROOM_CONTROL]: Detecting changed in n4d vars directory";
        isWorking=true;
        m_utils->getCurrentInfo();
    }
}

void ClassroomControlWidget::updateInfo(bool isAvailable, bool isEnabled, int cartConfigured, int maxNumCart){

    if (isAvailable){
        m_maxNumCart=maxNumCart;
        createWatcher();

        if (previousCart!=cartConfigured){
            previousCart=cartConfigured;
            showNotification=true;
            if (deactivationTimerLaunched){
                m_timer_deactivation->stop();
                m_timer_deactivation->start(deactivationTimeOut);
            }
        }else{
            showNotification=false;
        }
        qDebug()<<"[CLASSROOM_CONTROL]: Updating info";
        if (isEnabled){
            lastCartConfigured=cartConfigured;
            if (!deactivationTimerLaunched){
                m_timer_deactivation->start(deactivationTimeOut);
                deactivationTimerLaunched=true;
            }
            cartControlEnabled=true;
            title=i18n("Classroom control activated");
            setCurrentCartIndex(cartConfigured-1);
            QString cart=QString::number(cartConfigured);
            setIsCartControlEnabled(true);
            QString tmpIcon="classroom_control_cart_";
            tmpIcon.append(QString("%1").arg(cart));
            setIconName(tmpIcon);
            setIconNamePh("classroom_control");
            notificationBody=i18n("Controlling the cart number: ")+cart;
            setSubToolTip(title+'\n'+notificationBody); 
            if (showNotification){
                closeAllNotifications();
                KNotification *m_notification = new KNotification(QStringLiteral("Set"),KNotification::CloseOnTimeout,this);
                m_notification->setComponentName(QStringLiteral("classroomcontrol"));
                m_notification->setTitle(title);
                m_notification->setText(notificationBody);
                m_notification->setIconName(tmpIcon);
                m_notification->sendEvent();
            } 
        }else{
            cartConfigured=0;
            cartControlEnabled=false;
            createFileVarWatcher=false;
            m_timer_deactivation->stop();
            deactivationTimerLaunched=false;
            title=i18n("Classroom control disabled");
            setCurrentCart(1);
            setCurrentCartIndex(0);
            setIsCartControlEnabled(false);
            setIconName("classroom_control_off");
            setIconNamePh("classroom_control_off");
            setSubToolTip(title);
            if (automaticallyDeactivated){
                automaticallyDeactivated=false;
                if (!m_reactivationNotification){
                    closeAllNotifications();
                    QString bodyWarning=i18n("Automatically disabled");
                    KNotification *m_reactivationNotification=new KNotification(QStringLiteral("Warning"),KNotification::Persistent,this);
                    m_reactivationNotification->setComponentName(QStringLiteral("classroomcontrol"));
                    m_reactivationNotification->setTitle(title);
                    m_reactivationNotification->setText(bodyWarning);
                    m_reactivationNotification->setIconName("classroom_control_off");
                    QString action=i18n("Reactivate control of cart: ")+QString::number(lastCartConfigured);
                    auto reactivationAction=m_reactivationNotification->addAction(action);
                    connect(reactivationAction,&KNotificationAction::activated,this,&ClassroomControlWidget::reactivateControl);
                    m_reactivationNotification->sendEvent();
                }
            }else{    
                if (showNotification){
                    closeAllNotifications();
                    KNotification *m_notification = new KNotification(QStringLiteral("Unset"),KNotification::CloseOnTimeout,this);
                    m_notification->setComponentName(QStringLiteral("classroomcontrol"));
                    m_notification->setTitle(title);
                    m_notification->setText("");
                    m_notification->setIconName("classroom_control_off");
                    m_notification->sendEvent();
                }
            }
        }
        changeTryIconState(0);
        setCanEdit(true);
        showNotification=true;
        isWorking=false;

    }else{
        qDebug()<<"[CLASSROOM_CONTROL]: Updating info";
        disableApplet();
        isWorking=false; 
    }
}

void ClassroomControlWidget::disableApplet(){

    notificationBody=i18n("Mobile Classroom Control not available in this computer");
    m_timer_deactivation->stop();
    deactivationTimerLaunched=false;
    setCanEdit(false);
    setIconName("classroom_control");
    setIconNamePh("classroom_control");
    setSubToolTip(notificationBody);
    setCurrentStackIndex(0);
    changeTryIconState(2);

}

ClassroomControlWidget::TrayStatus ClassroomControlWidget::status() const
{
    return m_status;
}

void ClassroomControlWidget::changeTryIconState(int state){

    if (state==0){
    	setStatus(ActiveStatus);
        setToolTip(notificationTitle);
    }else if (state==1){
        setStatus(PassiveStatus);
    }else if (state==2){
        setStatus(HiddenStatus);
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

    if (newCart!=cartConfigured){
        setArePendingChanges(true);
    }else{
        setArePendingChanges(false);
    }

   setCurrentCart(newCart);
   setCurrentCartIndex(newCart-1);
}

void ClassroomControlWidget::applyChanges(){

    if (m_utils->isAdi()){

        emit m_utils->closeWarningSignal();

        closeAllNotifications();
        setShowError(false);
        setShowWaitMsg(true);
        setMsgCode(2);

        QString newCart="";
        QString cmd="";

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
    }else{
        disableApplet();
    }

}

void ClassroomControlWidget::applyChangesFinished(int exitCode, QProcess::ExitStatus exitStatus){    
   
    Q_UNUSED(exitCode);
   
    showNotification=true;
    
    if (exitStatus!=QProcess::NormalExit){
        int code=-6;
        notificationBody=i18n("Unable to configure classroom control");
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
            KNotification *m_notification = new KNotification(QStringLiteral("Error"),KNotification::CloseOnTimeout,this);
            m_notification->setComponentName(QStringLiteral("classroomcontrol"));
            m_notification->setTitle(title);
            m_notification->setText(notificationBody);
            m_notification->setIconName("classroom_control_error");
            m_notification->sendEvent();
        }

    }else{
        QString stdout=QString::fromLocal8Bit(m_applyChanges->readAllStandardOutput());
        QString stderr=QString::fromLocal8Bit(m_applyChanges->readAllStandardError());

        auto future= QtConcurrent::run([this,stdout,stderr](){
            return m_utils->getApplyChangesResult(stdout, stderr);
        });
        m_changesWatcher.setFuture(future);
    }
}

void ClassroomControlWidget::handleProcessingFinished(){

    QVariantList ret=m_changesWatcher.result();
    bool isError=ret[0].toBool();
    int code=ret[1].toInt();
    notificationBody=ret[2].toString();

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
                KNotification *m_notification = new KNotification(QStringLiteral("Error"),KNotification::CloseOnTimeout,this);
                m_notification->setComponentName(QStringLiteral("classroomcontrol"));
                m_notification->setTitle(title);
                m_notification->setText(notificationBody);
                m_notification->setIconName("classroom_control_error");
                m_notification->sendEvent();
            }
        }
    
    }else{
        if (!createDirectoryN4dWatcher || !createFileVarWatcher){
            if (!isWorking){
                getInfo();
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

    closeAllNotifications();

    setShowError(false);
    setShowWaitMsg(true);
    setMsgCode(3);
    
    showNotification=false;
    
    if (!isWorking){
        getInfo();
        setShowWaitMsg(false);
        setMsgCode(0);
        setArePendingChanges(false);
        setCurrentStackIndex(0);
    }
}

void ClassroomControlWidget::unlockCart(){

    if (m_utils->isAdi()){
        qDebug()<<"[CLASSROOM_CONTROL]: Unlock cart ...";

       emit m_utils->closeWarningSignal();
       closeAllNotifications();

       setShowError(false);
       setShowWaitMsg(true);
       setMsgCode(2);

       QString cmd="pkexec natfree-adi UNSET ";
       m_applyChanges->start("/bin/sh", QStringList()<< "-c" 
                           << cmd,QIODevice::ReadOnly);
    }else{
        disableApplet();
    }

}

void ClassroomControlWidget::showDeactivationWarning(){

    closeAllNotifications();
    m_timer_deactivation->stop();
  
    QString command="/usr/bin/classroom-control-dialog.py";
    KIO::CommandLauncherJob *job = nullptr;
    job = new KIO::CommandLauncherJob(command);
    job->start();

             
}

void ClassroomControlWidget::stopDeactivation(){

    closeAllNotifications();

    if (cartControlEnabled){
        m_timer_deactivation->stop();
        m_timer_deactivation->start(deactivationTimeOut);
    }

}

void ClassroomControlWidget::launchAutomaticDeactivation(){

    m_timer_deactivation->stop();
    automaticallyDeactivated=true;
    setShowError(false);
    setShowWaitMsg(true);
    setMsgCode(4);
   
    auto future=QtConcurrent::run([this](){
        return m_utils->automaticDeactivation();
    });
    m_deactivationWatcher.setFuture(future);
}

void ClassroomControlWidget::handleDeactivationFinished(){
     
    bool ret=m_deactivationWatcher.result();

    if (!ret){
        closeAllNotifications();
        QString titleError=i18n("Automatic deactivation has failed");
        QString bodyError=i18n("Classroom control remains active");
        KNotification *m_notification=new KNotification(QStringLiteral("Error"),KNotification::CloseOnTimeout,this);
        m_notification->setComponentName(QStringLiteral("classroomcontrol"));
        m_notification->setTitle(titleError);
        m_notification->setText(bodyError);
        m_notification->setIconName("classroom_control_error");
        m_notification->sendEvent();
        m_notification->sendEvent();
    }
    setShowWaitMsg(false);
    setMsgCode(0);

}

void ClassroomControlWidget::reactivateControl(){

    setShowError(false);
    setShowWaitMsg(true);
    setMsgCode(5);
    cartControlEnabled=true;

    auto future=QtConcurrent::run([this](){
        return m_utils->reactivateControl(this->lastCartConfigured);
    });
    m_reactivationWatcher.setFuture(future);
    
   
}

void ClassroomControlWidget::handleReactivationFinished(){

   bool ret=m_reactivationWatcher.result();

   if (!ret){
        closeAllNotifications();
        QString titleError=i18n("The reactivation has failed");
        QString bodyError=i18n("Classroom control remains deactivate");
        KNotification *m_notification=new KNotification(QStringLiteral("Error"),KNotification::CloseOnTimeout,this);
        m_notification->setComponentName(QStringLiteral("classroomcontrol"));
        m_notification->setTitle(titleError);
        m_notification->setText(bodyError);
        m_notification->setIconName("classroom_control_error");
        m_notification->sendEvent();
       
   }
   setShowWaitMsg(false);
   setMsgCode(0);

}

void ClassroomControlWidget::manageNavigation(int stackIndex)
{
    if (m_utils->isAdi()){
        if (stackIndex==0 && m_arePendingChanges){
            if (!m_showError){
                cancelChanges();
            }
        }else{
            setCurrentStackIndex(stackIndex);
        }
    }else{
        disableApplet();
    }
}

void ClassroomControlWidget::closeAllNotifications(){

    qDebug()<<"[CLASSROOM_CONTROL]: Clossing all notifications...";
   
    if (m_notification){
        m_notification->close();
    }

    if (m_reactivationNotification){
        m_reactivationNotification->close();
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

