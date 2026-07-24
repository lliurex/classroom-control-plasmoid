import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.16 as Kirigami

Rectangle {
    id: optionsContainer
    color: "transparent"
    enabled: !classroomControlWidget.showWaitMsg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
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
            visible: classroomControlWidget.showError
            text: getTextMsg(classroomControlWidget.errorCode)
            type: Kirigami.MessageType.Error
        }

        GridLayout {
            id: controlOptions
            Layout.fillWidth: true
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
        var msg = ""
        switch (code) {
            case -1: msg = i18n("Unable to get ip from interface"); break;
            case -2: msg = i18n("Mask value from interface is wrong"); break;
            case -3: msg = i18n("The selected cart is already beaing controlled by another computer"); break;
            case -4: msg = i18n("Insufficient number of hosts in subnet"); break;
            case -5: msg = i18n("Virtual interface not created"); break;
            case -6: msg = i18n("Unable to configure classroom control"); break;
            case 2:  msg = i18n("Applyng changes. Wait a moment..."); break;
            case 3:  msg = i18n("Restoring values. Wait a moment..."); break;
            case 4:  msg = i18n("Deactivating classroom control. Wait a moment..."); break;
            case 5:  msg = i18n("Reactivating classroom control. Wait a moment..."); break;
        }
        return msg;
    }
}
