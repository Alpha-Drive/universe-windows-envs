# universe-windows-envs

This repo contains a Visual Studio solution for hosting Universe environments in Windows. Unlike the other Universe environments, Windows does not run dockerized, so the environment runs natively. 

The [solution file](vnc-windows/CommonController/VisualStudio) is in `vnc-windows/CommonController/VisualStudio`. It currently consists of three projects:

0. CommonController
0. GTAVController
0. GTAVScriptHookProxy

For more on the structure of specific environments (like [GTA](vnc-gtav/README.md#structure)) see their readmes.

## Setup
Follow the setup for our initial windows env, [GTA](vnc-gtav/README.md).
