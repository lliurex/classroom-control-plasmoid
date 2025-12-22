import QtQuick 2.6
import QtQuick.Controls 2.6
import QtQuick.Layouts 1.15
import QtQuick.Window 2.2
import org.kde.plasma.components 3.0 as PC

ApplicationWindow {
	visible: true
	title: "Classroom Control"
	property int margin: 1
	flags:Qt.FramelessWindowHint | Qt.Window
	color:"#eff0f1"
	width: 610
	height: mainLayout.implicitHeight + 2 * margin
	minimumWidth: 610
	minimumHeight: mainLayout.Layout.minimumHeight + 2 * margin
	maximumWidth: 610
	maximumHeight: mainLayout.Layout.maximumHeight + 2 * margin
	Component.onCompleted: {
	    x = Screen.width/2 - width/2 
        y = Screen.height/2 - height/2

    }

    onClosing: {
     	if (bridge.closed(true))
     		close.accepted=true;
        else
        	close.accepted=false;	
              
    }

    ColumnLayout {
    	id: mainLayout
    	anchors.fill: parent
    	anchors.margins: margin
    	Layout.minimumWidth:610	
    	Layout.maximumWidth:610
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
	   				source:"/usr/share/icons/breeze/status/64/dialog-warning.svg"
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
