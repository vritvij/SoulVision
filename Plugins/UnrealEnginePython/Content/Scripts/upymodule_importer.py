#Imports upymodule.json modules and resolves dependencies

#example test
#import upymodule_importer as upym
#upym.parseJson("D:/Users/Admin/Documents/GitHub/tensorflow-ue4-mnist-example/Plugins/tensorflow-ue4/Content/Scripts/upymodule.json")

import upypip as pip
import sys
import unreal_engine as ue
import json

import upypip
pip = upypip.pip

#parse upymodule.json from path and install dependencies if missing
def parseJson(packagePath):

	try:
		with open(packagePath) as data_file:
			#TODO: catch file not found error    
			package = json.load(data_file)
			ue.log('Resolving upymodule dependencies for ' + package['name'])
			pythonModules = package['pythonModules']

			#loop over all the modules, check if we have them installed
			for module in pythonModules:
				version = pythonModules[module]

				ue.log(module + " " + version + " installed? " + str(pip.isInstalled(module)))
				if not pip.isInstalled(module, version):
					ue.log('Dependency not installed, fetching via pip...')
					pip.install(module + '==' + version)

			dependencyNoun = 'dependencies'
			if len(pythonModules) == 1:
				dependencyNoun = 'dependency'

			ue.log(str(len(pythonModules)) + ' ' + package['name'] + ' upymodule ' + dependencyNoun + ' resolved (if installation in progress, more async output will stream)')
	except:
		e = sys.exc_info()[0]
		ue.log('upymodule.json error: ' + str(e))

