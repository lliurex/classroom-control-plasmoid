import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PC3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.classroomcontrol 1.0

// Item - the most basic plasmoid component, an empty container.

Item {

    PC3.Page {

        anchors.centerIn:parent
        implicitWidth: Kirigami.Units.gridUnit * 12
        implicitHeight: Kirigami.Units.gridUnit * 6

        PlasmaExtras.PlaceholderMessage {
            id:phMsg
            anchors.centerIn: parent
            width: parent.width*2-(Kirigami.Units.gridUnit * 4)
            iconName: classroomControlWidget.iconNamePh
            text:classroomControlWidget.subToolTip
        }

        PC3.Button {
            height:35
            anchors.top:phMsg.bottom
            anchors.horizontalCenter:phMsg.horizontalCenter
            display:AbstractButton.TextBesideIcon
            visible:classroomControlWidget.canEdit
            icon.name:"configure"
            text:i18n("Configuration")
            onClicked:{
                classroomControlWidget.manageNavigation(1)
            }
        } 
    }
 }	
