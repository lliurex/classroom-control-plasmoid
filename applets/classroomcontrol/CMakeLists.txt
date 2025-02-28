
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.classroomcontrol)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.classroomcontrol")

set(classroomcontrolwidget_SRCS
    plugin/plugin.cpp
    plugin/ClassroomControlWidget.cpp
    plugin/ClassroomControlWidgetUtils.cpp
   
)

add_library(classroomcontrolplugin SHARED ${classroomcontrolwidget_SRCS})

target_link_libraries(classroomcontrolplugin
                      Qt5::Quick
                      Qt5::Core
                      Qt5::Widgets
                      KF5::CoreAddons
                      KF5::I18n
                      KF5::Notifications
                      KF5::KIOWidgets
                      Edupals::Base Edupals::N4D

                      )

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/classroomcontrol)
install(TARGETS classroomcontrolplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/classroomcontrol)
install(FILES "icons/classroom_control_off.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_92.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_93.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_94.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_95.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_96.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_97.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_98.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_99.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
#install(FILES "knotifications5/classroomcontrol.notifyrc"
 #       DESTINATION ${DATAROOTDIR}/usr/share/knotifications5/)
#install(FILES "autostart/lliurex-up-indicator"
#        DESTINATION ${DATAROOTDIR}/etc/bin)        
