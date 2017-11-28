function Component()
{
    if(installer.value("os") === "win") {
        component.addOperation("CreateShortcut", "@TargetDir@/db-sim.exe", "@UserStartMenuProgramsPath@/@StartMenuDir@/DBDesigner.lnk");
    }
}
