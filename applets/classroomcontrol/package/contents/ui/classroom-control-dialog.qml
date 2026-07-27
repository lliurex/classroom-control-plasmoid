import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.kde.plasma.components 3.0 as PC


ApplicationWindow {
    id:window
	visible: true
	title: "Classroom Control"
	property int margin: 1
	flags:Qt.FramelessWindowHint | Qt.Window
	color:"#eff0f1"
	width: 550
	height: 205 + 2 * margin
	minimumWidth: 550
	maximumWidth: 550
	minimumHeight: height
    maximumHeight: height
	Component.onCompleted: {
        window.x = (screen.width - window.width) / 2
        window.y = (screen.height - window.height) / 2
    }

    onClosing: (close) => {
        if (bridge.closed(true)) {
            close.accepted = true;
        } else {
            close.accepted = false;
        }
    }

   GridLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 10
        rows: 3
        columns: 2
        rowSpacing: 10
        columnSpacing: 10

        Item {
            Layout.preferredWidth: 60
            Layout.preferredHeight: 60
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            Image {
                source: "file:///usr/share/icons/hicolor/scalable/apps/classroom_control_error.svg"
                sourceSize.width:64
                sourceSize.height:64
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 60

            Text {
                id: warningText
                text: bridge.translateMsg.msg
                font.pointSize: 11
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Item {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 70

            Text {
                id: countDown
                font.pointSize: 50
                anchors.centerIn: parent
                text: bridge.timeRemaining.time
                color: bridge.timeRemaining.color
            }
        }

        Item {
            id: btnBox
            visible: true
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 40

            PC.Button {
                id: cancelBtn
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                display: AbstractButton.TextBesideIcon
                icon.name:"dialog-cancel"
                text: bridge.translateMsg.btnMsg
                onClicked:bridge.cancelClicked()
           
            }
        }
    }
}
