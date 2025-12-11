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
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

using namespace edupals;
using namespace std;
using namespace edupals::variant;


ClassroomControlWidget::ClassroomControlWidget(QObject *parent)
    : QObject(parent)
    , m_utils(new ClassroomControlWidgetUtils(this))
    , m_applyChanges(new QProcess(this))
    , m_timer_deactivation(new QTimer(this))
    , m_timer_countdown(new QTimer(this))

   
{
    m_utils->cleanCache();
    notificationTitle=i18n("Mobile Classroom Control");
    TARGET_VAR_FILE.setFileName(m_utils->controlModeVar);
    TARGET_FILE_ADI.setFileName(m_utils->natfreeServer);
    TARGET_DIR_N4DVARS.setPath(n4dVarPath);
    deactivationTimeOut=m_utils->getDeactivationTimeOut();
    qDebug()<<"[CLASSROOM_CONTROL]: TimeOut for automatic deactivation (in ms): "<<deactivationTimeOut;

    connect(m_applyChanges, (void (QProcess::*)(int, QProcess::ExitStatus))&QProcess::finished,
            this, &ClassroomControlWidget::applyChangesFinished);

    connect(m_timer_deactivation, &QTimer::timeout, this, &ClassroomControlWidget::showDeactivationWarning);
    connect(m_timer_countdown, &QTimer::timeout, this, &ClassroomControlWidget::launchAutomaticDeactivation);

    setSubToolTip(notificationTitle);
    plasmoidMode();

}  

void ClassroomControlWidget::plasmoidMode(){

    if (m_utils->showWidget()){
        if (m_utils->isClassroomControlAvailable()){
            createWatcher();
            updateInfo();
    	}else{
            disableApplet();
    	}
    }else{
        disableApplet();
    }

}

void ClassroomControlWidget::createWatcher(){

    watcher=new QFileSystemWatcher(this);

    if (TARGET_DIR_N4DVARS.exists()){
        if (!createDirectoryN4dWatcher){
            createDirectoryN4dWatcher=true;
            connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(updateInfo()));
            watcher->addPath(n4dVarPath);
        }
        if (TARGET_VAR_FILE.exists()){
            if (!createFileVarWatcher){
                createFileVarWatcher=true;
                connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(updateInfo()));
                watcher->addPath(m_utils->controlModeVar);
            }

        }
    }
    
} 

void ClassroomControlWidget::updateInfo(){

    if (!isWorking){
        qDebug()<<"[CLASSROOM_CONTROL]: Detecting changed in n4d vars directory";
        isWorking=true;
        bool enable=false;
        bool disable=false;
       
        if (TARGET_FILE_ADI.exists() && m_utils->isAdi()){
            m_maxNumCart=m_utils->getMaxNumCart();

            if (TARGET_VAR_FILE.exists()){
                qDebug()<<"[CLASSROOM_CONTROL]: Updating info...";
                QVariantList ret=m_utils->getCurrentCart();
                cartConfigured=ret[1].toInt();
            	createWatcher();
                if (cartConfigured>0){
                    lastCartConfigured=cartConfigured;
                    enable=true;
                    if (!deactivationTimerLaunched){
                        m_timer_deactivation->start(deactivationTimeOut);
                        deactivationTimerLaunched=true;
                    }
                }else{
                    enable=false;
                }
            }else{
                cartConfigured=0;
                enable=false;
            }
            if (previousCart!=cartConfigured){
                previousCart=cartConfigured;
                showNotification=true;
                if (deactivationTimerLaunched){
                    m_timer_deactivation->stop();
                    m_timer_countdown->stop();
                    m_timer_deactivation->start(deactivationTimeOut);
                }
            }else{
                showNotification=false;
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
                    m_notification = new KNotification(QStringLiteral("Set"),KNotification::CloseOnTimeout,this);
                    m_notification->setComponentName(QStringLiteral("classroomcontrol"));
                    m_notification->setTitle(title);
                    m_notification->setText(notificationBody);
                    m_notification->setIconName(tmpIcon);
                    m_notification->sendEvent();
                }         
            
            }else{
                cartControlEnabled=false;
                createDirectoryN4dWatcher=false;
                createFileVarWatcher=false;
                m_timer_deactivation->stop();
                m_timer_countdown->stop();
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
                        m_reactivationNotification=new KNotification(QStringLiteral("Warning"),KNotification::Persistent,this);
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
                        m_notification = new KNotification(QStringLiteral("Unset"),KNotification::CloseOnTimeout,this);
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
        }
    }
}

void ClassroomControlWidget::disableApplet(){

    notificationBody=i18n("Mobile Classroom Control not available in this computer");
    m_timer_deactivation->stop();
    m_timer_countdown->stop();
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
        if (!createDirectoryN4dWatcher || !createFileVarWatcher){
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

    closeAllNotifications();

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

    if (m_utils->isAdi()){
       qDebug()<<"[CLASSROOM_CONTROL]: Unlock cart ...";

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
    if (!m_deactivationNotification){
	    QString titleWarning=i18n("Classroom control will be deactivate");
	    QString bodyWarning=i18n("Deactivation will be occur in a few seconds");
	   
	    m_deactivationNotification=new KNotification(QStringLiteral("Warning"),KNotification::Persistent,this);
	    m_deactivationNotification->setComponentName(QStringLiteral("classroomcontrol"));
	    m_deactivationNotification->setTitle(titleWarning);
	    m_deactivationNotification->setText(bodyWarning);
	    m_deactivationNotification->setIconName("classroom_control_error");
	    QString action=i18n("Cancel deactivation");
	    auto cancelAction=m_deactivationNotification->addAction(action);
	    connect(cancelAction,&KNotificationAction::activated,this,&ClassroomControlWidget::stopDeactivation);
	    m_deactivationNotification->sendEvent();
    }	
    m_timer_countdown->start(60000);
             
}

void ClassroomControlWidget::stopDeactivation(){

    closeAllNotifications();
    m_timer_countdown->stop();

}

void ClassroomControlWidget::launchAutomaticDeactivation(){

    closeAllNotifications();
    m_timer_deactivation->stop();
    automaticallyDeactivated=true;
    m_timer_countdown->stop();
    setShowError(false);
    setShowWaitMsg(true);
    setMsgCode(4);
   
    QFuture<void> future=QtConcurrent::run([this](){
        this->ClassroomControlWidget::automaticDeactivation();
    });

}

void ClassroomControlWidget::automaticDeactivation(){
     
    bool ret=m_utils->automaticDeactivation();

    if (!ret){
        QString titleError=i18n("Automatic deactivation has failed");
        QString bodyError=i18n("Classroom control remains active");
        m_notification=new KNotification(QStringLiteral("Error"),KNotification::CloseOnTimeout,this);
        m_notification->setComponentName(QStringLiteral("classroomcontrol"));
        m_notification->setTitle(titleError);
        m_notification->setText(bodyError);
        m_notification->setIconName("classroom_control_error");
        m_notification->sendEvent();
 
    }
    setShowWaitMsg(false);
    setMsgCode(0);

}

void ClassroomControlWidget::reactivateControl(){

    closeAllNotifications();
    setShowError(false);
    setShowWaitMsg(true);
    setMsgCode(5);
    cartControlEnabled=true;
    QFuture<void> future=QtConcurrent::run([this](){
        this->ClassroomControlWidget::reactivate();
    });
   
}

void ClassroomControlWidget::reactivate(){

   bool ret=m_utils->reactivateControl(lastCartConfigured);

   if (!ret){
      QString titleError=i18n("The reactivation has failed");
      QString bodyError=i18n("Classroom control remains deactivate");
      m_notification=new KNotification(QStringLiteral("Error"),KNotification::CloseOnTimeout,this);
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

    if (m_deactivationNotification){
        m_deactivationNotification->close();
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

