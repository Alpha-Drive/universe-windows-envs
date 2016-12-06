This environment is focused on using GTAV as a self-driving car simulator with emphasis on learning to drive a real car vs playing GTAV as you normally would.
Controls are continuous **steering** from right to left (-1 to 1) on the x-axis of the `JoyStickActionSpace` and reverse to forward **throttle** (-1 to 1) on the joystick's z-axis.

GTAV only runs on Windows which makes this environment unique. 
ML libraries do not traditionally have first-class [support](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/contrib/cmake) for [Windows](https://github.com/BVLC/caffe/tree/windows),
so we expect that you'll be using a separate \*nix machine for your agent.

## Requirements
* The [GTAV requirements](http://www.pcgamer.com/gta-5-system-requirements-announced/) - although Windows server 2012 will also work with the Steam version of the game
* 4GB for GTAVController and its dependencies

**Network**

Connecting to an AWS region [30ms away](http://www.cloudping.info/) at 2MBps with `go-vncdriver` yields 20FPS @ 100ms latency and is very playable by a human,
while a direct ethernet connection creates a nearly indistinguishable experience from playing on your PC at 60FPS.


AMI setup
---------
_If you want to run the environment on AWS, do the following first_
* Get ec2gaming AMI
  * On AWS, in EC2, select launch instance. 
  * Under Community AMIs, search for *ec2gaming* and select one of the following:
```
ami-017dbf6a	(us-east)
ami-8735c5c3	(us-west-1)
ami-dfefeeef	(us-west-2)
ami-20175557	(eu-west-1)
ami-e47842f9	(eu-central-1)
ami-60cd6260	(ap-northeast-1)
ami-8c5b5bde	(ap-southeast-1)
ami-4d9eda77	(ap-southeast-2)
```
  * Choose the `g2.2xlarge` type in order to get the GPU required to run the game.
  * In step four _Add Storage_, make sure to change the EBS size from 35GB to 250GB+ (GTAV is around ~80GB and we want some extra room as well).
  * Don't worry about the keypair
  * The password for the instance is `rRmbgYum8g`. Once you log in using Microsoft Remote Desktop, you’ll be asked to change the Administrator password. Change it to something. If you’re on Windows, you’ll need to use a Mac or Linux or a mobile client to reset the password since there’s a bug in the Windows Remote Desktop client.
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

Run the install script (WARNING: If you play GTAV and have saved games or other settings, this will overwrite them. Your settings will be backed up to `~/Documents`)
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

![xbox360ce config](https://www.dropbox.com/s/5a2huyxdcby1qjz/Screenshot%202016-10-29%2014.59.17.png?dl=1)

* To test things out, open GTAV, the "Vjoy Feeder Demo", and repoen xbox360ce - Try sliding Axis X to steer the car and Axiz Z to control the car's throttle.
* Close "Vjoy Feeder Demo" and xbox360ce_x64.exe so that GTAVController.exe can take command of the virtual joystick device.
* Also, if you ever find that your mouse behaves strangely after running the simulator, open Vjoy Feeder Demo to reset things.

### Add vJoy and AutoIT to your system path

Add the following to your [system path](http://www.howtogeek.com/118594/how-to-edit-your-system-path-for-easy-command-line-access/).

```
C:\Program Files\vJoy\x86;C:\Program Files (x86)\AutoIt3\AutoItX;
```

### Run the environment
In PowerShell
```
python.exe $env:UNIVERSE_WINDOWS_ENVS_DIR/vnc-gtav/run_vnc_env.py
```

Have the server start on admin boot (likely AWS only)
```
Copy-Item "$env:UNIVERSE_WINDOWS_ENVS_DIR/vnc-gtav/run_vnc_env_gtav_shortcut.lnk" "$env:USERPROFILE\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
```

## Connecting a client

In \*nix, install [universe](https://github.com/openai/universe)

Run your agent
```
cd universe
bin/random_agent.py -e gtav.SaneDriving-v0 -r vnc://<your-windows-ip>:5900+15900 --fps 20
```

## Development

Open the project with Visual Studio

```
start "C:\Workspace\universe-windows-envs\vnc-windows\GTAVController\VisualStudio\GTAVController.sln"
```

## Structure

There are two Visual Studio projects that make up the environment *GTAVController* *GTAVScriptHookProxy*.

GTAVScriptHookProxy generates GTAVScriptHookProxy.asi which gets injected into the game and provides ScriptHookV information to GTAVController. This decouples us from the game executable and allows for easier debugging of most controller code.

`run_vnc_env.py` starts and monitors the above processes


