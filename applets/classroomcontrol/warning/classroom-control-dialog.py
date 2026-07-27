#! /usr/bin/python3
from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QUrl, QObject, Slot, Signal, Property,QTimer
from PySide6.QtQml import QQmlApplicationEngine
from PySide6.QtGui import QIcon

import os
import sys
import subprocess
import gettext
import dbus.mainloop.glib
from gi.repository import GLib
gettext.textdomain("plasma_applet_org.kde.plasma.classroomcontrol")
_=gettext.gettext


class Bridge(QObject):

	timeRemainingChanged=Signal()
	
	def __init__(self,iface):

		QObject.__init__(self)

		self.iface=iface
		self.indicatorColor="#3daee9"
		self.countdown=60
		self.currentCounter=0
		self.countdownTimer = QTimer(None)
		self.countdownTimer.timeout.connect(self.updateCountDown)
		self._timeRemaining={"time":"01:00","color":self.indicatorColor}


		self.initValues()

	#def __init__

	@Property(dict,notify=timeRemainingChanged)
	def timeRemaining(self):

		return self._timeRemaining

	#def timeRemaining	

	@timeRemaining.setter
	def timeRemaining(self,timeRemaining):

		self._timeRemaining=timeRemaining
		self.timeRemainingChanged.emit()	

	#def timeRemaining

	@Property(dict,constant=True)
	def translateMsg(self):

		return self._translateMsg

	#def translateMsg

	@Property(bool,constant=True)	
	def visibleCancelBtn(self):

		return self._visibleCancelBtn

	#def visibleCancelBtn	

	def initValues(self):
		
		warningMsg=_("Classroom Control will be deactivate in few seconds")
		cancelBtnMsg=_("Cancel deactivation")

		self._translateMsg={"msg":warningMsg,"btnMsg":cancelBtnMsg}
		self.countdownTimer.start(1000)
	
	#def init_values

	def updateCountDown(self):

		self.currentCounter+=1
		count=self.countdown-self.currentCounter

		if count>=0:
			mins,secs=divmod(count,60)

			if count<=10:
				self.indicatorColor="#ff0000"

			self.timeRemaining={"time":f"{mins:02d}:{secs:02d}","color":self.indicatorColor}
		else:
			self.countdownTimer.stop()
			self.iface.launchDeactivation()
			QApplication.quit()

	#def updateCountDown

	def closeWarningSignal(self):
		
		self.countdownTimer.stop()
		QApplication.quit()

    #def closeWarningSignal

	@Slot()
	def cancelClicked(self):

		self.countdownTimer.stop()
		self.iface.cancelDeactivation()
		QApplication.quit()

	#def cancelClicked

	@Slot(bool,result=bool)
	def closed(self,state):
		
		return True

	#def closed	
		
#class Bridge

if __name__=="__main__":

	app = QApplication()
	app.setDesktopFileName("classroom-control")
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
    	signal_name="closeWarningSignal",
    	dbus_interface="org.classroomcontrol.Bridge",
    	bus_name="com.classroomcontrol.DeactivationWarning"
    )

	url = QUrl("/usr/share/plasma/plasmoids/org.kde.plasma.classroomcontrol/contents/ui/classroom-control-dialog.qml")

	engine.load(url)
	if not engine.rootObjects():
		sys.exit(-1)

	engine.quit.connect(app.quit)
	ret=app.exec()
	del engine
	del app
	sys.exit(ret)
