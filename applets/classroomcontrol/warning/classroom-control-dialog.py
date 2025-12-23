#! /usr/bin/python3
from PySide2.QtWidgets import QApplication
from PySide2.QtCore import QUrl, QObject, Slot, Signal, Property,QTimer
from PySide2.QtQml import QQmlApplicationEngine
from PySide2.QtGui import QIcon

import os
import sys
import subprocess
import gettext
import dbus.mainloop.glib
from gi.repository import GLib
gettext.textdomain("plasma_applet_org.kde.plasma.classroomcontrol")
_=gettext.gettext


class Bridge(QObject):

	def __init__(self,iface):

		QObject.__init__(self)

		self.iface=iface
		self.indicatorColor="#3daee9"
		self.countdown=60
		self.currentCounter=0
		self.countdownTimer = QTimer(None)
		self.countdownTimer.timeout.connect(self.updateCountDown)
		self._timeRemaining=["01:00",self.indicatorColor]

		self.initValues()

	#def __init__
	
	def initValues(self):
		
		warningMsg=_("Classroom Control will be deactivate in few seconds")
		cancelBtnMsg=_("Cancel deactivation")

		self._translateMsg=[warningMsg,cancelBtnMsg]
		self.countdownTimer.start(1000)
	
	#def init_values

	def updateCountDown(self):

		self.currentCounter+=1

		if self.countdown-self.currentCounter >=0:
			count=self.countdown-self.currentCounter
			
			if count==60:
				self.timeRemaining=["01:00",self.indicatorColor]
			elif count<10:
				self.indicatorColor="#ff0000"
				self.timeRemaining=["00:0"+str(count),self.indicatorColor]
			else:
				if count==10:
					self.indicatorColor="#ff0000"
				self.timeRemaining=["00:"+str(count),self.indicatorColor]
		else:
			self.countdownTimer.stop()
			self.iface.launchDeactivation()
			app.quit()

	#def updateCountDown

	def _getTranslateMsg(self):

		return self._translateMsg

	#def _getTranslateMsg	
	
	def _getTimeRemaining(self):

		return self._timeRemaining

	#def _getTimeRemaining	

	def _setTimeRemaining(self,timeRemaining):

		self._timeRemaining=timeRemaining
		self.on_timeRemaining.emit()	

	#def _setTimeRemaining

	def closeWarningSignal(self):
		
		self.countdownTimer.stop()
		app.quit()

    #def closeWarningSignal

	@Slot()
	def cancelClicked(self):

		self.countdownTimer.stop()
		self.iface.cancelDeactivation()
		app.quit()

	#def cancelClicked

	@Slot(bool,result=bool)
	def closed(self,state):
		
		return True

	#def closed	
		
	on_timeRemaining=Signal()
	timeRemaining=Property('QVariantList',_getTimeRemaining,_setTimeRemaining, notify=on_timeRemaining)
	translateMsg=Property('QVariantList',_getTranslateMsg,constant=True)

#class Bridge

if __name__=="__main__":

	app = QApplication()
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = dbus.SessionBus()

	REMOTE_BUS="com.classroomcontrol.DeactivationWarning"
	REMOTE_PATH="/DeactivationWarning"
	REMOTE_IFACE="org.classroomcontrol.Bridge"

	proxy=bus.get_object(REMOTE_BUS,REMOTE_PATH)
	iface=dbus.Interface(proxy,dbus_interface=REMOTE_IFACE)

	engine = QQmlApplicationEngine()
	engine.clearComponentCache()
	context=engine.rootContext()
	bridge=Bridge(iface)
	context.setContextProperty("bridge", bridge)

	bus.add_signal_receiver(
    	handler_function=bridge.closeWarningSignal,
    	signal_name="closeWarning",
    	dbus_interface="org.classroomcontrol.Bridge",
    	bus_name="com.classroomcontrol.DeactivationWarning"
    )

	url = QUrl("/usr/share/plasma/plasmoids/org.kde.plasma.classroomcontrol/contents/ui/classroom-control-dialog.qml")

	engine.load(url)
	if not engine.rootObjects():
		sys.exit(-1)

	engine.quit.connect(QApplication.quit)
	app.setWindowIcon(QIcon("/usr/share/icons/hicolor/scalable/apps/classroom_control.svg"));
	ret=app.exec_()
	del engine
	del app
	sys.exit(ret)
