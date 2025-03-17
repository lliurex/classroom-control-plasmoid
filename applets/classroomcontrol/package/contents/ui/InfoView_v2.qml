import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.classroomcontrol 1.0

// Item - the most basic plasmoid component, an empty container.

Item {

    PC3.Page {

        anchors.centerIn:parent
        implicitWidth: PlasmaCore.Units.gridUnit * 12
        implicitHeight: PlasmaCore.Units.gridUnit * 6

        PlasmaExtras.PlaceholderMessage {
            id:phMsg
            anchors.centerIn: parent
            width: parent.width*2-(PlasmaCore.Units.gridUnit * 4)
            iconName: Plasmoid.icon
            text:Plasmoid.toolTipSubText
        }
        Row{
            anchors.top:phMsg.bottom
            anchors.horizontalCenter:phMsg.horizontalCenter
            spacing:20
            visible:classroomControlWidget.canEdit

            PC3.Button{
                id:unlockBtn
                text:i18n("Unlock cart")
                icon.name:"document-decrypt.svg"
                KeyNavigation.right:settingsBtn
                enabled:classroomControlWidget.isCartControlEnabled?true:false
                onClicked: classroomControlWidget.unlockCart()
            }

            PC3.Button {
                id:settingsBtn
                display:AbstractButton.TextBesideIcon
                icon.name:"configure"
                text:i18n("Configuration")
                onClicked:{
                    classroomControlWidget.manageNavigation(1)
                }
            }
        } 
    }
 }	
