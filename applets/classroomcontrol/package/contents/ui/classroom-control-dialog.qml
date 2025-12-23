import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import org.kde.plasma.components as PC

ApplicationWindow {
	visible: true
	title: "Classroom Control"
	property int margin: 1
	flags:Qt.FramelessWindowHint | Qt.Window | Qt.WindowStaysOnTopHint
	color:"#eff0f1"
	width: 550
	height: mainLayout.implicitHeight + 2 * margin
	minimumWidth: 550
	minimumHeight: mainLayout.Layout.minimumHeight + 2 * margin
	maximumWidth: 550
	maximumHeight: mainLayout.Layout.maximumHeight + 2 * margin
	Component.onCompleted: {
	    x = Screen.width/2 - width/2 
        y = Screen.height/2 - height/2

    }

    onClosing:(close)=>{
     	if (bridge.closed(true))
     		close.accepted=true;
        else
        	close.accepted=false;	
              
    }

    ColumnLayout {
    	id: mainLayout
    	anchors.fill: parent
    	anchors.margins: margin
    	Layout.minimumWidth:550	
    	Layout.maximumWidth:550
    	Layout.minimumHeight:205
    	Layout.maximumHeight:205
    	
	   	GridLayout {
	   		id: grid
	   		Layout.topMargin: 10
	   		Layout.bottomMargin: 10
	   		rows: 3
	   		columns: 2
	   		Rectangle {
	   			color:"transparent"
	   			Layout.rowSpan: 1
	   			Layout.columnSpan: 1
	   			Layout.leftMargin:10
	   			width:60
	   			height:60
	   			Image{
	   				source:"/usr/share/icons/hicolor/scalable/apps/classroom_control_error.svg"
	   				sourceSize.width:64
	   				sourceSize.height:64
	   				anchors.centerIn:parent
	   			}
	   		}
	   		Rectangle {
	   			color:"transparent"
	   			Layout.rowSpan: 1
	   			Layout.columnSpan: 1
	   			height:60
	   			Layout.fillWidth: true
	   			Layout.leftMargin:10
	   			Text{
	   				id:warningText
	   				text:bridge.translateMsg[0]
	   				font.family: "Quattrocento Sans Bold"
	   				font.pointSize: 11
	   				anchors.left: parent.left
	   				anchors.verticalCenter:parent.verticalCenter
	   			}
	   			
	   		}
	   		Rectangle {
	   			color:"transparent"
	   			Layout.rowSpan: 1
	   			Layout.columnSpan: 2
	   			Layout.fillWidth: true
	   			height:70
	   			Text {
	   				id:countDown
	   				visible:true
	   				font.family: "Quattrocento Sans Bold"
	   				font.pointSize: 50
	   				anchors.centerIn:parent
	   				text:bridge.timeRemaining[0]
	   				color:bridge.timeRemaining[1]
	   			}
	   		}
	   		Rectangle {
	   			id:btnBox
	   			color:"transparent"
	   			visible:true
	   			Layout.rowSpan: 1
	   			Layout.columnSpan: 2
	   			Layout.fillWidth: true
	   			Layout.rightMargin:10
	   			height:60

	   			PC.Button {
	   				id:cancelBtn
	   				anchors.right: parent.right
	   				anchors.verticalCenter:parent.verticalCenter
	   				Layout.preferredHeight:40
	   				display:AbstractButton.TextBesideIcon
	   				icon.name:"dialog-cancel"
	   				text:bridge.translateMsg[1]
	   				Keys.onReturnPressed:cancelBtn.clicked()
	   				Keys.onEnterPressed:cancelBtn.clicked()
	   				onClicked:{
	   					bridge.cancelClicked()

					}
				}
		    }
		}
	
	 }
}  		
