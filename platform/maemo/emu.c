// (c) Copyright 2010 javicq, All rights reserved.
// Free for non-commercial use.

// For commercial use, separate licencing terms must be obtained.

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>

#include "emu.h"
#include "maemo.h"
#include "../common/arm_utils.h"
#include "../common/fonts.h"
#include "../common/emu.h"

#include <Pico/PicoInt.h>
#include <Pico/Patch.h>
#include <Pico/sound/mix.h>
#include <zlib/zlib.h>

//#define PFRAMES

#ifdef BENCHMARK
#define OSD_FPS_X 220
#else
#define OSD_FPS_X 260
#endif

#define SCREEN_WIDTH 320

int engineState;
int select_exits = 0;

char romFileName[PATH_MAX];

static short __attribute__((aligned(4))) sndBuffer[2*44100/50];
static struct timeval noticeMsgTime = { 0, 0 };	// when started showing
static int osd_fps_x;
char noticeMsg[64];			// notice msg to draw
unsigned char *PicoDraw2FB = NULL;  // temporary buffer for alt renderer
int reset_timing = 0;

static void emu_msg_cb(const char *msg);
static void emu_msg_tray_open(void);

void emu_noticeMsgUpdated(void)
{
	gettimeofday(&noticeMsgTime, 0);
}

void emu_getMainDir(char *dst, int len)
{
	sprintf( dst, "%s/.picodrive/bios/", getenv( "HOME" ) );
	//strcpy( dst, "/opt/picodrive/bios/" );
	/*
	extern char **g_argv;
	int j;

	strncpy(dst, g_argv[0], len);
	len -= 32; // reserve
	if (len < 0) len = 0;
	dst[len] = 0;
	for (j = strlen(dst); j > 0; j--)
		if (dst[j] == '/') { dst[j+1] = 0; break; }*/
}

void emu_Init(void)
{
	// make temp buffer for alt renderer
	PicoDraw2FB = malloc((8+320)*(8+240+8));
	if (!PicoDraw2FB)
	{
		printf("PicoDraw2FB == 0\n");
	}

	PicoInit();
	PicoMessage = emu_msg_cb;
	PicoMCDopenTray = emu_msg_tray_open;
	//PicoMCDcloseTray = menu_loop_tray;
}

static void scaling_update(void)
{
	PicoOpt &= ~0x4100;
	PicoOpt |= 0x0100;
	/*switch (currentConfig.scaling) {
		default: break; // off
		case 1:  // hw hor
		case 2:  PicoOpt |=  0x0100; break; // hw hor+vert
		case 3:  PicoOpt |=  0x4000; break; // sw hor
	}*/
}



void emu_Deinit(void)
{
	// save SRAM
	/*if((currentConfig.EmuOpt & 1) && SRam.changed) {
		emu_SaveLoadGame(0, 1);
		SRam.changed = 0;
	}*/
/*
	if (!(currentConfig.EmuOpt & 0x20)) {
		FILE *f = fopen(PicoConfigFile, "r+b");
		if (!f) emu_WriteConfig(0);
		else {
			// if we already have config, reload it, except last ROM
			fseek(f, sizeof(currentConfig.lastRomFile), SEEK_SET);
			fread(&currentConfig.EmuOpt, 1, sizeof(currentConfig) - sizeof(currentConfig.lastRomFile), f);
			fseek(f, 0, SEEK_SET);
			fwrite(&currentConfig, 1, sizeof(currentConfig), f);
			fflush(f);
			fclose(f);
#ifndef NO_SYNC
			sync();
#endif
		}
	}
*/
	free(PicoDraw2FB);

	PicoExit();
}

void emu_setDefaultConfig(void)
{
	memset(&currentConfig, 0, sizeof(currentConfig));
	currentConfig.lastRomFile[0] = 0;
	currentConfig.EmuOpt  = 0x1f | 0x600; // | confirm_save, cd_leds
	currentConfig.PicoOpt = 0x0f | 0x20 | 0xe00 | 0x1000; // | use_940, cd_pcm, cd_cdda, scale/rot
	currentConfig.PsndRate = 22050; // 44100;
	currentConfig.PicoRegion = 0; // auto
	currentConfig.PicoAutoRgnOrder = 0x184; // US, EU, JP
	currentConfig.Frameskip = -1; // auto
	currentConfig.CPUclock = 200;
	currentConfig.volume = 100;
	currentConfig.KeyBinds[ 0] = 1<<0; // SACB RLDU
	currentConfig.KeyBinds[ 4] = 1<<1;
	currentConfig.KeyBinds[ 2] = 1<<2;
	currentConfig.KeyBinds[ 6] = 1<<3;
	currentConfig.KeyBinds[14] = 1<<4;
	currentConfig.KeyBinds[13] = 1<<5;
	currentConfig.KeyBinds[12] = 1<<6;
	currentConfig.KeyBinds[ 8] = 1<<7;
	currentConfig.KeyBinds[15] = 1<<26; // switch rend
	currentConfig.KeyBinds[10] = 1<<27; // save state
	currentConfig.KeyBinds[11] = 1<<28; // load state
	currentConfig.KeyBinds[23] = 1<<29; // vol up
	currentConfig.KeyBinds[22] = 1<<30; // vol down
	currentConfig.gamma = 100;
	currentConfig.PicoCDBuffers = 64;
	currentConfig.scaling = 0;
}

void osd_text(int x, int y, const char *text)
{
	int len = strlen(text)*8;
	int *p, i, h;
	x &= ~1; // align x
	len = (len+1) >> 1;
	for (h = 0; h < 8; h++) {
		p = (int *) ((unsigned short *) md_screen+x+SCREEN_WIDTH*(y+h));
		for (i = len; i; i--, p++) *p = (*p>>2)&0x39e7;
	}
	emu_textOut16(x, y, text);
}

static void cd_leds(void)
{
//	static
	int old_reg;
//	if (!((Pico_mcd->s68k_regs[0] ^ old_reg) & 3)) return; // no change // mmu hack problems?
	old_reg = Pico_mcd->s68k_regs[0];

	/*if ((PicoOpt&0x10)||!(currentConfig.EmuOpt&0x80)) {
		// 8-bit modes
		unsigned int col_g = (old_reg & 2) ? 0xc0c0c0c0 : 0xe0e0e0e0;
		unsigned int col_r = (old_reg & 1) ? 0xd0d0d0d0 : 0xe0e0e0e0;
		*(unsigned int *)((char *)gp2x_screen + SCREEN_WIDTH*2+ 4) =
		*(unsigned int *)((char *)gp2x_screen + SCREEN_WIDTH*3+ 4) =
		*(unsigned int *)((char *)gp2x_screen + SCREEN_WIDTH*4+ 4) = col_g;
		*(unsigned int *)((char *)gp2x_screen + SCREEN_WIDTH*2+12) =
		*(unsigned int *)((char *)gp2x_screen + SCREEN_WIDTH*3+12) =
		*(unsigned int *)((char *)gp2x_screen + SCREEN_WIDTH*4+12) = col_r;
	} else*/ {
		// 16-bit modes
		unsigned int *p = (unsigned int *)((short *)md_screen + SCREEN_WIDTH*2+4);
		unsigned int col_g = (old_reg & 2) ? 0x06000600 : 0;
		unsigned int col_r = (old_reg & 1) ? 0xc000c000 : 0;
		*p++ = col_g; *p++ = col_g; p+=2; *p++ = col_r; *p++ = col_r; p += SCREEN_WIDTH/2 - 12/2;
		*p++ = col_g; *p++ = col_g; p+=2; *p++ = col_r; *p++ = col_r; p += SCREEN_WIDTH/2 - 12/2;
		*p++ = col_g; *p++ = col_g; p+=2; *p++ = col_r; *p++ = col_r;
	}
}

static int EmuScan16(unsigned int num, void *sdata)
{
	//if (!(Pico.video.reg[1]&8)) num += 8;
	DrawLineDest = (unsigned short *) md_screen + SCREEN_WIDTH*(num+1);

	return 0;
}

int localPal[0x100];
static void (*vidCpyM2)(void *dest, void *src) = NULL;

static void blit(const char *fps, const char *notice)
{
	int emu_opt = currentConfig.EmuOpt;

	if (notice || (emu_opt & 2)) {
		int h = 232;
		//if (currentConfig.scaling == 2 && !(Pico.video.reg[1]&8)) h -= 8;
		if (notice) osd_text(4, h, notice);
		if (emu_opt & 2)
			osd_text(osd_fps_x, h, fps);
	}
	if ((emu_opt & 0x400) && (PicoMCD & 1))
		cd_leds();

	maemo_video_flip();
	DrawLineDest = md_screen;
}


// clears whole screen or just the notice area (in all buffers)
static void clearArea(int full)
{
	if (full) maemo_cls();
	else memset( md_screen+SCREEN_WIDTH*232*2, 0, SCREEN_WIDTH*8*2 );
}


static void vidResetMode(void)
{
	PicoDrawSetColorFormat(1);
	PicoScan = EmuScan16;
	PicoScan(0, 0);
	Pico.m.dirtyPal = 1;
}


static void emu_msg_cb(const char *msg)
{
	// 16bit accurate renderer
	memset( md_screen+SCREEN_WIDTH*232*2, 0, SCREEN_WIDTH*8*2 );
	osd_text( 4, 232, msg );
	gettimeofday(&noticeMsgTime, 0);
	noticeMsgTime.tv_sec -= 2;

	/* assumption: emu_msg_cb gets called only when something slow is about to happen */
	reset_timing = 1;
}

static void emu_state_cb(const char *str)
{
	clearArea(0);
	blit("", str);
}

static void emu_msg_tray_open(void)
{
	strcpy(noticeMsg, "CD tray opened");
	gettimeofday(&noticeMsgTime, 0);
}

static void update_volume(int has_changed, int is_up)
{
	static int prev_frame = 0, wait_frames = 0;
	int vol = currentConfig.volume;

	if (has_changed)
	{
		if (vol < 5 && (PicoOpt&8) && prev_frame == Pico.m.frame_count - 1 && wait_frames < 12)
			wait_frames++;
		else {
			if (is_up) {
				if (vol < 99) vol++;
			} else {
				if (vol >  0) vol--;
			}
			wait_frames = 0;
			maemo_sound_volume(vol, vol);
			currentConfig.volume = vol;
		}
		sprintf(noticeMsg, "VOL: %02i", vol);
		gettimeofday(&noticeMsgTime, 0);
		prev_frame = Pico.m.frame_count;
	}

	// set the right mixer func
	if (!(PicoOpt&8)) return; // just use defaults for mono
	if (vol >= 5)
		PsndMix_32_to_16l = mix_32_to_16l_stereo;
	else {
		mix_32_to_16l_level = 5 - vol;
		PsndMix_32_to_16l = mix_32_to_16l_stereo_lvl;
	}
}

static void change_fast_forward(int set_on)
{
	static void *set_PsndOut = NULL;
	static int set_Frameskip, set_EmuOpt, is_on = 0;

	if (set_on && !is_on) {
		set_PsndOut = PsndOut;
		set_Frameskip = currentConfig.Frameskip;
		set_EmuOpt = currentConfig.EmuOpt;
		PsndOut = NULL;
		currentConfig.Frameskip = 8;
		currentConfig.EmuOpt &= ~4;
		is_on = 1;
	}
	else if (!set_on && is_on) {
		PsndOut = set_PsndOut;
		currentConfig.Frameskip = set_Frameskip;
		currentConfig.EmuOpt = set_EmuOpt;
		PsndRerate(1);
		update_volume(0, 0);
		reset_timing = 1;
		is_on = 0;
	}
}

void emu_set_state( int state ) {
	engineState = state;
}

static void updateKeys(void)
{
	unsigned long keys;

	keys = maemo_joystick_read(0);
	PicoPad[0] = keys & 0xFFFF;
	PicoPad[1] = keys >> 16;
}


static void updateSound(int len)
{
	if (PicoOpt&8) len<<=1;

	/* avoid writing audio when lagging behind to prevent audio lag */
	if (PicoSkipFrame != 2)
		maemo_sound_write(PsndOut, len<<1);
}


static void SkipFrame(int do_audio)
{
	PicoSkipFrame=do_audio ? 1 : 2;
	PicoFrame();
	PicoSkipFrame=0;
}

static inline void simpleWait(int thissec, int lim_time)
{
	struct timeval tval;

	//spend_cycles(1024);
	//pthread_yield();
	gettimeofday(&tval, 0);
	if(thissec != tval.tv_sec) tval.tv_usec+=1000000;

	while(tval.tv_usec < lim_time)
	{
		//spend_cycles(1024);
		pthread_yield();
		gettimeofday(&tval, 0);
		if(thissec != tval.tv_sec) tval.tv_usec+=1000000;
	}
}

void emu_Loop(void)
{
	static int PsndRate_old = 0, PicoOpt_old = 0, EmuOpt_old = 0, pal_old = 0;
	char fpsbuff[24]; // fps count c string
	struct timeval tval; // timing
	int thissec = 0, frames_done = 0, frames_shown = 0, oldmodes = 0;
	int target_fps, target_frametime, lim_time, vsync_offset, i;
	char *notice = 0;

	printf("entered emu_Loop()\n");

	EmuOpt_old = currentConfig.EmuOpt;
	fpsbuff[0] = 0;

	// make sure we are in correct mode
	vidResetMode();
	scaling_update();
	Pico.m.dirtyPal = 1;
	oldmodes = ((Pico.video.reg[12]&1)<<2) ^ 0xc;

	// pal/ntsc might have changed, reset related stuff
	target_fps = Pico.m.pal ? 50 : 60;
	target_frametime = 1000000/target_fps;
	reset_timing = 1;

	// prepare sound stuff
	if (currentConfig.EmuOpt & 4)
	{
		int snd_excess_add;
		if (PsndRate != PsndRate_old || (PicoOpt&0x20b) != (PicoOpt_old&0x20b) || Pico.m.pal != pal_old ) {
			PsndRerate(Pico.m.frame_count ? 1 : 0);
		}
		snd_excess_add = ((PsndRate - PsndLen*target_fps)<<16) / target_fps;
		printf("starting audio: %i len: %i (ex: %04x) stereo: %i, pal: %i\n",
			PsndRate, PsndLen, snd_excess_add, (PicoOpt&8)>>3, Pico.m.pal);
		maemo_start_sound(PsndRate, 16, (PicoOpt&8)>>3);
		maemo_sound_volume(currentConfig.volume, currentConfig.volume);
		PicoWriteSound = updateSound;
		update_volume(0, 0);
		memset(sndBuffer, 0, sizeof(sndBuffer));
		PsndOut = sndBuffer;
		PsndRate_old = PsndRate;
		PicoOpt_old  = PicoOpt;
		pal_old = Pico.m.pal;
	} else {
		PsndOut = NULL;
	}

	// prepare CD buffer
	if (PicoMCD & 1) PicoCDBufferInit();

	// calc vsync offset to sync timing code with vsync
	if (currentConfig.EmuOpt&0x2000) {
		gettimeofday(&tval, 0);
		gettimeofday(&tval, 0);
		vsync_offset = tval.tv_usec;
		while (vsync_offset >= target_frametime)
			vsync_offset -= target_frametime;
		if (!vsync_offset) vsync_offset++;
		printf("vsync_offset: %i\n", vsync_offset);
	} else
		vsync_offset = 0;

	// loop?
	while (engineState == PGS_Running)
	{
		int modes;

		gettimeofday(&tval, 0);
		if (reset_timing) {
			reset_timing = 0;
			thissec = tval.tv_sec;
			frames_shown = frames_done = tval.tv_usec/target_frametime;
		}

		// show notice message?
		if (noticeMsgTime.tv_sec)
		{
			static int noticeMsgSum;
			if((tval.tv_sec*1000000+tval.tv_usec) - (noticeMsgTime.tv_sec*1000000+noticeMsgTime.tv_usec) > 2000000) { // > 2.0 sec
				noticeMsgTime.tv_sec = noticeMsgTime.tv_usec = 0;
				clearArea(0);
				notice = 0;
			} else {
				int sum = noticeMsg[0]+noticeMsg[1]+noticeMsg[2];
				if (sum != noticeMsgSum) { clearArea(0); noticeMsgSum = sum; }
				notice = noticeMsg;
			}
		}

		// check for mode changes
		modes = ((Pico.video.reg[12]&1)<<2)|(Pico.video.reg[1]&8);
		if (modes != oldmodes)
		{
			int scalex = 320;
			osd_fps_x = OSD_FPS_X;
			if (modes & 4) {
				vidCpyM2 = vidCpyM2_40col;
			} else {
				if (PicoOpt & 0x100) {
					vidCpyM2 = vidCpyM2_32col_nobord;
					scalex = 256;
					osd_fps_x = OSD_FPS_X - 64;
				} else {
					vidCpyM2 = vidCpyM2_32col;
				}
			}
			/*if (currentConfig.scaling == 2 && !(modes&8)) // want vertical scaling and game is not in 240 line mode
			     gp2x_video_RGB_setscaling(8, scalex, 224);
			else gp2x_video_RGB_setscaling(0, scalex, 240);*/
			oldmodes = modes;
			clearArea(1);
		}

		// second changed?
		if (thissec != tval.tv_sec)
		{
			if (currentConfig.EmuOpt & 2)
				sprintf(fpsbuff, "%02i/%02i", frames_shown, frames_done);
			if (fpsbuff[5] == 0) { fpsbuff[5] = fpsbuff[6] = ' '; fpsbuff[7] = 0; }
			thissec = tval.tv_sec;

			if (PsndOut == 0 && currentConfig.Frameskip >= 0) {
				frames_done = frames_shown = 0;
			} else {
				// it is quite common for this implementation to leave 1 fame unfinished
				// when second changes, but we don't want buffer to starve.
				if(PsndOut && frames_done < target_fps && frames_done > target_fps-5) {
					updateKeys();
					SkipFrame(1); frames_done++;
				}

				frames_done  -= target_fps; if (frames_done  < 0) frames_done  = 0;
				frames_shown -= target_fps; if (frames_shown < 0) frames_shown = 0;
				if (frames_shown > frames_done) frames_shown = frames_done;
			}
		}
#ifdef PFRAMES
		sprintf(fpsbuff, "%i", Pico.m.frame_count);
#endif

		lim_time = (frames_done+1) * target_frametime + vsync_offset;
		if(currentConfig.Frameskip >= 0) { // frameskip enabled
			for(i = 0; i < currentConfig.Frameskip; i++) {
				updateKeys();
				SkipFrame(1); frames_done++;
				if (PsndOut && !reset_timing) { // do framelimitting if sound is enabled
					gettimeofday(&tval, 0);
					if(thissec != tval.tv_sec) tval.tv_usec+=1000000;
					if(tval.tv_usec < lim_time) { // we are too fast
						simpleWait(thissec, lim_time);
					}
				}
				lim_time += target_frametime;
			}
		} else if(tval.tv_usec > lim_time) { // auto frameskip
			// no time left for this frame - skip
			if (tval.tv_usec - lim_time >= 300000) {
				/* something caused a slowdown for us (disk access? cache flush?)
				 * try to recover by resetting timing... */
				reset_timing = 1;
				continue;
			}
			updateKeys();
			SkipFrame(tval.tv_usec < lim_time+target_frametime*2); frames_done++;
			continue;
		}

		updateKeys();
		PicoFrame();

		// check time
		gettimeofday(&tval, 0);
		if (thissec != tval.tv_sec) tval.tv_usec+=1000000;

		if (currentConfig.Frameskip < 0 && tval.tv_usec - lim_time >= 300000) // slowdown detection
			reset_timing = 1;
		else if (PsndOut != NULL || currentConfig.Frameskip < 0)
		{
			// sleep or vsync if we are still too fast
			// usleep sleeps for ~20ms minimum, so it is not a solution here
			if (!reset_timing && tval.tv_usec < lim_time)
			{
				// we are too fast
				if (vsync_offset) {
					if (lim_time - tval.tv_usec > target_frametime/2)
						simpleWait(thissec, lim_time - target_frametime/4);
					//gp2x_video_wait_vsync();
				} else {
					simpleWait(thissec, lim_time);
				}
			}
		}

		blit(fpsbuff, notice);

		frames_done++; frames_shown++;
	}

	//change_fast_forward(0);

	if (PicoMCD & 1) PicoCDBufferFree();
/*
	// save SRAM
	if((currentConfig.EmuOpt & 1) && SRam.changed) {
		emu_state_cb("Writing SRAM/BRAM..");
		emu_SaveLoadGame(0, 1);
		SRam.changed = 0;
	}*/
}

void emu_ResetGame(void)
{
	PicoReset(0);
	reset_timing = 1;
}

static void build_snapshot_path( char* path, int slot ) {
	sprintf( path, "%s/.picodrive", getenv( "HOME" ) );
	mkdir( path, 0755 );
	char* ptr = strrchr( romFileName, '/' );
	if( ptr ) ptr++; else ptr = romFileName;
	if( slot < 0 )
		sprintf( path, "%s/.picodrive/%s.pds", getenv("HOME"), ptr );
	else
		sprintf( path, "%s/.picodrive/%s.%d.pds", getenv("HOME"), ptr, slot+1 );
}

static int emu_snapshot( int save, int slot ) {
	char snapshot_path[PATH_MAX];
	build_snapshot_path( snapshot_path, slot );
	emu_setSaveStateCbs(1);
	gzFile f = gzopen( snapshot_path, save ? "wb" : "rb" );
	if( !f ) {
		fprintf( stderr, "Cannot open snapshot file\n" );
		return -1;
	}
	if( save ) gzsetparams(f, 9, Z_DEFAULT_STRATEGY);
	PmovState( save ? 5 : 6, f);
	areaClose(f);
	if( !save ) Pico.m.dirtyPal=1;
	return 0;
}

void emu_load_snapshot() {
	emu_snapshot(0,-1);
}

void emu_save_snapshot() {
	emu_snapshot(1,-1);
}

void emu_quickload( int slot ) {
	if( emu_snapshot(0,slot) ) {
		sprintf( noticeMsg, "LOAD SLOT %d FAILED", slot+1 );
		emu_noticeMsgUpdated();
	} else {
		sprintf( noticeMsg, "LOAD SLOT %d OK", slot+1 );
		emu_noticeMsgUpdated();
	}
}

void emu_quicksave( int slot ) {
	if( emu_snapshot(1,slot) ) {
		sprintf( noticeMsg, "SAVE SLOT %d FAILED", slot+1 );
		emu_noticeMsgUpdated();
	} else {
		sprintf( noticeMsg, "SAVE SLOT %d OK", slot+1 );
		emu_noticeMsgUpdated();
	}
}
