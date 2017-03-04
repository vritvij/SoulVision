import upycmd as cmd
import sys
from threading import Thread
from distutils.version import LooseVersion


class PipInstall:
	#global modules class static variable
	modules = None

	def __init__(self):
		return None

	#blocking actions, should be run on off-thread
	def pipModuleAction(self, command, args, verbose=True):
		return cmd.run('pip ' + command + ' ' + args, cmd.PythonHomeScriptsPath(), verbose)

	#use this if you need to work on the resulting list to query current dependencies
	def listDict(self, verbose=True):
		#get the list of all the modules from pip
		resultString = self.pipModuleAction('list','--format=columns', verbose)

		#convert to lines for parsing
		lines = resultString.split("\n")
		resultDict = {}

		#exclude first two lines, then pair up the results
		for i in range(2,len(lines)):
			splitEntry = lines[i].split() #split and ignore all whitespace
			resultDict[splitEntry[0]] = splitEntry[1]

		#save list to cache
		PipInstall.modules = resultDict
		return resultDict

	def isDesiredVersionSufficient(self, desired, current):
		return LooseVersion(desired) <= LooseVersion(current)

	def isInstalled(self, module, desiredVersion=None):
		if PipInstall.modules == None:
			PipInstall.modules = self.listDict(False)
		if module in PipInstall.modules:
			#did we specify a version? ensure we've installed that version at least
			if desiredVersion:
				#print('current version: ' + str(PipInstall.modules[module]))
				#print('desired version: ' + str(desiredVersion))
				return self.isDesiredVersionSufficient(desiredVersion, PipInstall.modules[module])
			else:
				return True
		else:
			return False

	#Threaded actions
	def install(self, module):
		PipInstall.modules = None #our cache is no longer valid
		action = self.pipModuleAction
		t = Thread(target=action, args=('install',module,))
		t.start()

	def uninstall(self, module):
		PipInstall.modules = None #our cache is no longer valid
		action = self.pipModuleAction
		t = Thread(target=action, args=('uninstall -y',module,))
		t.start()

	def list(self):
		action = self.listDict
		t = Thread(target=action)
		t.start()

pip = PipInstall()