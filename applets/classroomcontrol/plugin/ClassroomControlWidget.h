#ifndef PLASMA_CLASSROOM_CONTROL_WIDGET_H
#define PLASMA_CLASSROOM_CONTROL_WIDGET_H

#include <QObject>
#include <QProcess>
#include <QPointer>
#include <KNotification>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QFileSystemWatcher>
#include <KIO/CommandLauncherJob>

#include <variant.hpp>

#include "ClassroomControlWidgetUtils.h"

using namespace edupals;
using namespace edupals::variant;

class QTimer;
class KNotification;
class AsyncDbus;


class ClassroomControlWidget : public QObject
{
    Q_OBJECT


    Q_PROPERTY(TrayStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString toolTip READ toolTip NOTIFY toolTipChanged)
    Q_PROPERTY(QString subToolTip READ subToolTip NOTIFY subToolTipChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(QString iconNamePh READ iconNamePh NOTIFY iconNamePhChanged)
    Q_PROPERTY(int currentCart READ currentCart NOTIFY currentCartChanged)
    Q_PROPERTY(int currentStackIndex READ currentStackIndex NOTIFY currentStackIndexChanged)
    Q_PROPERTY(int currentCartIndex READ currentCartIndex NOTIFY currentCartIndexChanged)
    Q_PROPERTY (bool isCartControlEnabled READ isCartControlEnabled NOTIFY isCartControlEnabledChanged)
    Q_PROPERTY(bool arePendingChanges READ arePendingChanges NOTIFY arePendingChangesChanged)
    Q_PROPERTY(bool canEdit READ canEdit NOTIFY canEditChanged)
    Q_PROPERTY(int maxNumCart READ maxNumCart CONSTANT)
    Q_PROPERTY(bool showError READ showError NOTIFY showErrorChanged)
    Q_PROPERTY(int errorCode READ errorCode NOTIFY errorCodeChanged)
    Q_PROPERTY(bool showWaitMsg READ showWaitMsg NOTIFY showWaitMsgChanged)
    Q_PROPERTY(int msgCode READ msgCode NOTIFY msgCodeChanged)
    Q_ENUMS(TrayStatus)

public:
    /**
     * System tray icon states.
     */
    enum TrayStatus {
        ActiveStatus=0,
        PassiveStatus,
        HiddenStatus
    };

    ClassroomControlWidget(QObject *parent = nullptr);

    TrayStatus status() const;
    void changeTryIconState (int state);
    void setStatus(TrayStatus status);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QString subToolTip() const;
    void setSubToolTip(const QString &subToolTip);

    QString iconName() const;
    void setIconName(const QString &name);

    QString iconNamePh() const;
    void setIconNamePh(const QString &name);

    int currentStackIndex();
    void setCurrentStackIndex(int);

    int currentCart();
    void setCurrentCart(int);

    int currentCartIndex();
    void setCurrentCartIndex(int);

    bool isCartControlEnabled();
    void setIsCartControlEnabled(bool);

    bool arePendingChanges();
    void setArePendingChanges(bool);

    bool showError();
    void setShowError(bool);

    int errorCode();
    void setErrorCode(int);

    bool canEdit();
    void setCanEdit(bool);

    int maxNumCart();

    bool showWaitMsg();
    void setShowWaitMsg(bool);

    int msgCode();
    void setMsgCode(int);

public slots:
    
    void updateInfo();
    void changeCart(int cart);
    void changeControlMode(bool isCartControlEnabled);
    void manageNavigation(int stackIndex);
    void applyChanges();
    void cancelChanges();
    void unlockCart();
    void openHelp();
  
signals:
   
    void toolTipChanged();
    void subToolTipChanged();
    void iconNameChanged();
    void iconNamePhChanged();
    void currentStackIndexChanged();
    void statusChanged();
    void currentCartChanged();
    void currentCartIndexChanged();
    void isCartControlEnabledChanged();
    void arePendingChangesChanged();
    void showErrorChanged();
    void errorCodeChanged();
    void canEditChanged();
    void showWaitMsgChanged();
    void msgCodeChanged();

private:

    TrayStatus m_status = PassiveStatus;
    QString m_iconName = QStringLiteral("classroom_control");
    QString m_iconNamePh = QStringLiteral("classroom_control");
    QString m_toolTip;
    QString m_subToolTip;
    int m_currentStackIndex=0;
    int m_currentCart=1;
    int m_currentCartIndex=0;
    bool m_isCartControlEnabled=false;
    bool m_arePendingChanges=false;
    bool m_canEdit=false;
    bool m_showError=false;
    int m_errorCode=0;
    bool isWorking=false;
    int m_maxNumCart=0;
    bool m_showWaitMsg=false;
    int m_msgCode=0;
    int cartConfigured=0;
    int previousCart=0;
    int lastCartConfigured=0;
    int deactivationTimeOut=3600000;
    bool cartControlEnabled=false;
    QString notificationTitle;
    QString notificationBody;
    QString title;
    QFile TARGET_VAR_FILE;
    QFile TARGET_FILE_ADI;
    QDir TARGET_DIR_N4DVARS;
    QString n4dVarPath="/var/lib/n4d/variables/";
    QFileSystemWatcher *watcher=nullptr;
    ClassroomControlWidgetUtils* m_utils;
    bool createFileVarWatcher=false;
    bool createDirectoryN4dWatcher=false;
    bool showNotification=true; 
    bool deactivationTimerLaunched=false;
    bool automaticallyDeactivated=false;
    QProcess *m_applyChanges=nullptr;
    QPointer<KNotification> m_notification;
    QPointer<KNotification> m_reactivationNotification;
    QTimer *m_timer_deactivation = nullptr;
    QList<QPointer <KNotification>> activeNotifications;
    bool deleteNoticationWorker=false;
    int maxManualCloseNotifications=4;
    int manualCloseNotificationsCount=0; 
    void plasmoidMode();
    void createWatcher();
    void disableApplet();
    void showDeactivationWarning();
    void launchAutomaticDeactivation();
    void stopDeactivation();
    void reactivateControl();
    void closeAllNotifications(QList<QPointer<KNotification>> openNotifications);
    void automaticDeactivation();
    void reactivate();

private slots:
    
    void applyChangesFinished(int exitCode, QProcess::ExitStatus exitStatus);

};


#endif // PLASMA_CLASSROOM_CONTROL_WIDGET_H
