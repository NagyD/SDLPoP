/*
 * Nintendo Switch entry point for SDLPoP.
 *
 * 1. If the app is not running as a full Application (for example in applet mode from the Album),
 *    show a message and exit with +.
 * 2. If the game data is missing from sdmc:/switch/SDLPoP/data/, list the missing files and exit with +.
 * 3. Otherwise enter sdmc:/switch/SDLPoP/ and start the game through sdlpop_main() (main.c).
 *
 * The game data files are not part of this repository: the user must copy them from a legally
 * obtained copy of the game (see README_SWITCH.md).
 */

#include <switch.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SDLPOP_ROOT "sdmc:/switch/SDLPoP/"

int sdlpop_main(int argc, char *argv[]);

/* Files required in data/ (relative to SDLPOP_ROOT). */
static const char *const required_files[] = {
	"data/PRINCE.DAT", "data/KID.DAT", "data/GUARD.DAT", "data/FAT.DAT",
	"data/SKEL.DAT", "data/SHADOW.DAT", "data/VIZIER.DAT", "data/PV.DAT",
	"data/VDUNGEON.DAT", "data/VPALACE.DAT", "data/TITLE.DAT",
	"data/light.png",
	NULL
};

/* Directories required in data/ (relative to SDLPOP_ROOT). */
static const char *const required_dirs[] = {
	"data/font",
	NULL
};

static bool path_exists(const char *relative_path, bool is_dir)
{
	struct stat st;
	char full_path[256];
	snprintf(full_path, sizeof(full_path), SDLPOP_ROOT "%s", relative_path);
	if (stat(full_path, &st) != 0)
		return false;
	return is_dir ? S_ISDIR(st.st_mode) : S_ISREG(st.st_mode);
}

/* Waits for the + button without closing the program by itself. */
static void wait_for_plus(void)
{
	PadState pad;
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	padInitializeDefault(&pad);
	while (appletMainLoop()) {
		padUpdate(&pad);
		if (padGetButtonsDown(&pad) & HidNpadButton_Plus)
			break;
		consoleUpdate(NULL);
	}
}

static void append_missing(char *list, size_t size, const char *name)
{
	size_t used = strlen(list);
	snprintf(list + used, size - used, "%s%s", used ? ", " : "", name);
}

static bool check_game_data(char *missing, size_t size)
{
	bool all_present = true;
	for (int i = 0; required_files[i] != NULL; i++) {
		if (!path_exists(required_files[i], false)) {
			append_missing(missing, size, required_files[i]);
			all_present = false;
		}
	}
	for (int i = 0; required_dirs[i] != NULL; i++) {
		if (!path_exists(required_dirs[i], true)) {
			append_missing(missing, size, required_dirs[i]);
			all_present = false;
		}
	}
	return all_present;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	/*
	 * Initialise the libnx console ONLY on the error paths. Calling consoleInit and consoleExit
	 * before sdlpop_main leaves stdout attached to a freed console, and the first printf of the
	 * game then crashes on Ryujinx. Keep the normal path free of console calls.
	 */
	if (appletGetAppletType() != AppletType_Application) {
		consoleInit(NULL);
		printf("SDLPoP must be started from hbmenu.\n\n");
		printf("In applet mode (Album) the memory is limited.\n");
		printf("Start it from hbmenu.\n\n");
		printf("Press + to exit.\n");
		wait_for_plus();
		consoleExit(NULL);
		return 0;
	}

	char missing[1024] = "";
	if (!check_game_data(missing, sizeof(missing))) {
		consoleInit(NULL);
		printf("Game data not found in /switch/SDLPoP/data/\n\n");
		printf("Copy the files from your own legal copy of the game,\n");
		printf("together with light.png and the font folder (see README_SWITCH.md).\n\n");
		printf("Missing: %s\n\n", missing);
		printf("Press + to exit.\n");
		wait_for_plus();
		consoleExit(NULL);
		return 1;
	}

	if (chdir(SDLPOP_ROOT) != 0) {
		consoleInit(NULL);
		printf("Cannot open /switch/SDLPoP/ on the microSD card.\n\n");
		printf("Press + to exit.\n");
		wait_for_plus();
		consoleExit(NULL);
		return 1;
	}

	char *args[] = { "sdlpop.nro", NULL };
	return sdlpop_main(1, args);
}
