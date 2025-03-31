import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.classroomcontrol 1.0
// Item - the most basic plasmoid component, an empty container.
Item {

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

    Plasmoid.switchWidth: units.gridUnit * 5
    Plasmoid.switchHeight: units.gridUnit * 5

    Plasmoid.icon:classroomControlWidget.iconName
    Plasmoid.toolTipMainText: classroomControlWidget.toolTip
    Plasmoid.toolTipSubText: classroomControlWidget.subToolTip

    Component.onCompleted: {
       plasmoid.removeAction("configure");
       plasmoid.setAction("openHelp",i18n("See help"),"help-contents")
    }

   
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.fullRepresentation: Item{
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
    
 }  
