function Component()
{
    // default constructor
}


Component.prototype.createOperations = function()
{
    component.createOperations();
	if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/db-sim.exe", "@StartMenuDir@/db-sim.lnk",
            "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll",
            "iconId=1", "description=Open SiQAD");
    }
	
	
	if (installer.value("os") == "win") { 
	try {
		var userProfile = installer.environmentVariable("USERPROFILE");
		installer.setValue("UserProfile", userProfile);
		component.addOperation("CreateShortcut", "@TargetDir@/db-sim.exe", "@UserProfile@/Desktop/db-sim.lnk");
	} catch (e) {
		// Do nothing if key doesn't exist
	}
	}
}