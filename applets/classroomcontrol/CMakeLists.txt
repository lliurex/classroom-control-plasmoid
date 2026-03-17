
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.classroomcontrol)


#######################################################################################
# Notes Library
set(PLUGIN_TARGET "classroomcontrolplugin")
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.classroomcontrol")
ecm_add_qml_module(${PLUGIN_TARGET} URI org.kde.plasma.private.classroomcontrol VERSION 2.0)


target_sources(${PLUGIN_TARGET} PRIVATE
    plugin/plugin.cpp
    plugin/ClassroomControlWidget.cpp
    plugin/ClassroomControlWidgetUtils.cpp
    plugin/ClassroomControlWidgetAdaptor.cpp
    plugin/plugin.h
    plugin/ClassroomControlWidget.h
    plugin/ClassroomControlWidgetUtils.h
    plugin/ClassroomControlWidgetAdaptor.h
   
)

target_link_libraries(${PLUGIN_TARGET} PRIVATE
                      Qt6::Quick
                      Qt6::Core
                      Qt6::Widgets
                      Qt6::Concurrent
                      KF6::CoreAddons
                      KF6::I18n
                      KF6::Notifications
                      KF6::KIOWidgets
                      Edupals::Base Edupals::N4D

                      )

target_include_directories(${PLUGIN_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugin)                      
ecm_finalize_qml_module(${PLUGIN_TARGET})

install(TARGETS ${PLUGIN_TARGET} DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/classroomcontrol)
install(FILES "icons/classroom_control.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_1.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_2.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_3.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_4.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_5.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_6.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_7.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_8.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_9.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_10.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_11.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_12.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_13.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_14.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_cart_15.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_error.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/classroom_control_off.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "knotifications6/classroomcontrol.notifyrc"
       DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
install(FILES "warning/classroom-control-dialog.py"
       DESTINATION ${DATAROOTDIR}/usr/bin/)

