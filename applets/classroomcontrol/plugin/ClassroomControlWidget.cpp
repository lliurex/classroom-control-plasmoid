#include "ClassroomControlWidget.h"
#include "ClassroomControlWidgetUtils.h"

#include <KLocalizedString>
#include <KFormat>
#include <KNotification>
#include <KRun>
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
    connect(m_applyChanges, (void (QProcess::*)(int, QProcess::ExitStatus))&QProcess::finished,
            this, &ClassroomControlWidget::applyChangesFinished);

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
        setCanEdit(false);
        setCurrentStackIndex(0);
        changeTryIconState(1);
    }

}

void ClassroomControlWidget::createWatcher(){

    watcher=new QFileSystemWatcher(this);
    TARGET_FILE.setFileName(m_utils->controlModeVar);

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
        TARGET_FILE.setFileName(m_utils->controlModeVar);
        if (TARGET_FILE.exists()){
            qDebug()<<"[CLASSROOM_CONTROL]: Updating info...";
            auto[isError,cart]=m_utils->getCurrentCart();
            initCart=cart;
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
                notificationBody=i18n("Controlling cart number: ")+cart;
                setSubToolTip(title+'\n'+notificationBody); 
                m_notification=KNotification::event(QStringLiteral("Set"),title,notificationBody,tmpIcon,nullptr,KNotification::CloseOnTimeout,QStringLiteral("classroomcontrol"));
         
            }else{
                cartControlEnabled=false;
                title=i18n("Classroom control disabled");
                setCurrentCartIndex(0);
                setIsCartControlEnabled(false);
                setIconName("classroom_control_off");
                setIconNamePh("classroom_control_off");
                setSubToolTip(title);
                m_notification=KNotification::event(QStringLiteral("Unset"),title,"","classroom_control_off",nullptr,KNotification::CloseOnTimeout,QStringLiteral("classroomcontrol"));

            }
            changeTryIconState(0);
            setCanEdit(true);
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

    if (isCartControlEnabled!=cartControlEnabled){
        setArePendingChanges(true);
    }else{
        setArePendingChanges(false);
    }
    setIsCartControlEnabled(isCartControlEnabled);
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

    cartControlEnabled=m_isCartControlEnabled;

    if (m_applyChanges->state() != QProcess::NotRunning) {
        m_applyChanges->kill();
    }

    if (m_isCartControlEnabled){
        newCart=QString::number(m_currentCart);
        qDebug()<<"[CLASSROOM_CONTROL]: Apply changes. New Cart: "<<newCart;
        cmd="pkexec natfree-server -v natfree00 SET "+newCart;
    }else{
        qDebug()<<"[CLASSROOM_CONTROL]: Apply changes. Disable classroom control";
        cmd="pkexec natfree-server -v natfree00 UNSET ";
    }
    m_applyChanges->start("/bin/sh", QStringList()<< "-c" 
                       << cmd,QIODevice::ReadOnly);

}

void ClassroomControlWidget::applyChangesFinished(int exitCode, QProcess::ExitStatus exitStatus){    
   
    Q_UNUSED(exitCode);
    bool isError=false;
    int code=0;
    
    if (exitStatus!=QProcess::NormalExit){
        isError=true;
        code=-4;
    }else{
        QString stdout=QString::fromLocal8Bit(m_applyChanges->readAllStandardOutput());
        QString stderr=QString::fromLocal8Bit(m_applyChanges->readAllStandardError());

        auto[tmpError,tmpCode]=m_utils->getApplyChangesResult(stdout, stderr);
        isError=tmpError;
        code=tmpCode;
    }
    
    if (isError){
        if (code==-1){
            notificationBody=i18n("Unable to get ip from interface");
        }else if (code==-2){
            notificationBody=i18n("Mask value from interface is wrong");
        }else if (code==-3){
            notificationBody=i18n("The selected cart is already beaing controlled by another computer");
        }else if (code==-4){
            notificationBody=i18n("Unable to configure classroom control");
        }

        setShowWaitMsg(false);
        setMsgCode(0);
        setArePendingChanges(false);
        setShowError(true);
        setErrorCode(code);
        setIconName("classroom_control_error");
        setIconNamePh("classroom_control_error");
        title=i18n("Error configuring classroom control");
        setSubToolTip(title+'\n'+notificationBody);
        m_notification=KNotification::event(QStringLiteral("Error"),title,notificationBody,"classroom_control_error",nullptr,KNotification::CloseOnTimeout,QStringLiteral("classroomcontrol"));

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

    QString cmd="pkexec natfree-server -v natfree00 UNSET ";
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

