import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as Components
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.16 as Kirigami

import org.kde.plasma.private.classroomcontrol 1.0

Rectangle{
	id: optionsContainer
	color:"transparent"
	enabled:!classroomControlWidget.showWaitMsg

	GridLayout{
		id: controlOptions
		rows: 3
		flow: GridLayout.TopToBottom
		rowSpacing:15
		width:parent.width

		RowLayout{
			id:head
			Layout.fillWidth:true
			PC3.ToolButton {
				height:35
				width:35
				Layout.rightMargin:controlOptions.width/3 - 50
				icon.name:"arrow-left.svg"
				PC3.ToolTip{
					id:backTP
					text:i18n("Back to main view")
				}
				onClicked:{
					backTP.hide()
					classroomControlWidget.manageNavigation(0)
				}
			} 

			Components.Label{
				id:headText
				text:i18n("Classroom control settings")
				font.italic:true
				font.pointSize:11
				Layout.fillWidth:true
				Layout.alignment:Qt.AlignHCenter
			}
		}

		Kirigami.InlineMessage{
			id:warningMsg
	    	visible:classroomControlWidget.showError?true:false
	    	text:getTextMsg(classroomControlWidget.errorCode)
	    	type:Kirigami.MessageType.Error
	    	implicitWidth:parent.width-10
	    	Layout.leftMargin:5
	    	Layout.rightMargin:5
	   	}

	   	RowLayout{
	   		id:cartModeRow
	   		Layout.fillWidth:true
	   		Layout.leftMargin:15
	   		Layout.rightMargin:15
	   		visible:!classroomControlWidget.showWaitMsg

	   		PC3.CheckBox {
	   			id:cartModeCB
	   			checked:classroomControlWidget.isCartControlEnabled?true:false
	   			text:i18n("Controlling the cart number:")
	   			enabled:true
	   			font.pointSize: 11
	   			Layout.rightMargin:10
	   			onToggled:classroomControlWidget.changeControlMode(cartModeCB.checked);
	   		}

	   		PC3.ComboBox{
	   			id:cartValues
	   			currentIndex:classroomControlWidget.currentCartIndex
	   			model:classroomControlWidget.maxNumCart
	   			delegate:ItemDelegate{
	   				width:300
	   				text:index+1
	   			}
	   			displayText:currentIndex+1
	   			enabled:cartModeCB.checked
	   			Layout.preferredWidth:50
	   			onActivated:{
	   				classroomControlWidget.changeCart(cartValues.currentValue+1)
	   			}
	   		}
	   	}
	}

	PlasmaExtras.PlaceholderMessage {
		id:phMsg
		anchors.centerIn: parent
		visible:classroomControlWidget.showWaitMsg
		width: parent.width-(PlasmaCore.Units.gridUnit *2 )
		iconName: "view-refresh.svg"
		text:getTextMsg(classroomControlWidget.msgCode)
	}

	RowLayout {
		id:buttomsRow
		anchors.bottom:optionsContainer.bottom
		anchors.bottomMargin:15
		anchors.fill:optionsContainer.fill
		spacing:10
		visible:!classroomControlWidget.showWaitMsg

		PC3.Button{
			id:unlockBtn
			text:i18n("Unlock cart")
			icon.name:"document-decrypt.svg"
			KeyNavigation.right:applyBtn
			enabled:{
				if (classroomControlWidget.isCartControlEnabled){
					if (classroomControlWidget.showError){
						false
					}else{
						true
					}
				}else{
					false
				}
			}
			onClicked: classroomControlWidget.unlockCart()
			Layout.leftMargin:10
			Layout.rightMargin:optionsContainer.width-(unlockBtn.width+applyBtn.width+cancelBtn.width+40)
		}

		PC3.Button {
			id:applyBtn
			text: i18n("Apply")
			icon.name: "dialog-ok"
			KeyNavigation.right: cancelBtn
			enabled:classroomControlWidget.arePendingChanges
			onClicked: classroomControlWidget.applyChanges()
		}

		PC3.Button {
			id: cancelBtn
			text: i18n("Cancel")
			icon.name: "dialog-cancel"
			enabled:{
				if (classroomControlWidget.arePendingChanges){
					true
				}else{
					if (classroomControlWidget.showError){
						true
					}else{
						false
					}
				}
			}
			onClicked: classroomControlWidget.cancelChanges()
		}
	}

	function getTextMsg(code){

		var msg=""

		switch (code){
			case -1:
				msg=i18n("Unable to get ip from interface")
				break;
			case -2:
				msg=i18nd("Mask value from interface is wrong")
				break;
			case -3:
				msg=i18n("The selected cart is already beaing controlled by another computer")
				break;
			case -4:
				msg=i18n("Insufficient number of hosts in subnet")
				break;
			case -5:
				msg=i18n("Virtual interface not created")
				break;
			case -6:
				msg=i18n("Unable to configure classroom control")
				break;
			case 2:
				msg=i18n("Applyng changes. Wait a moment...")
				break;
			case 3:
				msg=i18n("Restoring values. Wait a moment...")
				break;
		}
		return msg;
	}
}
