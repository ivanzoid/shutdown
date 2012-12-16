#include <windows.h>
#include <stdio.h>
#include <string.h>

#define PRINT_NONE		0
#define PRINT_ERROR		1
#define PRINT_NORMAL	2
#define PRINT_VERBOSE	3

// global vars
int verbosity = PRINT_NORMAL;

void usage( void );
void message( int level, const char *msg );

int main( int argc, char *argv[] )
{
	HANDLE				hToken;
	TOKEN_PRIVILEGES	tp;
	UINT				uFlags = 0;
	bool				logoff = false;

	if ( argc < 2 ) {
		usage();
		return 0;
	}

	for ( int n = 1; n < argc; n++ ) {
		if ( argv[n][0] == '-' ) {
			for ( int i = 1; i < (int)strlen( argv[n] ); i++ ) {
				switch ( argv[n][i] ) {
				case 'p':
					uFlags = EWX_POWEROFF;
					break;
				case 's':
					uFlags = EWX_SHUTDOWN;
					break;
				case 'r':
					uFlags = EWX_REBOOT;
					break;
				case 'l':
					uFlags = EWX_LOGOFF;
					logoff = true;
					break;
				case 'f':
					uFlags |= EWX_FORCE;
					break;
				case 'v':
					verbosity = PRINT_VERBOSE;
					break;
				case 'S':
					verbosity = PRINT_NONE;
					break;
				case 'h':
				case '?':
					usage();
					break;
				default:
					printf( "ERROR: unknown option: %c\n", argv[n][i] );
					break;
				}
			}
		}
	}

	if ( uFlags == EWX_FORCE || (uFlags == 0 && logoff == false) )
	{
		printf( "You must specify at least on required option!\n" );
		return 1;
	}

	message( PRINT_NORMAL, "preparing to exit Windows...\n" );

	// Get a token for this process. 
	message( PRINT_VERBOSE, "getting token for this process\n" );
    if ( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) ) {
		message( PRINT_ERROR, "OpenProcessToken: couldn't get token for this proccess\n" );
		return 2;
	}

	// Get the LUID for the shutdown privilege.
	message( PRINT_VERBOSE, "getting LUID for the shutdown privilege\n" );
	if ( !LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid ) ) {
		message( PRINT_ERROR, "LookupPrivilegeValue: couldn't get LUID for shutdown privilege\n" );
		return 3;
	}

	tp.PrivilegeCount = 1;				// one privilege to set
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get the shutdown privilege for this process. 
	message( PRINT_VERBOSE, "getting the shutdown privilege for this process\n" );
	if ( AdjustTokenPrivileges( hToken, FALSE, &tp, 0, ( PTOKEN_PRIVILEGES )NULL, 0 ) == 0 ) {
		message( PRINT_ERROR, "AdjustTokenPrivileges: couldn't get the shutdown privilege for this process\n" );
		return 4;
	}

	message( PRINT_NORMAL, "exiting Windows...\n" );
	if ( !ExitWindowsEx( uFlags, 0 ) ) {
		message( PRINT_ERROR, "ExitWindowsEx: couldn't shudown system\n" );
		return 5;
	}
}

void message( int level, const char *msg )
{
	if ( level == PRINT_NONE )
		return;

	if ( level <= verbosity ) {	// message are the same or higher than verbosity level
		switch ( level ) {
		case PRINT_ERROR:
			printf( "error: " );
			break;
		case PRINT_VERBOSE:
			printf( "..." );
			break;
		}
		printf( msg );
	}
}

void usage( void )
{
	printf(
		"Shutdown by CHiEF v0.72.\n"
		"This is utility to shutdown your Windows system.\n"
		"USAGE:\tshutdown type [options]\n"
		"TYPE are:\n"
		"\t-p\tshutdown the system and turn off the power\n"
		"\t\tthe system must support the power-off feature\n"
		"\t-s\tshutdown the system and do not turn off the power\n"
		"\t-r\trestart the system\n"
		"\t-l\tlog off current user\n"
		"OPTIONS are:\n"
		"\t-f\tforced shutdown: terminate running proccesses without any questions\n"
		"\t-v\tverbose mode\n"
		"\t-S\tsilent mode: do not print anything, even if error occurs\n"
	);
}
