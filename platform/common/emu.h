// (c) Copyright 2010 javicq, All rights reserved.
// Free for non-commercial use.

// For commercial use, separate licencing terms must be obtained.

#define _P_ENABLE_KEY 0x01
#define _P_ENABLE_TS 0x02
#define _P_ENABLE_ACC 0x04

typedef struct {
	unsigned short keymap[0x100];
	unsigned char inputFlags;
	int accelSensitivity;
} player_opt;

typedef struct {
	char lastRomFile[512];
	int EmuOpt;		// LSb->MSb: use_sram, show_fps, enable_sound, gzip_saves,
					// squidgehack, no_save_cfg_on_exit, <unused>, 16_bit_mode
					// craigix_ram, confirm_save, show_cd_leds, confirm_load
					// A_SNs_gamma, perfect_vsync, giz_scanlines, giz_dblbuff
					// vsync_mode, show_clock, no_frame_limitter
	int PicoOpt;  // used for config saving only, see Pico.h
	int PsndRate; // ditto
	int PicoRegion; // ditto
	int Frameskip;
	int CPUclock;
	int KeyBinds[32];
	int volume;
	int gamma;
	int JoyBinds[4][32];
	int PicoAutoRgnOrder;
	int PicoCDBuffers;
	int scaling; // maemo: 0=pixel doubling, 1=original size, 2=stretch to fill
	char displayTS;
	int screensaver;
	player_opt player[2];
} currentConfig_t;


extern currentConfig_t currentConfig;
extern char *PicoConfigFile;
extern unsigned char *rom_data;
extern char noticeMsg[64];
extern int state_slot;
extern int config_slot, config_slot_current;
extern unsigned char *movie_data;


int   emu_ReloadRom(void);
int   emu_SaveLoadGame(int load, int sram);
int   emu_ReadConfig(int game, int no_defaults);
int   emu_WriteConfig(int game);
char *emu_GetSaveFName(int load, int is_sram, int slot);
int   emu_checkSaveFile(int slot);
void  emu_setSaveStateCbs(int gz);
void  emu_updateMovie(void);
int   emu_cdCheck(int *pregion);
int   emu_findBios(int region, char **bios_file);
void  emu_textOut8 (int x, int y, const char *text);
void  emu_textOut16(int x, int y, const char *text);

