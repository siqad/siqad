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
}