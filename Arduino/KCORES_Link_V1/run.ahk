#NoEnv
#NoTrayIcon
SetWorkingDir %A_ScriptDir%
SetRegView 32
SetBatchLines -1

RegRead, ArduinoUninstall, HKEY_LOCAL_MACHINE, SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Arduino, UninstallString
ArduinoEXE := StrReplace(ArduinoUninstall, "uninstall.exe" , "arduino.exe")
SplitPath, A_ScriptDir, OutDirName

if !FileExist(OutDirName . ".ino")
{
  count := 0
  Loop, *.ino, 1, 1 
    count += 1
  if (count = 1)
    FileMove, *.ino, %OutDirName%.ino
  else
    Exitapp
}

Run, %ArduinoEXE% --preferences-file preferences.txt %OutDirName%.ino