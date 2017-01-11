# universe-windows-envs

This repo contains the necessary components for running Windows games in [Universe](https://github.com/openai/universe). The initial environment we are releasing is GTA V.

## Setup

To setup the GTA V environment, follow the [readme here](vnc-gtav/README.md).

## Development
This repo contains a Visual Studio solution for hosting Universe environments in Windows. Unlike the other Universe environments, Windows does not run dockerized, so the environment runs natively. 

The [solution file](vnc-windows/CommonController/VisualStudio) is in `vnc-windows/CommonController/VisualStudio`. It currently consists of three projects:

0. CommonController
0. GTAVController
0. GTAVScriptHookProxy

CommonController consists of code that can be used across all environments such as the websocket protocol in `AgentConn`, and joystick control in `JoystickController`.

For more on specific environments (like [GTA](vnc-gtav/README.md#structure)) see their readme's.

### Structure

There are two channels by which information flows in Universe (websockets and VNC). In Windows, we use TightVNC and [Websocketpp](https://github.com/zaphoyd/websocketpp). TightVNC runs as a vanilla server with no modifications besides configuration to capture the game window only. This codebase therefore deals with the websockets side of things.

In addition to implementing the [websocket protocol](https://github.com/openai/universe/blob/master/universe/rewarder/remote.py) in C++, processes like the VNC server, the game, and the websocket server are managed [via python](https://github.com/openai/universe-windows-envs/blob/master/vnc-gtav/run_vnc_env.py)



