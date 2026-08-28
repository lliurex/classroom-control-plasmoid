import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PC3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

Rectangle {
    id: optionsContainer
    color: "transparent"
    enabled: !classroomControlWidget.showWaitMsg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        anchors.bottomMargin:15
        spacing: 15

        RowLayout {
            id: head
            Layout.fillWidth: true

            PC3.ToolButton {
                id: backBtn
                implicitHeight: 35
                implicitWidth: 35
                icon.name: "arrow-left"

                PC3.ToolTip {
                    id: backTP
                    text: i18n("Back to main view")
                }
                onClicked: {
                    backTP.hide()
                    classroomControlWidget.manageNavigation(0)
                }
            }

            PC3.Label {
                id: headText
                text: i18n("Classroom control settings")
                font.italic: true
                font.pointSize: 11
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Kirigami.InlineMessage {
            id: warningMsg
            Layout.fillWidth: true
            Layout.leftMargin:15
            Layout.rightMargin:15
            visible: classroomControlWidget.showError
            text: getTextMsg(classroomControlWidget.errorCode)
            type: Kirigami.MessageType.Error
        }

        GridLayout {
            id: controlOptions
            Layout.fillWidth: true
            Layout.leftMargin:15
            columns: 2
            columnSpacing: 10
            rowSpacing: 10
            visible: !classroomControlWidget.showWaitMsg

            PC3.CheckBox {
                id: cartModeCB
                checked: classroomControlWidget.isCartControlEnabled
                text: i18n("Controlling the cart number:")
                font.pointSize: 11
                Layout.alignment: Qt.AlignVCenter
                onToggled: classroomControlWidget.changeControlMode(cartModeCB.checked)
            }

            PC3.ComboBox {
                id: cartValues
                currentIndex: classroomControlWidget.currentCartIndex
                model: classroomControlWidget.maxNumCart
                delegate: ItemDelegate {
                    width: 150
                    text: index + 1
                }
                displayText: currentIndex + 1
                enabled: cartModeCB.checked
                Layout.preferredWidth: 60
                Layout.alignment: Qt.AlignVCenter
                onActivated: {
                    classroomControlWidget.changeCart(cartValues.currentValue + 1)
                }
            }
        }

        PlasmaExtras.PlaceholderMessage {
            id: phMsg
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin:15
            Layout.rightMargin:15
            Layout.alignment:Qt.AlignHCenter | Qt.AlignVCenter
            visible: classroomControlWidget.showWaitMsg
            iconName: "view-refresh"
            text: getTextMsg(classroomControlWidget.msgCode)
        }

        Item {
            Layout.fillHeight: true
            visible: !classroomControlWidget.showWaitMsg
        }

        RowLayout {
            id: buttomsRow
            Layout.fillWidth: true
            Layout.leftMargin:15
            Layout.rightMargin:15
            spacing: 10
            visible: !classroomControlWidget.showWaitMsg

            PC3.Button {
                id: unlockBtn
                text: i18n("Unlock cart")
                icon.name: "document-decrypt"
                KeyNavigation.right: applyBtn
                enabled: classroomControlWidget.isCartControlEnabled && !classroomControlWidget.showError
                onClicked: classroomControlWidget.unlockCart()
            }

            Item {
                Layout.fillWidth: true
            }

            PC3.Button {
                id: applyBtn
                text: i18n("Apply")
                icon.name: "dialog-ok"
                KeyNavigation.right: cancelBtn
                enabled: classroomControlWidget.arePendingChanges
                onClicked: classroomControlWidget.applyChanges()
            }

            PC3.Button {
                id: cancelBtn
                text: i18n("Cancel")
                icon.name: "dialog-cancel"
                enabled: classroomControlWidget.arePendingChanges || classroomControlWidget.showError
                onClicked: classroomControlWidget.cancelChanges()
            }
        }
    }

    function getTextMsg(code) {

        switch (code) {
            case -1: 
                return i18n("Unable to get ip from interface")
            case -2:
                return i18n("Mask value from interface is wrong")
            case -3: 
                return i18n("The selected cart is already beaing controlled by another computer")
            case -4: 
                return i18n("Insufficient number of hosts in subnet")
            case -5: 
                return i18n("Virtual interface not created")
            case -6: 
                return i18n("Unable to configure classroom control")
            case 2:
                return i18n("Applyng changes. Wait a moment...")
            case 3:
                return i18n("Restoring values. Wait a moment...")
            case 4:
                return i18n("Deactivating classroom control. Wait a moment...")
            case 5:
                return i18n("Reactivating classroom control. Wait a moment...")
            default:
                return ""
        }
    }
}
