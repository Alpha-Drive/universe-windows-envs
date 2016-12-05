vnc-windows
-----------
This is a statically linked library `CommonController.lib` that contains common code and settings for windows projects.

New Project
-----------------
Creating a new windows controller project is as easy as calling:
```
python.exe $env:UNIVERSE_ENVS_DIR\windows-boilerplate\new-windows-controller.py --name "AwesomeGame"
```

Property pages `.props` structure:

* `common.props` - properties common to all windows game controllers
* `YourGameController.vcxproj` - properties specific to your game, inherits from `common.props`
