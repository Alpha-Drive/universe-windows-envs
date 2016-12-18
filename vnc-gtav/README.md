# Universe GTAV

This environment is focused on using GTAV as a self-driving car simulator.
Controls are continuous **steering** from right to left (-1 to 1) on the x-axis of the `JoyStickActionSpace` and reverse to forward **throttle** (-1 to 1) on the joystick's z-axis.

GTAV only runs on Windows which makes this environment unique. 
ML libraries do not traditionally have first-class [support](https://github.com/tensorflow/tensorflow/blob/master/RELEASE.md#release-0120) for [Windows](https://github.com/BVLC/caffe/tree/windows),
so we expect that you'll be using a separate \*nix machine for your agent.

## Requirements
* The [GTAV requirements](http://www.pcgamer.com/gta-5-system-requirements-announced/) - although Windows server 2012 will also work with the Steam version of the game
* 4GB for GTAVController and its dependencies

**Network**

Connecting to an AWS region [30ms away](http://www.cloudping.info/) at 2MBps with `go-vncdriver` yields 20FPS @ 100ms latency and is very playable by a human,
while a direct ethernet connection creates a nearly indistinguishable experience from playing on your PC at 60FPS.

Using the prebuilt AMI
-----------------------
* On AWS, in EC2, select launch instance. 
* Under Community AMIs, search for *universe-gtav-0.0.10* and select one of the following:
```
ami-96eb9cf9 ap-south-1
ami-d0e9e3b4 eu-west-2
ami-bc3b1acf eu-west-1
ami-118c5a7f ap-northeast-2
ami-6690f901 ap-northeast-1
ami-7dcf5711 sa-east-1
ami-5b27953f ca-central-1
ami-513d9232 ap-southeast-1
ami-ce1922ad ap-southeast-2
ami-9ead6df1 eu-central-1
ami-cfa9a1d8 us-east-1
ami-51cb9134 us-east-2
ami-0ef4a26e us-west-1
ami-2ff4404f us-west-2
```
* Choose the `g2.2xlarge` type in order to get the GPU required to run the game.
* Under <kbd>Configure Instance</kbd> -> <kbd>EBS-optimized instance</kbd>, check <kbd>Launch as EBS-optimized instance</kbd>
* Don't worry about the keypair
* The password for the instance is `d33pdriveisalive!`. Once you log in using Microsoft Remote Desktop or [Remmina](http://askubuntu.com/questions/154121/why-wont-remmina-connect-to-windows-7-remote-desktop/204161#204161), you’ll be asked to change the Administrator password. Change it to something.
* Open Steam and purchase the game on your new instance. 

_Note everything will feel very sluggish at first as the [SSD gets populated from S3](http://docs.aws.amazon.com/AWSEC2/latest/WindowsGuide/ebs-initialize.html). This is a one time thing and will be much faster on subsequent runs._

* Open the game and run <kbd>Story Mode</kbd> (WARNING: NSFW)
  * Steam will begin to download, recharge with some [meditation](http://marc.ucla.edu/mindful-meditations) for ~20 minutes until it completes. 
* Close GTA (<kbd>Esc</kbd> -> <kbd>Game</kbd> -> <kbd>Exit Game</kbd>)
* Run <kbd>install.bat</kbd> on the desktop (WARNING: If you play GTAV and have saved games or other settings, this will overwrite them. Your old settings will be backed up to `~/Documents`)
  
Now skip to [running the environment](#run-the-environment)

AMI setup from scratch
----------------------
* Get the ec2gaming AMI
  * On AWS, in EC2, select launch instance. 
  * Under Community AMIs, search for *ec2gaming* and select one of the following:
```
ami-017dbf6a (us-east)
ami-8735c5c3 (us-west-1)
ami-dfefeeef (us-west-2)
ami-20175557 (eu-west-1)
ami-e47842f9 (eu-central-1)
ami-60cd6260 (ap-northeast-1)
ami-8c5b5bde (ap-southeast-1)
ami-4d9eda77 (ap-southeast-2) 
```
  * Choose the `g2.2xlarge` type in order to get the GPU required to run the game.
  * In step four _Add Storage_, make sure to change the EBS size from 35GB to 250GB+ (GTAV is around ~80GB and we want some extra room as well).
  * Don't worry about the keypair
  * The password for the instance is `rRmbgYum8g`. Once you log in using Microsoft Remote Desktop or [Remmina](http://askubuntu.com/questions/154121/why-wont-remmina-connect-to-windows-7-remote-desktop/204161#204161), you’ll be asked to change the Administrator password. Change it to something.
  * When you log in, search for *disk management* and open _Create and format hard disk partitions_
    * Right click the C:\ drive and extend it to the full amount

Windows Setup
-------------
Follow the [vnc-windows setup](../vnc-windows/INSTALL.md)

Environment setup
-----------------
* Install the Steam version of the game (the standalone version does not work on Windows Server 2012 as it's missing the necessary Windows Media Feature Pack). Otherwise the standalone version should work, and has the convenient `--offline` flag to keep the game from updating, but currently only the Steam version is tested.
* Play GTAV to make sure it's working

#### Set an environment variable to point to your GTAV directory

For the Steam version installed in C:\ use:
```
[Environment]::SetEnvironmentVariable("GTAV_DIR", "C:\Program Files (x86)\Steam\steamapps\common\Grand Theft Auto V", "User")
```

For the standalone version in C:\
```
[Environment]::SetEnvironmentVariable("GTAV_DIR", "C:\Program Files\Rockstar Games\Grand Theft Auto V", "User")
```
Close GTAV if it's open.

Open a new PowerShell window.

Run the install script (WARNING: If you play GTAV and have saved games or other settings, this will overwrite them. Your old settings will be backed up to `~/Documents`)
```
cd $env:UNIVERSE_WINDOWS_ENVS_DIR\vnc-gtav
python install.py
```

Install Vjoy

[Download the installer](https://drive.google.com/file/d/0B2UgaM91sqeAVE4wWWh3emFDbms/view) and run it. This is locked to a known working of version of vJoySetup.exe hosted in my Google Drive. You can install from vJoy's source forge if you're feeling [rebellious](https://sourceforge.net/projects/vjoystick/files/). [The file will be in your Downloads folder, despite any signature errors you see.]


#### Xbox360ce setup
_Xbox360ce is a gamepad emulator that we will need in order to route control from vjoy to GTAV_
* Close GTAV if it's open
* Open x360ce_x64.exe in th GTAV folder and choose to create `xinput1_3.dll`
* Vjoy should then be automatically detected. Search for online settings and when it's done, click _Finish_
* Close xbox360ce
* Open the x360ce.ini and replace everything from `AxisToDPadDeadZone` down with [our config](https://gist.githubusercontent.com/crizCraig/f680f65653641412eba28c3c47421bcf/raw/4abd3be3802555f57d96389bf0a189dad8cd90de/x360ce.ini) 
* Save the file and reopen xbox360ce_x64.exe, your config should then look like this

![xbox360ce config](https://www.dropbox.com/s/adk9f5kme2weau6/Screen%20Shot%202016-12-12%20at%206.49.16%20PM.png?dl=1)

* To test things out, open GTAV, the "Vjoy Feeder Demo", and repoen xbox360ce - Try sliding Axis X to steer the car and Axiz Z to control the car's throttle.
* Close "Vjoy Feeder Demo" and xbox360ce_x64.exe so that GTAVController.exe can take command of the virtual joystick device.
* Also, if you ever find that your mouse behaves strangely after running the simulator, open Vjoy Feeder Demo to reset things.

### Add vJoy and AutoIT to your system path

Add the following to your [system path](http://www.howtogeek.com/118594/how-to-edit-your-system-path-for-easy-command-line-access/).

```
C:\Program Files\vJoy\x86;C:\Program Files (x86)\AutoIt3\AutoItX;
```

Restart Windows

### Run the environment

Open <kbd>run.bat</kbd> or run the following command in Powershell

```
python $env:UNIVERSE_WINDOWS_ENVS_DIR/vnc-gtav/run_vnc_env.py
```

The run script will now start GTAV if it's not started and send keys to load <kbd>Story Mode</kbd> if it's not already loaded.

Note that closing your RDP session will kill the environment (there is a nice logout of RDP batch file on the desktop that can close the RDP session without killing the env)

*ProTip*: To get back to the desktop from the game on hit the Windows key twice, or Alt+Tab

#### Skip reload
To avoid loading the saved game every episode (which takes ~40s), you can open <kbd>run skip reload.bat</kbd> or pass <kbd>-s</kbd> to run_vnc_env.py
```
python $env:UNIVERSE_WINDOWS_ENVS_DIR/vnc-gtav/run_vnc_env.py -s
```
This can speed up iterative development of non-ML work, but is bad for long training runs where the car can drive off the road and get into other irrecoverable states.

#### Have the server start on admin boot (likely AWS only) - still needs RDP session for GTAV to start
```
Copy-Item "$env:UNIVERSE_WINDOWS_ENVS_DIR/vnc-gtav/run_vnc_env_gtav_shortcut.lnk" "$env:USERPROFILE\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
```

## Connecting a client

Install [Universe](https://github.com/openai/universe)

### Random Agent

```
cd universe
bin/random_agent.py -e gtav.SaneDriving-v0 -r vnc://<your-windows-ip>:5900+15900 --fps 8
```

### Pretrained models

Please see [deepdrive-universe](https://github.com/deepdrive/deepdrive-universe)

## Development

Open the project with Visual Studio

```
vnc-windows\CommonController\VisualStudio\GymWindowsControllers.sln
```

## Structure

There are two Visual Studio projects that make up the environment *GTAVController* *GTAVScriptHookProxy*.

GTAVScriptHookProxy generates GTAVScriptHookProxy.asi which gets injected into the game and provides ScriptHookV information to GTAVController. This decouples us from the game executable and allows for easier debugging of most controller code.

`run_vnc_env.py` starts and monitors the above processes


