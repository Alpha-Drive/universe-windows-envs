# vnc-windows

## Setup
Open PowerShell (click the Start Menu in the bottom left and type "PowerShell", then right click and select "Run as Administrator")

HINT: To Paste commands into Powershell, make sure you don't have anything selected. Do this by pressing enter. Then, to paste, just right-click in Powershell (don't be a left clicker!).

Windows update - AMI only - Run Windows Update and restart if necessary. It will stay at downloading 0kbps for a few minutes while it allocates disk space. The update takes several minutes, but you can continue to install things while it takes place.

Disable Windows update to prevent the install notification from interfering with VNC sessions by running this [at startup](http://www.howtogeek.com/208224/how-to-add-programs-files-and-folders-to-system-startup-in-windows-8.1/)

Open the “Run” dialog box by pressing the Windows key + R. Type “shell:startup” (without the quotes) in the “Open” edit box and click “OK.”

Create a file called `disable-windows-update.bat`, right click -> Edit, and paste the following.
```
 net stop wuauserv
```

Allow Powershell to install things
```
Set-ExecutionPolicy RemoteSigned
Y
```

Install [Chocolatey](https://chocolatey.org/install) to use as a Windows package manager.
```
iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))
```

Restart PowerShell

Turn off UAC
```
C:\Windows\System32\cmd.exe /k %windir%\System32\reg.exe ADD HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System /v EnableLUA /t REG_DWORD /d 0 /f
```

Turn off IE ESC
```
REG ADD "HKLM\SOFTWARE\Microsoft\Active Setup\Installed Components\{A509B1A7-37E"
```

If Windows update has not finished yet, it's probably a good idea to stop and wait for it. Once you've restarted, continue with the following:

Get Chrome (optional)
```
choco install googlechrome
```

Install requirements (one line at a time, or in different Powershell windows to do so in parallel)

**Note what we're installing and omit things, like python, that you already have**

```
choco feature enable -n allowGlobalConfirmation
choco install python2
choco install vcredist2013 vcredist2015
choco install autoit
```

Restart PowerShell

Upgrade pip 
```
python -m pip install --upgrade pip
```

Add `C:\tools\python2\Scripts` to your system [path](http://www.howtogeek.com/118594/how-to-edit-your-system-path-for-easy-command-line-access/). 
If you did not install python with choco, use that instead of `C:\tools\python2\`, likely something like `C:\Python27`. 

Set your PYTHONPATH
```
setx PYTHONPATH "C:\tools\python2;C:\tools\python2\Lib;C:\tools\python2\DLLs;C:\tools\python2\Lib\lib-tk;C:\Workspace\universe" -m
```



Set TightVNC to not run on startup
```
& "C:/Program Files/TightVNC/tvnserver.exe" -remove
```

To allow loopback connections in TightVNC, select test locally in the "Access Control" tab (this is only needed if your agent is running locally)

Restart PowerShell

```
mkdir /Workspace
```

Download the binaries (about 2GB) you'll need to control the game (including the source) [from here](https://www.dropbox.com/s/ljx7uiodptxr0f3/universe-windows-envs.zip?dl=1)
and extract them to `C:\Workspace` that you just created. 

Now install our python dependencies
```
cd /Workspace/universe-windows-envs/vnc-windows
pip install -r requirements.txt
```

Set up the UNIVERSE_WINDOWS_ENVS_DIR environment variable

```
[Environment]::SetEnvironmentVariable("UNIVERSE_WINDOWS_ENVS_DIR", "C:\Workspace\universe-windows-envs", "User")
```

Restart PowerShell. Check that you can see these environment variables

```
Get-ChildItem Env:
```

### Set up your VNC

Click the TightVNC logo in the bottom right. If you don't see it, open TightVNC server.
- Disable `Serve Java Viewer to Web Clients`
- Set Main server port to `5900`
- Set the Primary Password to "openai"
- Add grcWindow to the Video tab under class names
![tightVNC-gta](https://dl.dropboxusercontent.com/u/9632169/tightvnc-gta.png)

Check that you can connect to the machine (from OSX)
```
open vnc://<your-windows-ip>:5900
```

### Start the environment automatically on system boot (AWS only)

Set the server to automatically log in our user as administrator. Enter your password in quotation marks. TODO: Pull this from an EC2 env variable.
```
$administrator_password = ENTER_YOUR_PASSWORD

$winlogon = "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\winlogon"
[Microsoft.Win32.Registry]::SetValue("$winlogon","AutoAdminLogon",1,[Microsoft.Win32.RegistryValueKind]::DWord)
[Microsoft.Win32.Registry]::SetValue("$winlogon","DefaultUserName","Administrator",[Microsoft.Win32.RegistryValueKind]::String)
[Microsoft.Win32.Registry]::SetValue("$winlogon","DefaultUserPassword","test",[Microsoft.Win32.RegistryValueKind]::String)
```

OR if you like GUI's, install and enter your password into [autologon](https://technet.microsoft.com/en-us/sysinternals/bb963905) to have the server logon on startup.

If you're just running an environment, you're done with the Windows setup portion! 

## Developing

Install Git (I recommend [GitHub for Windows](https://desktop.github.com/) and [doing this](http://stackoverflow.com/questions/4485059/git-bash-is-extremely-slow-in-windows-7-x64) to speed things up)

You will have already downloaded the git repo in `C:\Workspace` during setup.

You'll need to install Visual Studio Community 2013 (6GB) though. This takes ~10mins, so you can do the other tasks below while it's downloading.
```
choco install visualstudiocommunity2013
```

Make sure dependencies are downloaded by right clicking the Solution in the Solution Explorer and "Enable package restore"

If you get the following error
```
Error	4	error LNK1104: cannot open file 'libboost_system-vc120-mt-gd-1_61.lib'	C:\Workspace\universe-windows-envs\vnc-gtav\GTAVController\VisualStudio\LINK	GTAVController
```

Then open Tools -> Nuget Package Manager -> Package Manager Console and hit the `Restore` button in the banner that pops up in the console.

## Making Windows nicer

I'd recommend installing a few things that will make development easier

```
choco install consolez github pycharm-community atom
```

I highly recommend using [ConsoleZ](https://github.com/cbucher/console) as a replacement for PowerShell. Start it with:
```
C:\ProgramData\chocolatey\lib\ConsoleZ\tools\Console.exe
```

You can use my dotfiles to customize it
```
cd /Workspace
git clone https://github.com/nottombrown/dotfiles.git
Remove-Item -Recurse -Force "$env:USERPROFILE\AppData\Roaming\Console"
cmd.exe /c mklink /j "$env:USERPROFILE\AppData\Roaming\Console" "C:\Workspace\dotfiles\consolez"
```

You can also customize ConsoleZ by following [these directions](https://www.maketecheasier.com/console-2-windows-command-prompt-alternative/).

* Set PowerShell as your default shell (it's in `C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe`)
* Go to `settings > Hotkeys` and set commands for copy, paste, new tab and "send ctrl-c"
* Go to `settings > Mouse` and set left click to select

[Resharper](https://www.jetbrains.com/resharper/download/) for Visual Studio & C++ is also recommended, although it's paid-only after 30 days.

## Packages

 You can get installed packages with:
```
choco list -l
```

# Debugging

If the User-Script is having problems, you can look around the following files:
- `C:\Program Files\Amazon\Ec2ConfigService\Logs\Ec2ConfigLog.txt`
- `C:\CFN\LOG\cfn-init.log`
