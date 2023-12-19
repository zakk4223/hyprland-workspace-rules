# Set workspace rules for monitors 


This plugin adds a keyword to the hyprland config that allows you to specify workspace rules for all workspaces on a monitor.


## Using hyprpm, Hyprland's official plugin manager (recommended)
1. Run `hyprpm add https://github.com/zakk4223/hyprWorkspaceLayouts` and wait for hyprpm to build the plugin.
2. Run `hyprpm enable hyprWorkspaceLayouts`


## Configuration 

The plugin adds just one keyword to the config: ```monitorwsrule```
The syntax is the same as the built in ```workspace``` keyword, except the workspace name/id/etc is replaced with either a display name, or the ```desc:``` just like the 'monitors' keyword.

```
monitorwsrule=DP-4,gapsIn:6,layoutopt:orientation:top
```

This will set the master layout's orientation to 'top' and the inner gaps to 6 for any workspace on display DP-4. 
