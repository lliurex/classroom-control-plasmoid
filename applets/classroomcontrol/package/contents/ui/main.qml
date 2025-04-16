import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.classroomcontrol 1.0
// Item - the most basic plasmoid component, an empty container.

PlasmoidItem {

    id:classroomControlApplet
    
    ClassroomControlWidget{
        id:classroomControlWidget

    }


    Plasmoid.status: {
        /* Warn! Enum types are accesed through ClassName not ObjectName */
        switch (classroomControlWidget.status){
            case ClassroomControlWidget.ActiveStatus:
                return PlasmaCore.Types.ActiveStatus
            case ClassroomControlWidget.PassiveStatus:
                return PlasmaCore.Types.PassiveStatus
           
        }
        return  PlasmaCore.Types.ActiveStatus
        
    }

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.icon:classroomControlWidget.iconName
    toolTipMainText: classroomControlWidget.toolTip
    toolTipSubText: classroomControlWidget.subToolTip

    Component.onCompleted: {
       Plasmoid.setInternalAction("configure", configureAction)

    }

   
    fullRepresentation: Item{
        id:root
        Layout.fillWidth:true
        QQC2.StackView{
            id:stackLayout
            property int currentIndex:classroomControlWidget.currentStackIndex
            width:parent.width
            height:parent.height
            initialItem:infoPanel
            onCurrentIndexChanged:{
                switch (currentIndex){
                    case 0:
                        stackLayout.replace(infoPanel)
                        break;
                    case 1:
                        stackLayout.replace(controlPanel)
                        break;
                }
            }


            Component{
                id:infoPanel
                InfoView{
                    id:infoView
                }
            }
            Component{
                id:controlPanel
                ControlView{
                    id:controlView
                }
            }

        }
    }

    function action_openHelp(){

        classroomControlWidget.openHelp()

    }

    PlasmaCore.Action {
        id: configureAction
        text: i18n("See help")
        icon.name:"help-contents"
        onTriggered: action_openHelp
    }
    
 }  
