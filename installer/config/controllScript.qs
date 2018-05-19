function Controller()
{
}

Controller.prototype.IntroductionPageCallback = function()
{
    var widget = gui.currentPageWidget(); // get the current wizard page
    if (widget != null) {
        widget.title = "SiQAD Installer"; // set the page title
        widget.MessageLabel.setText("Welcome to the installer. Please follow the prompts to install the simulator and the dependencies."); // set the welcome text
    }
}

Controller.prototype.ComponentSelectionPageCallback = function()
{
	//QMessageBox.information("identifier", "Title", "text", QMessageBox.Ok);
	var widget = gui.pageById(QInstaller.ComponentSelection); // get the introduction wizard page
	var pyComponentId = "siqad.python";
	
	//Checking for dependencies in order to give the correct prompts on component selection page.

	//The file names of the various Anaconda releases.
	var Anaconda3_64 = "/Anaconda3 (64-bit)";
	var Anaconda3_32 = "/Anaconda3 (32-bit)";
	var Anaconda2_64 = "/Anaconda2 (64-bit)";
	var Anaconda2_32 = "/Anaconda2 (32-bit)";
	var Anaconda3 = "/Anaconda3";
	var Anaconda2 = "/Anaconda2";
	
	//Which sub-versions of Python to check for (2.8 will be the last release of Python 2).
	var pyVerCount_3 = 9;
	var pyVerCount_2 = 8;
	
	//The counters to be used for counting through the various Python versions.
	var pyCount_3 = 0;
	var pyCount_2 = 0;
	
	//The string stems of the versions of Python to check for
	var pyStem_3 = "/Python 3.";
	var pyStem_2 = "/Python 2.";
	
	
	//A flag is used to skip checking other Python installation possibilities if one is already found.
	var flag = false;

	//Checking for a Python 3 installation.
	for(pyCount_3 = pyVerCount_3; pyCount_3 >= 0; pyCount_3--){
		if( installer.fileExists(installer.value("UserStartMenuProgramsPath") + pyStem_3 + pyCount_3.toString())
			|| installer.fileExists(installer.value("AllUsersStartMenuProgramsPath") + pyStem_3 + pyCount_3.toString())
			|| installer.fileExists(installer.value("HomeDir") + "/AppData/Local/Programs/Python/Python3" + pyCount_3.toString() + "-32")
			|| installer.fileExists(installer.value("HomeDir") + "/AppData/Local/Programs/Python/Python3" + pyCount_3.toString() + "-64")
			|| installer.fileExists(installer.value("HomeDir") + pyCount_3.toString() + "-32") || installer.fileExists(installer.value("HomeDir") + pyCount_3.toString() + "-64")
			|| installer.fileExists(installer.value("C:/") + pyCount_3.toString() + "-32") || installer.fileExists(installer.value("C:/") + pyCount_3.toString() + "-64")
			|| installer.fileExists(installer.value("C:/Program Files (x86)") + pyCount_3.toString() + "-32") || installer.fileExists(installer.value("C:/Program Files (x86)") + pyCount_3.toString() + "-64")
			|| installer.fileExists(installer.value("C:/Program Files") + pyCount_3.toString() + "-32") || installer.fileExists(installer.value("C:/Program Files") + pyCount_3.toString() + "-64")
			){
			QMessageBox.information("python3", "SiQAD Installer", "A valid Python 3 runtime was detected on this computer. If you know that this is incorrect, please opt in to Python installation for full SiQAD simulator functionality.", QMessageBox.Ok);
			flag = true;
		}
	}
	
	//Checking for an Anaconda 3 installation.
	if ( (installer.fileExists(installer.value("UserStartMenuProgramsPath") + Anaconda3_64) || installer.fileExists(installer.value("UserStartMenuProgramsPath") + Anaconda3_32)
	|| installer.fileExists(installer.value("AllUsersStartMenuProgramsPath") + Anaconda3_64) || installer.fileExists(installer.value("AllUsersStartMenuProgramsPath") + Anaconda3_32)
	|| installer.fileExists(installer.value("HomeDir") + Anaconda3) || installer.fileExists("C:/ProgramData" + Anaconda3) || installer.fileExists("C:/Program Files" + Anaconda3)
	|| installer.fileExists("C:/Program Files (x86)" + Anaconda3) || installer.fileExists("C:/" + Anaconda3) ) && !flag ){
		QMessageBox.information("anaconda3", "SiQAD Installer", "An Anaconda 3 runtime was detected on this computer. If you know that this is incorrect, please opt in to Python installation. Python 3 is required for full SiQAD simulator functionality.", QMessageBox.Ok);
		flag = true;
	}
	
	//Checking for an Anaconda 2 installation.
	else if ( ( installer.fileExists(installer.value("UserStartMenuProgramsPath") + Anaconda2_64) || installer.fileExists(installer.value("UserStartMenuProgramsPath") + Anaconda2_32)
	|| installer.fileExists(installer.value("AllUsersStartMenuProgramsPath") + Anaconda2_64) || installer.fileExists(installer.value("AllUsersStartMenuProgramsPath") + Anaconda2_32)
	|| installer.fileExists(installer.value("HomeDir") + Anaconda2) || installer.fileExists("C:/ProgramData" + Anaconda2) || installer.fileExists("C:/Program Files" + Anaconda2)
	|| installer.fileExists("C:/Program Files (x86)" + Anaconda2) || installer.fileExists("C:/" + Anaconda2) ) && !flag ){
		QMessageBox.information("anaconda2", "SiQAD Installer", "An Anaconda 2 runtime was detected on this computer; please change to the Python 3 package for full SiQAD Simulator functionality. If you know that this is incorrect, please opt in to Python installation. Python 3 is required for full SiQAD simulator functionality.", QMessageBox.Ok);
		flag = true;
	}
	
	//Checking for a Python 2 installation.
	if (!flag){
		for(pyCount_2 = pyVerCount_2; pyCount_2 >= 0; pyCount_2--){
			if( installer.fileExists(installer.value("UserStartMenuProgramsPath") + pyStem_2 + pyCount_2.toString())
				|| installer.fileExists(installer.value("AllUsersStartMenuProgramsPath") + pyStem_2 + pyCount_2.toString())
				|| installer.fileExists(installer.value("HomeDir") + "/AppData/Local/Programs/Python/Python2" + pyCount_2.toString() + "-32")
				|| installer.fileExists(installer.value("HomeDir") + "/AppData/Local/Programs/Python/Python2" + pyCount_2.toString() + "-64")
				|| installer.fileExists(installer.value("HomeDir") + pyCount_2.toString() + "-32") || installer.fileExists(installer.value("HomeDir") + pyCount_2.toString() + "-64")
				|| installer.fileExists(installer.value("C:/") + pyCount_2.toString() + "-32") || installer.fileExists(installer.value("C:/") + pyCount_2.toString() + "-64")
				|| installer.fileExists(installer.value("C:/Program Files (x86)") + pyCount_2.toString() + "-32") || installer.fileExists(installer.value("C:/Program Files (x86)") + pyCount_2.toString() + "-64")
				|| installer.fileExists(installer.value("C:/Program Files") + pyCount_2.toString() + "-32") || installer.fileExists(installer.value("C:/Program Files") + pyCount_2.toString() + "-64")
				){
				QMessageBox.information("python2", "SiQAD Installer", "A Python 2 runtime was detected on this computer; Python 3 could not be detected. If you know that this is incorrect, please opt out of Python installation. Python 3 is required for full SiQAD simulator functionality.", QMessageBox.Ok);
				if (widget != null){
					widget.selectComponent(pyComponentId);
				}
				flag = true;
			}
		}
	}
	
	if (!flag){
		QMessageBox.information("none", "SiQAD Installer", "No valid Python runtime was detected on this computer. If you know that this is incorrect, please opt out of Python installation. Python 3 is required for full SiQAD simulator functionality.", QMessageBox.Ok);
		if (widget != null){
			widget.selectComponent(pyComponentId);
		}
	}
}

Controller.prototype.FinishedPageCallback = function()
{
	//Triggers Python installation if the Python component was installed.
	var component = installer.componentByName('siqad.python');
    if ( component.isInstalled() ) {
        installer.performOperation("Execute", "@TargetDir@/python-3.6.5.exe");
    }

}