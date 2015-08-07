# amd-saturation-changer
This project is created increase saturation and brightness while playings games for AMD graphics cards.

It checks the active list of process in windows and adjusts the saturation to levels given in the config file.
You must place a settings.ini file in the directoy of the executable.

### settings.ini
```
process_name=csgo.exe
normal_saturation=100
process_saturation=200
normal_brightness=0
process_brightness=25
normal_contrast=100
process_contrast=125
logical_display_id=0
```
