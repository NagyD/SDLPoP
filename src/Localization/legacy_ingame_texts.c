/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2023  Dávid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/

#include <stdio.h>

#include "legacy_ingame_texts.h"

// Note : bottom text box cannot display more than 28 chars

void str_one_second_left(enum localization loc, char* string, size_t string_size) {
	switch (loc) {
	case FR:
		strncpy(string, "DERNIERE SECONDE", string_size); break;
	case DE:
		strncpy(string, "1 SEKUNDE UEBRIG", string_size); break;
	case EN:
	default:
		strncpy(string, "1 SECOND LEFT", string_size); break;
	}
}

void str_seconds_left(enum localization loc, char* string, size_t string_size, word rem_sec) {
	switch (loc) {
	case FR:
		snprintf(string, string_size, "ENCORE %d SECONDES", rem_sec); break;
	case DE:
		snprintf(string, string_size, "%d SEKUNDEN UEBRIG", rem_sec); break;
	case EN:
	default:
		snprintf(string, string_size, "%d SECONDS LEFT", rem_sec); break;
	}
}

void str_minutes_left(enum localization loc, char* string, size_t string_size, word rem_min) {
	switch (loc) {
	case FR:
		snprintf(string, string_size, "ENCORE %d MINUTES", rem_min); break;
	case DE:
		snprintf(string, string_size, "%d MINUTEN UEBRIG", rem_min); break;
	case EN:
	default:
		snprintf(string, string_size, "%d MINUTES LEFT", rem_min); break;
	}
}

void str_time_expired(enum localization loc, char* string, size_t string_size) {
	switch (loc) {
	case FR:
		strncpy(string, "TEMPS ECOULE!", string_size); break;
	case DE:
		strncpy(string, "DIE ZEIT IST ABGELAUFEN!", string_size); break;
	case EN:
	default:
		strncpy(string, "TIME HAS EXPIRED!", string_size); break;
	}
}

void str_level(enum localization loc, char* string, size_t string_size, byte disp_level) {
	switch (loc) {
	case FR:
		snprintf(string, string_size, "NIVEAU %d", disp_level); break;
	case DE:
		snprintf(string, string_size, "LEVEL %d", disp_level); break;
	case EN:
	default:
		snprintf(string, string_size, "LEVEL %d", disp_level); break;
	}
}

void str_pressbutton(enum localization loc, char* string, size_t string_size) {
	switch (loc) {
	case FR:
		strncpy(string, "PRESSEZ LE BOUTON", string_size); break;
	case DE:
		strncpy(string, "KNOPFDRUCK ZUM WEITERSPIELEN", string_size); break;
	case EN:
	default:
		strncpy(string, "Press Button to Continue", string_size); break;
	}
}

void str_pause(enum localization loc, char* string, size_t string_size) {
	switch (loc) {
	case FR:
		strncpy(string, "PAUSE", string_size); break;
	case DE:
		strncpy(string, "PAUSE", string_size); break;
	case EN:
	default:
		strncpy(string, "GAME PAUSED", string_size); break;
	}
}

void str_save(enum localization loc, char* string, size_t string_size) {
	switch (loc) {
	case FR:
		strncpy(string, "PARTIE SAUVEE", string_size); break;
	case DE:
		strncpy(string, "SPIEL WURDE GESPEICHERT", string_size); break;
	case EN:
	default:
		strncpy(string, "GAME SAVED", string_size); break;
	}
}

void str_unable_save(enum localization loc, char* string, size_t string_size) {
	switch (loc) {
	case FR:
		strncpy(string, "SAUVEGARDE IMPOSSIBLE", string_size); break;
	case DE:
		strncpy(string, "SPIEL NICHT GESPEICHERT", string_size); break;
	case EN:
	default:
		strncpy(string, "UNABLE TO SAVE GAME", string_size); break;
	}
}

void str_copy_protection_bottom(enum localization loc, char* string, size_t string_size,
								word copy_word, word copy_line, word copy_page) {
	switch (loc) {
	case FR:
		snprintf(string, string_size, "PAGE %d LIGNE %d MOT %d", copy_page, copy_line, copy_word); break;
	case DE:
		snprintf(string, string_size, "SEITE %d ZEILE %d WORT %d", copy_page, copy_line, copy_word); break;
	case EN:
	default:
		snprintf(string, string_size, "WORD %d LINE %d PAGE %d", copy_word, copy_line, copy_page);
	}
}

void str_copy_protection_dialog(enum localization loc, char* string, size_t string_size,
									   word copy_word, word copy_line, word copy_page) {
	switch (loc) {
	case FR:
		snprintf(string, string_size,
				 "Buvez la potion correspondant à la première "
				 "lettre du Mot %d Ligne %d de la Page %d du Manuel.",
				 copy_word, copy_line, copy_page);
		break;
	case DE:
		snprintf(string, string_size,
				 "Trinken Sie die magische Flasche mit dem Anfangsbuchstaben von\n"
				 "Wort %d in Zeile %d auf Seite %d.",
				 copy_word, copy_line, copy_page);
		break;
	case EN:
	default:
		snprintf(string, string_size,
				 "Drink potion matching the first letter of Word %d on Line %d\n"
				 "of Page %d of the manual.",
				 copy_word, copy_line, copy_page);
	}
}

void str_dialog(enum localization loc, char* string, size_t string_size, const char* text) {
	switch (loc) {
	case FR:
		snprintf(string, string_size, "%s\n\nPressez une touche pour continuer.", text);
		break;
	case DE:
		snprintf(string, string_size, "%s\n\nDrücken Sie eine Taste.", text); break;
	case EN:
	default:
		snprintf(string, string_size, "%s\n\nPress any key to continue.", text);
	}
}

void str_loading(enum localization loc, char* string, size_t string_size) {
	switch (loc) {
	case FR:
		strncpy(string, "Chargement. . .", string_size); break;
	case DE:
		strncpy(string, "Bitte Warten. . .", string_size); break;
	case EN:
	default:
		strncpy(string, "Loading. . . .", string_size); break;
	}
}
