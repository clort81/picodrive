
/*
	MAEMO specific emulator code

	Uses SDL for video, audio and event processing

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <glib.h>
#include <pthread.h>
#include <gst/gst.h>

#include "maemo.h"
#include "../common/arm_utils.h"
#include "emu.h"
#include "../common/emu.h"
#include "../../Pico/PicoInt.h"

#define min(a,b) (a<b?a:b)

int quickSlot;

extern gboolean osso_events( gboolean block );

static int running = 0;
static pthread_t sdl_thread;

void *md_screen;

#define SND_BUFFER_SIZE 0x200000
static unsigned char sound_buffer[SND_BUFFER_SIZE];
static volatile int soundptrr, soundptrw;

static SDL_Surface* screen;
static unsigned long keystates;
static unsigned long mousestates;
static unsigned long accelstates;

static GstElement* pipeline;

static struct {
	char* file;
	char request;
	char playing;
	gint64 seek;
} music_request;

static inline void check_music() {
}

static void sdl_key( int key, int press ) {
	unsigned long cmd = currentConfig.player[0].keymap[key];
	if( cmd < 0x8000 ) {
		if( currentConfig.player[0].inputFlags & _P_ENABLE_KEY ) {
			if( press ) keystates |= cmd; else keystates &= ~cmd;
		}
	} else switch( cmd ) {
		case CMD_QUIT:
			running = 0;
			maemo_quit();
			break;
		case CMD_QUICKLOAD: case CMD_QUICKLOAD + 1: case CMD_QUICKLOAD + 2: case CMD_QUICKLOAD + 3:
			emu_set_state( PGS_QuickLoad );
			quickSlot = cmd & 0x03;
			break;
		case CMD_QUICKSAVE: case CMD_QUICKSAVE + 1: case CMD_QUICKSAVE + 2: case CMD_QUICKSAVE + 3:
			emu_set_state( PGS_QuickSave );
			quickSlot = cmd & 0x03;
			break;
	}
	cmd = currentConfig.player[1].keymap[key];
	if( cmd < 0x8000 && currentConfig.player[1].inputFlags & _P_ENABLE_KEY ) {
		cmd <<= 16;
		if( press ) keystates |= cmd; else keystates &= ~cmd;
	}
}

static void sdl_mouse( int x, int y, int mode ) {
	if( ! (mode & 0x01) ) mousestates = 0;
	if( !mode ) return;
	unsigned long tmpmouse = 0;
#define set( cmd ) tmpmouse |= cmd
	if( x >= 0 && x <= 300 && y >= 0 && y <= 480 ) {
		if( x >= 0 && x <= 100 ) set( MDB_LEFT );
		if( x >= 200 && x <= 300 ) set( MDB_RIGHT );
		if( y >= 0 && y <= 160 ) set( MDB_UP );
		if( y >= 320 && y <= 480 ) set( MDB_DOWN );
	} else if( x >= 700 && x <= 800 && y >= 0 && y <= 480) {
		if( y <= 160 ) set( MDB_A );
		else if( y <= 320 ) set( MDB_B );
		else set( MDB_C );
	}
	if( currentConfig.player[0].inputFlags & _P_ENABLE_TS ) mousestates |= tmpmouse;
	if( currentConfig.player[1].inputFlags & _P_ENABLE_TS ) mousestates |= tmpmouse << 16;
}

static void read_accelerometer() {
	if( ! (currentConfig.player[0].inputFlags & _P_ENABLE_ACC || currentConfig.player[1].inputFlags & _P_ENABLE_ACC) ) return;
	accelstates = 0;
	int x, y, z;
	FILE* f = fopen( "/sys/class/i2c-adapter/i2c-3/3-001d/coord", "r" );
	if( !f ) return;
	fscanf( f, "%d %d %d", &x, &y, &z );
	fclose( f );
	if( currentConfig.player[0].inputFlags & _P_ENABLE_ACC ) {
		int sens = currentConfig.player[0].accelSensitivity;
		if( x > sens ) accelstates |= MDB_LEFT;
		else if( x < -sens ) accelstates |= MDB_RIGHT;
		if( y > sens ) accelstates |= MDB_UP;
		else if( y < -sens ) accelstates |= MDB_DOWN;
	}
	if( currentConfig.player[1].inputFlags & _P_ENABLE_ACC ) {
		int sens = currentConfig.player[1].accelSensitivity;
		if( x > sens ) accelstates |= MDB_LEFT << 16;
		else if( x < -sens ) accelstates |= MDB_RIGHT << 16;
		if( y > sens ) accelstates |= MDB_UP << 16;
		else if( y < -sens ) accelstates |= MDB_DOWN << 16;
	}
}

static void* SDL_Loop( void* obj ) {
	while( running ) {
		SDL_Event event;
		while( SDL_PollEvent( &event ) ) {
			switch( event.type ) {
			case SDL_MOUSEBUTTONDOWN:
				sdl_mouse( event.button.x, event.button.y, 1 );
				break;
			case SDL_MOUSEBUTTONUP:
				sdl_mouse( event.button.x, event.button.y, 0 );
				break;
			case SDL_MOUSEMOTION:
				sdl_mouse( event.button.x, event.button.y, 2 );
				break;
			case SDL_KEYDOWN:
				if( event.key.keysym.sym == SDLK_ESCAPE && event.key.keysym.mod & KMOD_CTRL ) maemo_quit();
				else sdl_key( event.key.keysym.scancode, 1 );
				break;
			case SDL_KEYUP:
				sdl_key( event.key.keysym.scancode, 0 );
				break;
			case SDL_QUIT:
				maemo_quit();
				break;
			}
		}
		read_accelerometer();
		check_music();
		while( osso_events( FALSE ) );
		SDL_Delay( 50 );
	}
	return NULL;
}

static void audio_loop( void* userdata, Uint8* stream, int len ) {
	register int ptrr = soundptrr, ptrw = soundptrw;
	int total = ptrw - ptrr;
	if( total < 0 ) {
		total = min( SND_BUFFER_SIZE - ptrr, len );
		memcpy( stream, sound_buffer + ptrr, total );
		stream += total;
		len -= total;
		ptrr = ( ptrr + total ) % SND_BUFFER_SIZE;
		total = ptrw;
	}
	total = min( total, len );
	memcpy( stream, sound_buffer + ptrr, total );
	soundptrr = ( ptrr + total ) % SND_BUFFER_SIZE;
}

void maemo_quit() {
	emu_set_state( PGS_Quit );
	running = 0;
}

/* video stuff */

static inline void disp_ts_buttons() {
	register int i, j;
	register unsigned short* buffer = screen->pixels;
	for( i = 1; i <= 3; i++ )
		for( j = 0; j < 480; j+=2 )
			buffer[j*800+i*100] = 0xFFFF;
	for( i = 1; i < 3; i++ )
		for( j = 0; j < 300; j+=2 )
			buffer[j+i*160*800] = 0xFFFF;
	for( i = 0; i < 480; i+=2 )
		buffer[i*800+700] = 0xFFFF;
	for( i = 1; i < 3; i++ )
		for( j = 700; j < 800; j+=2 )
			buffer[i*160*800+j] = 0xFFFF;
}

void maemo_video_flip(void)
{
#define clerp16(a,b) ( ( ( ( a >> 12 ) + ( b >> 12 ) ) & 0x1F ) << 11 | ( ( ( a >> 6 ) + ( b >> 6 ) ) & 0x3F ) << 5 | ( ( ( a >> 1 ) + ( b >> 1 ) ) & 0x1F ) )
	register int i, j;
	register unsigned short *buffer = md_screen;
	unsigned short *frame = screen->pixels;
	register unsigned short *scan1 = frame;
	register unsigned short *scan2 = frame;
	switch( currentConfig.scaling ) {
	case 1:
		for( j = 0; j < 240; j++ ) {
			scan1 = frame + j*800 + 96000 + 240;
			for( i = 0; i < 320; i++ ) {
				*scan1++ = *buffer++;
			}
		}
		break;
	case 0: {
		register int offh = Pico.video.reg[12]&1 ? 80 : 144;
		for( j = 0; j < 240; j++ ) {
			scan1 = frame + j*800*2 + offh;
			scan2 = frame + j*800*2 + 800 + offh;
			for( i = 0; i < 320; i++ ) {
				register unsigned short pix = *buffer++;
				*scan1++ = pix;
				*scan1++ = pix;
				*scan2++ = pix;
				*scan2++ = pix;
			}
		} }
		break;
	case 2:
		if( Pico.video.reg[12]&1 ) {
			for( j = 0; j < 240; j++ ) {
				scan1 = frame + j*800*2;
				scan2 = frame + j*800*2 + 800;
				for( i = 0; i < 320; i++ ) {
					register unsigned short pix = *buffer++;
					if( i & 0x01 ) {
						*scan1 = clerp16( *scan1, pix );
						*scan2 = clerp16( *scan2, pix );
					}
					*scan1++ = pix;
					*scan1++ = pix;
					*scan2++ = pix;
					*scan2++ = pix;
					if( ! (i & 0x01) ) {
						*scan1++ = pix;
						*scan2++ = pix;
					}
				}
			}
		} else {
			for( j = 0; j < 240; j++ ) {
				scan1 = frame + j*800*2 + 16;
				scan2 = frame + j*800*2 + 800 + 16;
				buffer = ((unsigned short*)md_screen) + j*320;
				for( i = 0; i < 256; i++ ) {
					register unsigned short pix = *buffer++;
					*scan1++ = pix;
					*scan1++ = pix;
					*scan1++ = pix;
					*scan2++ = pix;
					*scan2++ = pix;
					*scan2++ = pix;
				}
			}
		}
		break;
	}
	if( currentConfig.displayTS ) disp_ts_buttons(); //memcpy( frame, ts_key_layout, sizeof(ts_key_layout) );
	SDL_Flip(screen);
}

void maemo_memset_buffer(int offset, int byte, int len)
{
	memset( md_screen + offset, byte, len );
}


void maemo_cls(void)
{
	memset( md_screen, 0, 320*240*2 );
}


unsigned long maemo_joystick_read(int allow_usb_joy)
{
	return keystates | mousestates | accelstates;
}


int mp3_get_offset() {
	gint64 pos;
	GstFormat fmt = GST_FORMAT_TIME;
	if( !music_request.file ) {
		return 0;
	}
	GstState state;
	return 0;
}

void mp3_start_play( char* file, int seek ) {
	//printf( "START PLAY: %d %s\n", seek, file );
	if( seek < 0 ) {
		//if( music_request.file == file ) return;
		seek = 0;
	}
	FILE *f = fopen( "/home/user/.picodrive/picodrive.log", "a" );
	fprintf( f, "Loading offset: %d\n", seek );
	fclose(f);
	music_request.file = file;
	music_request.seek = (gint64)seek * (gint64)1000000;
	music_request.request = 1;
}

void maemo_start_sound(int rate, int bits, int stereo)
{
/*	SDL_AudioSpec as;
	memset( &as, 0, sizeof(as) );
	as.callback = audio_loop;
	as.channels = stereo ? 2 : 1;
	as.format = AUDIO_S16;
	as.freq = rate;
	as.samples = 0x200;

	memset( sound_buffer, 0, sizeof(sound_buffer) );
	soundptrw = soundptrr = 0;

	gst_init( 0, 0 );

	int err;
    pa_sample_spec ps;
    ps.channels = stereo ? 2 : 1;
    ps.format = PA_SAMPLE_S16LE;
    ps.rate = rate;
    audio = pa_simple_new( NULL, "pulse_s", PA_STREAM_PLAYBACK, NULL, "wavlaunch", &ps, NULL, NULL, &err );

    GError* error;
	GstElement* pipeline = gst_parse_launch( "filesrc name=mp3file ! mp3parse ! nokiamp3dec ! pulsesink", &error );
	if( !pipeline ) printf( "Error tope de gordo!!\n" );
	if( error ) {
		printf( "Error creating pipeline: %s\n", error->message );
	}
	GstElement* filesrc = gst_bin_get_by_name( GST_BIN(pipeline), "mp3file" );
	g_object_set( G_OBJECT(filesrc), "location", "/home/user/MyDocs/ROMS/megadrive/soniccd/Sonic CD 26.mp3", NULL );
	//GstBus* bus = gst_pipeline_get_bus( GST_PIPELINE(pipeline) );
	//gst_bus_add_watch( bus, bus_call, g_loop );
	//gst_object_unref( bus );

	printf( "About to play...\n" );
	gst_element_set_state( pipeline, GST_STATE_PLAYING );
	printf( "Play MP3 successful\n" );
*/
	//SDL_OpenAudio( &as, NULL );
	//SDL_PauseAudio(0);

	printf( "SOUND INIT CALLED\n" );
	//maemo_play_music( "/home/user/MyDocs/ROMS/megadrive/soniccd/Sonic CD 26.mp3" );
}


void maemo_sound_write(void *buff, int len)
{
	register int ptrr = soundptrr, ptrw = soundptrw;
	register int total = ptrr - ptrw - 1;
	if( total < 0 ) {
		total = min( SND_BUFFER_SIZE - ptrw, len );
		memcpy( sound_buffer + ptrw, buff, total );
		buff += total;
		len -= total;
		ptrw = ( ptrw + total ) % SND_BUFFER_SIZE;
	}
	total = min( len, total );
	memcpy( sound_buffer + ptrw, buff, total );
	soundptrw = ( ptrw + total ) % SND_BUFFER_SIZE;
}

void maemo_sound_sync(void)
{
	//ioctl(sounddev, SOUND_PCM_SYNC, 0);
}

void maemo_sound_volume(int l, int r)
{
 	/*l=l<0?0:l; l=l>255?255:l; r=r<0?0:r; r=r>255?255:r;
 	l<<=8; l|=r;
 	ioctl(mixerdev, SOUND_MIXER_WRITE_PCM, &l);*/ /*SOUND_MIXER_WRITE_VOLUME*/
}

/* common */
void maemo_init(void)
{
	printf("entering init()\n"); fflush(stdout);

	// Init Display
	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTTHREAD );
	SDL_WM_SetCaption( "PicoDrive for Maemo " G_STRINGIFY(GAME_VERSION), NULL );
	SDL_ShowCursor( SDL_DISABLE );
	SDL_WM_GrabInput( SDL_GRAB_OFF );

	screen = SDL_SetVideoMode( 800, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF );
	SDL_WM_ToggleFullScreen( screen );
	SDL_SetCursor( SDL_DISABLE );

	md_screen = malloc( 320*240*2 );

	// Init Audio
	gst_init( 0, 0 );
	SDL_AudioSpec as;
	memset( &as, 0, sizeof(as) );
	as.callback = audio_loop;
	as.channels = 2;
	as.format = AUDIO_S16;
	as.freq = 22050;
	as.samples = 0x200;

	memset( sound_buffer, 0, sizeof(sound_buffer) );
	soundptrw = soundptrr = 0;
	SDL_OpenAudio( &as, NULL );
	SDL_PauseAudio(0);


	// Init input
	keystates = mousestates = accelstates = 0;

	running = 1;
	pthread_create( &sdl_thread, NULL, SDL_Loop, NULL );

	printf("exiting init()\n"); fflush(stdout);
}

char *ext_menu = 0, *ext_state = 0;

void maemo_deinit(void)
{
	running = 0;
	pthread_join( sdl_thread, NULL );


	SDL_PauseAudio(1);
	SDL_CloseAudio();

	free( md_screen );

	SDL_FreeSurface( screen );
	SDL_Quit();

	printf("all done, quitting\n");
}

/* lprintf */
void lprintf(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
}

