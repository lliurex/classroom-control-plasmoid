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
    TARGET_FILE.setFileName("/usr/bin/natfree-server");
    notificationTitle=i18n("Mobile Classroom Control");
    setSubToolTip(notificationTitle);
    plasmoidMode();

}  

void ClassroomControlWidget::plasmoidMode(){

	if (m_utils->showWidget()){
        if (TARGET_FILE.exists()){
    		createWatcher();
            updateInfo();
    	}else{
            notificationBody=i18n("Mobile Classroom Control not available in this computer");
            m_canEdit=false;
            setSubToolTip(notificationBody);
    		changeTryIconState(1);
    	}
    }else{
        m_canEdit=false;
        changeTryIconState(1);
    }

}

void ClassroomControlWidget::createWatcher(){

    watcher=new QFileSystemWatcher(this);
    TARGET_FILE.setFileName(controlModeVar);
    if (!TARGET_FILE.exists()){
        if (!createdFolderWatcher){
            createdFolderWatcher=true;
            connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(updateInfo()));
            watcher->addPath(n4dVarsPath);
        }
    }else{
        if (!createFileWatcher){
            createFileWatcher=true;
            connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(updateInfo()));
            watcher->addPath(controlModeVar);
        }
    }
} 

void ClassroomControlWidget::updateInfo(){

    if (!isWorking){
        isWorking=true;
        bool enable=false;
        TARGET_FILE.setFileName(controlModeVar);
        if (TARGET_FILE.exists()){
            qDebug()<<"Updating info...";
            auto[isError,cart]=m_utils->getCurrentCart();
            initCart=cart;
            m_maxNumCart=m_utils->getMaxNumCart();
        	
            if (initCart>0){
                enable=true;
            }
            createWatcher();
        }else{
            if (createFileWatcher){
                createFileWatcher=false;
            }
        }
        if (enable){
            cartControlEnabled=true;
            QString title=i18n("Classroom control activated");
            setCurrentCartIndex(initCart-1);
            QString cart=QString::number(initCart);
            setIsCartControlEnabled(true);
            QString tmpIcon="classroom_control_cart_";
            tmpIcon.append(QString("%1").arg(cart));
            setIconName(tmpIcon);
            notificationBody=i18n("Controlling cart number: ")+cart;
            setSubToolTip(title+'\n'+notificationBody);          
        }else{
            cartControlEnabled=false;
            QString title=i18n("Classroom control disabled");
            setCurrentCartIndex(initCart);
            setIsCartControlEnabled(false);
            setIconName("classroom_control_off");
            setSubToolTip(title);
        }
        changeTryIconState(0);
        m_canEdit=true;
        isWorking=false;
    }
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

    int newCart=0;

    if (m_isCartControlEnabled){
        newCart=m_currentCart;
    }

    cartControlEnabled=m_isCartControlEnabled;

    m_utils->updateCart(newCart);
    if (!isWorking){
        updateInfo();
        setArePendingChanges(false);
        setCurrentStackIndex(0);
    }
}

void ClassroomControlWidget::cancelChanges(){
	
    if (!isWorking){
        updateInfo();
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

int ClassroomControlWidget::maxNumCart(){

    return m_maxNumCart;
}

