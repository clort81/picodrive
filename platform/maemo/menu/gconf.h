#ifndef _GUI_GCONF_H_
#define _GUI_GCONF_H_

#define kGConfPath "/apps/maemo/picodrive"
#define kGConfRomFile kGConfPath "/" "rom"
#define kGConfSaver kGConfPath "/" "saver"
#define kGConfSound kGConfPath "/" "sound"
#define kGConfTurboMode kGConfPath "/" "turbo"
#define kGConfFrameskip kGConfPath "/" "frameskip"
#define kGConfTransparency kGConfPath "/" "transparency"
#define kGConfScaler kGConfPath "/" "scaler"
#define kGConfDisplayFramerate kGConfPath "/" "display-framerate"
#define kGConfSpeedhacks kGConfPath "/" "speedhacks"

#define kGConfSaveDir kGConfPath "/" "savedir"

#define kGConfPlayerPathBufferLen 128
#define kGConfPlayerPath kGConfPath "/player%d"

#define kGConfPlayerKeyboardPath "/" "keyboard"
#define kGConfPlayerKeyboardEnable kGConfPlayerKeyboardPath "/" "enable"

#define kGConfPlayerTouchscreenPath "/" "touchscreen"
#define kGConfPlayerTouchscreenEnable kGConfPlayerTouchscreenPath "/" "enable"
#define kGConfPlayerTouchscreenShow kGConfPlayerTouchscreenPath "/" "show_buttons"

#define kGConfPlayerAccelerometerPath "/" "accelerometer"
#define kGConfPlayerAccelerometerEnable kGConfPlayerAccelerometerPath "/" "enable"
#define kGConfPlayerAccelerometerZero kGConfPlayerAccelerometerPath "/" "zero"
#define kGConfPlayerAccelerometerSensitivity kGConfPlayerAccelerometerPath "/" "sensitivity"

#endif

