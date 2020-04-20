
#ifndef __MAEMO_H__
#define __MAEMO_H__

void maemo_init(void);
void maemo_deinit(void);
void maemo_quit();

/* video */
void maemo_video_flip(void);
void maemo_cls(void);

/* sound */
void maemo_start_sound(int rate, int bits, int stereo);
void maemo_sound_write(void *buff, int len);
void maemo_sound_volume(int l, int r);

/* joy */
unsigned long maemo_joystick_read(int allow_usb_joy);

extern void *md_screen;

enum MDButton {
	MDB_UP		= 0x01,
	MDB_DOWN	= 0x02,
	MDB_LEFT	= 0x04,
	MDB_RIGHT	= 0x08,
	MDB_B		= 0x10,
	MDB_C		= 0x20,
	MDB_A 		= 0x40,
	MDB_START 	= 0x80,
	MDB_X		= 0x100,
	MDB_Y		= 0x200,
	MDB_Z		= 0x400,
	MDB_MODE	= 0x800,
	CMD_QUIT 	= 0x8000,
	CMD_QUICKLOAD = 0x8008,
	CMD_QUICKSAVE = 0x800C
};

#endif
