<?xml version="1.0" encoding="utf-8"?>
define(`choose', `dnl
ifelse(eval(MAEMO_MAJOR < 5), 1, `$1', `$2')dnl>
')dnl
define(`N_', `$*')dnl
<gconfschemafile>
  <schemalist>
    <schema>
      <key>/schemas/apps/maemo/picodrive/display-framerate</key>
      <applyto>/apps/maemo/picodrive/display-framerate</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Display framerate</short>
        <long>
          Display a framerate counter in the lower left corner.
        </long>
      </locale>
    </schema>
    <schema>
      <key>/schemas/apps/maemo/picodrive/frameskip</key>
      <applyto>/apps/maemo/picodrive/frameskip</applyto>
      <owner>picodrive</owner>
      <type>int</type>
      <default>0</default>
      <locale name="C">
        <short>Frameskip</short>
        <long>
          Skip this many frames after rendering one frame (or 0 for auto).
        </long>
      </locale>
    </schema>
    <schema>
      <key>/schemas/apps/maemo/picodrive/rom</key>
      <applyto>/apps/maemo/picodrive/rom</applyto>
      <owner>picodrive</owner>
      <type>string</type>
      <default></default>
      <locale name="C">
        <short>ROM to load</short>
        <long>
          Full path to the ROM file to load on next startup.
        </long>
      </locale>
    </schema>
    <schema>
      <key>/schemas/apps/maemo/picodrive/savedir</key>
      <applyto>/apps/maemo/picodrive/savedir</applyto>
      <owner>picodrive</owner>
      <type>string</type>
      <default></default>
      <locale name="C">
        <short>Saved games folder</short>
        <long>
          Last folder used in the load/save dialog.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/saver</key>
      <applyto>/apps/maemo/picodrive/saver</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>true</default>
      <locale name="C">
        <short>Enable power saving</short>
        <long>
          This will save and close the emulator when it is deactivated or
          the device enters idle state.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/sound</key>
      <applyto>/apps/maemo/picodrive/sound</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>true</default>
      <locale name="C">
        <short>Enable sound</short>
        <long>
          Enable emulation and output of sound.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/scaler</key>
      <applyto>/apps/maemo/picodrive/scaler</applyto>
      <owner>picodrive</owner>
      <type>int</type>
      <default>0</default>
      <locale name="C">
        <short>Scaler</short>
        <long>
          Name of the preferred scaler to use. Available scalers depend on 
          platform. Leave empty to select best scaler available.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/turbo</key>
      <applyto>/apps/maemo/picodrive/turbo</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Turbo mode</short>
        <long>
          Do not sleep at all between frames.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/player1/keyboard/enable</key>
      <applyto>/apps/maemo/picodrive/player1/keyboard/enable</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>true</default>
      <locale name="C">
        <short>Player 1 keyboard</short>
        <long>
          Enable key mappings for player 1.
        </long>
      </locale>
    </schema>
dnl Player 1 keybindings
define(`HELP', `')dnl
define(`BUTTON', `dnl
    <schema>
     <key>/schemas/apps/maemo/picodrive/player1/keyboard/$2</key>
      <applyto>/apps/maemo/picodrive/player1/keyboard/$2</applyto>
      <owner>picodrive</owner>
      <type>int</type>
      <default>choose($4,$5)</default>
      <locale name="C">
        <short>$1 button</short>
      </locale>
    </schema>
dnl')dnl
define(`ACTION', `dnl
    <schema>
     <key>/schemas/apps/maemo/picodrive/player1/keyboard/$2</key>
      <applyto>/apps/maemo/picodrive/player1/keyboard/$2</applyto>
      <owner>picodrive</owner>
      <type>int</type>
      <default>choose($4,$5)</default>
      <locale name="C">
        <short>$1 action</short>
      </locale>
    </schema>
dnl')dnl
define(`LAST', `')dnl
include(buttons.inc)dnl
undefine(`HELP')dnl
undefine(`BUTTON')dnl
undefine(`ACTION')dnl
undefine(`LAST')dnl
    <schema>
     <key>/schemas/apps/maemo/picodrive/player1/touchscreen/enable</key>
      <applyto>/apps/maemo/picodrive/player1/touchscreen/enable</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 1 touchscreen</short>
        <long>
          Enable touchscreen buttons for player 1.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/player1/touchscreen/show_buttons</key>
      <applyto>/apps/maemo/picodrive/player1/touchscreen/show_buttons</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 1 touchscreen show</short>
        <long>
          Show touchscreen buttons for player 1.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/player1/accelerometer/enable</key>
      <applyto>/apps/maemo/picodrive/player1/accelerometer/enable</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 1 accelerometer</short>
        <long>
          Enable accelerometer control for player 1.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/player1/accelerometer/sensitivity</key>
      <applyto>/apps/maemo/picodrive/player1/accelerometer/sensitivity</applyto>
      <owner>picodrive</owner>
      <type>int</type>
      <default>1</default>
      <locale name="C">
        <short>Player 1 accelerometer sensitivity</short>
        <long>
          Sensitivity of accelerometer control for player 1.
        </long>
      </locale>
    </schema>    
    <schema>
     <key>/schemas/apps/maemo/picodrive/player2/keyboard/enable</key>
      <applyto>/apps/maemo/picodrive/player2/keyboard/enable</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 2 keyboard</short>
        <long>
          Enable key mappings for player 2.
        </long>
      </locale>
    </schema>
dnl Player 2 keybindings
define(`HELP', `')dnl
define(`BUTTON', `dnl
    <schema>
     <key>/schemas/apps/maemo/picodrive/player2/keyboard/$2</key>
      <applyto>/apps/maemo/picodrive/player2/keyboard/$2</applyto>
      <owner>picodrive</owner>
      <type>int</type>
      <default>0</default>
      <locale name="C">
        <short>$1 button</short>
      </locale>
    </schema>
dnl')dnl
define(`ACTION', `')dnl
define(`LAST', `')dnl
include(buttons.inc)
undefine(`HELP')dnl
undefine(`BUTTON')dnl
undefine(`ACTION')dnl
undefine(`LAST')dnl
    <schema>
     <key>/schemas/apps/maemo/picodrive/player2/touchscreen/enable</key>
      <applyto>/apps/maemo/picodrive/player2/touchscreen/enable</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 2 touchscreen</short>
        <long>
          Enable touchscreen buttons for player 2.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/player2/touchscreen/show_buttons</key>
      <applyto>/apps/maemo/picodrive/player2/touchscreen/show_buttons</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 2 touchscreen show</short>
        <long>
          Show touchscreen buttons for player 2.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/picodrive/player2/accelerometer/enable</key>
      <applyto>/apps/maemo/picodrive/player2/accelerometer/enable</applyto>
      <owner>picodrive</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 2 accelerometer</short>
        <long>
          Enable accelerometer control for player 2.
        </long>
      </locale>
    </schema>    
  </schemalist>
</gconfschemafile>
