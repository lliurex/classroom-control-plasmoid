
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.classroomcontrol)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.classroomcontrol")
ecm_add_qml_module(classroomcontrolplugin URI org.kde.plasma.private.classroomcontrol)

target_sources(classroomcontrolplugin PRIVATE
    plugin/plugin.cpp
    plugin/ClassroomControlWidget.cpp
    plugin/ClassroomControlWidgetUtils.cpp
   
)

target_link_libraries(classroomcontrolplugin PRIVATE
                      Qt6::Quick
                      Qt6::Core
                      Qt6::Widgets
                      KF6::CoreAddons
                      KF6::I18n
                      KF6::Notifications
                      KF6::KIOWidgets
                      Edupals::Base Edupals::N4D

                      )
ecm_finalize_qml_module(classroomcontrolplugin)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/classroomcontrol)
install(TARGETS classroomcontrolplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/classroomcontrol)
install(FILES "icons/classroom_control.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_1.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_2.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_3.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_4.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_5.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_6.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_7.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_8.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_9.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_10.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_11.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_12.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_13.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_14.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_15.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_error.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_off.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)  
install(FILES "knotifications6/classroomcontrol.notifyrc"
       DESTINATION ${DATAROOTDIR}/usr/share/knotifications6/)
