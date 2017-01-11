# universe-windows-envs

## Setup
Follow the setup for our initial windows env, [GTA](vnc-gtav/README.md).

## Development
This repo contains a Visual Studio solution for hosting Universe environments in Windows. Unlike the other Universe environments, Windows does not run dockerized, so the environment runs natively. 

The [solution file](vnc-windows/CommonController/VisualStudio) is in `vnc-windows/CommonController/VisualStudio`. It currently consists of three projects:

0. CommonController
0. GTAVController
0. GTAVScriptHookProxy

## Structure

There are two channels by which information flows in Universe (websockets and VNC). In Windows, we use TightVNC and [Websocketpp](https://github.com/zaphoyd/websocketpp). TightVNC runs as a vanilla server with no modifications besides configuration to capture the game window only. This codebase therefore deals with the websockets side of things.

### VNC

### Websockets

For more on the structure of specific environments (like [GTA](vnc-gtav/README.md#structure)) see their readmes.

