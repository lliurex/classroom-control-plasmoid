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
    
{
    m_utils->cleanCache();
    notificationTitle=i18n("Mobile Classroom Control");
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
                QString title=i18n("Classroom control activated");
                setCurrentCartIndex(initCart-1);
                QString cart=QString::number(initCart);
                setIsCartControlEnabled(true);
                QString tmpIcon="classroom_control_cart_";
                tmpIcon.append(QString("%1").arg(cart));
                setIconName(tmpIcon);
                setIconNamePh("classroom_control");
                notificationBody=i18n("Controlling cart number: ")+cart;
                setSubToolTip(title+'\n'+notificationBody);          
            }else{
                cartControlEnabled=false;
                QString title=i18n("Classroom control disabled");
                setCurrentCartIndex(0);
                setIsCartControlEnabled(false);
                setIconName("classroom_control_off");
                setIconNamePh("classroom_control_off");
                setSubToolTip(title);
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
}

void ClassroomControlWidget::applyChanges(){

    setShowWaitMsg(true);
    setMsgCode(2);

    int newCart=0;

    if (m_isCartControlEnabled){
        newCart=m_currentCart;
    }

    cartControlEnabled=m_isCartControlEnabled;

    m_utils->updateCart(newCart);
    
   if (!isWorking){
        updateInfo();
        setShowWaitMsg(false);
        setMsgCode(0);
        setArePendingChanges(false);
        setCurrentStackIndex(0);
    }
}

void ClassroomControlWidget::cancelChanges(){
	
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

    qDebug()<<"Unlock cart";
}

void ClassroomControlWidget::manageNavigation(int stackIndex)
{
    if (stackIndex==0 && m_arePendingChanges){
        cancelChanges();
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

    qDebug()<<"Detectado cambio";
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

