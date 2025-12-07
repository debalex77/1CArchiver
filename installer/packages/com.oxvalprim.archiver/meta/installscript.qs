function Component() {}

function isVCRedistInstalled() {
    // Citim cheia direct din registry prin QtIFW
    var val = installer.value(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64\\Installed"
    );

    console.log("VC Installed registry value =", val);

    return val === "1";    // instalat
}

Component.prototype.createOperations = function() {
    component.createOperations();

    var exe = "@TargetDir@/1CArchiver.exe";
    var ico = "@TargetDir@/backup.ico";

    // SHORTCUTS
    component.addOperation("CreateShortcut", exe, "@DesktopDir@/1CArchiver.lnk",
                           "workingDirectory=@TargetDir@", "iconPath=" + ico);
    component.addOperation("CreateShortcut", exe, "@StartMenuDir@/1CArchiver.lnk",
                           "workingDirectory=@TargetDir@", "iconPath=" + ico);

    // -------------------------------
    //   VERIFICARE VC_REDIST
    // -------------------------------
    if (isVCRedistInstalled()) {
        console.log("✔ VC Redist este instalat → SKIP");
        return;
    }

    console.log("✘ VC Redist NU este instalat → instalăm...");

    var url = "https://aka.ms/vs/17/release/vc_redist.x64.exe";
    var outFile = "@TargetDir@/vc_redist.x64.exe";

    // Descărcare
    component.addOperation(
        "Execute",
        "powershell.exe",
        "-NoProfile", "-ExecutionPolicy", "Bypass",
        "-Command",
        "Invoke-WebRequest -Uri '" + url + "' -OutFile '" + outFile + "'",
        "ignoreErrors=true"
    );

    // Instalare silent
    component.addOperation(
        "Execute",
        outFile,
        "/install", "/quiet", "/norestart",
        "ignoreErrors=true"
    );
};
