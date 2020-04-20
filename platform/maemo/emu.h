// (c) Copyright 2010 javicq, All rights reserved.
// Free for non-commercial use.

// For commercial use, separate licencing terms must be obtained.

// engine states
enum TPicoGameState {
	PGS_Paused = 1,
	PGS_Running,
	PGS_Quit,
	PGS_KeyConfig,
	PGS_ReloadRom,
	PGS_ReloadRomState,
	PGS_Menu,
	PGS_RestartRun,
	PGS_QuickSave,
	PGS_QuickLoad
};

extern char romFileName[];
extern int engineState;
extern int quickSlot;

void emu_Init(void);
void emu_Deinit(void);
void emu_Loop(void);
void emu_ResetGame(void);
void emu_forcedFrame(void);
void emu_set_state( int state );
void emu_load_snapshot();
void emu_save_snapshot();
void emu_quicksave( int slot );
void emu_quickload( int slot );

void osd_text(int x, int y, const char *text);

