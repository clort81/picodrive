#include <unistd.h>
#include <glib.h>

int main( int argc, char*argv[] ) {
	execlp( G_STRINGIFY(OSSO_GAMES_STARTUP), G_STRINGIFY(OSSO_GAMES_STARTUP), G_STRINGIFY(GAME_CONF) );
}
