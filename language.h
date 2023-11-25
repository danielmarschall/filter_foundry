/*
    This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
    Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
    Copyright (C) 2018-2022 Daniel Marschall, ViaThinkSoft

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef LANGUAGE_H_
#define LANGUAGE_H_

// Attention: Please save this file as UTF-8 without BOM!
// Also, if you extend this, please also extend language_win.rc and language_mac.r!
// The ID numbers must be continuous, otherwise the MAC stringlist (*r) does not work

// TODO: Name the constants MSGID_* and not MSG_*_ID

#define MSG_PREMIERE_COMPAT_ID 1
#define MSG_PREMIERE_COMPAT_ENUS "This version of Filter Foundry is not compatible with Adobe Premiere!";
#define MSG_PREMIERE_COMPAT_DEDE "Diese Version von Filter Foundry ist mit Adobe Premiere nicht kompatibel!"

#define MSG_RUNDLL_ERR_ID 2
#define MSG_RUNDLL_ERR_ENUS "You tried to execute this DLL with RunDLL32. This is a Photoshop plugin!"
#define MSG_RUNDLL_ERR_DEDE "Es wurde versucht, diese DLL mit RunDLL32 aufzurufen, aber dies ist ein Photoshop-Plugin!"

#define MSG_INCOMPATIBLE_OBFUSCATION_ID 3
#define MSG_INCOMPATIBLE_OBFUSCATION_ENUS "Incompatible obfuscation."
#define MSG_INCOMPATIBLE_OBFUSCATION_DEDE "Inkompatible Verschlüsselung."

#define MSG_FILTER_PROTECTED_ID 4
#define MSG_FILTER_PROTECTED_ENUS "The filter is protected."
#define MSG_FILTER_PROTECTED_DEDE "Der Filter ist geschützt."

#define MSG_LOADFILE_UNKNOWN_FORMAT_ID 5
#define MSG_LOADFILE_UNKNOWN_FORMAT_ENUS "It is not a text parameter file, nor a standalone Mac/PC filter created by Filter Factory/Filter Foundry."
#define MSG_LOADFILE_UNKNOWN_FORMAT_DEDE "Dies ist keine gültige Paramter-Datei, und auch kein Mac/PC Filter, der mit Filter Factory oder Filter Foundry erstellt wurde."

#define MSG_INVALID_PARAMETER_DATA_ID 6
#define MSG_INVALID_PARAMETER_DATA_ENUS "Invalid parameter data."
#define MSG_INVALID_PARAMETER_DATA_DEDE "Ungültige Parameter-Daten."

#define MSG_CANNOT_ZOOM_ID 7
#define MSG_CANNOT_ZOOM_ENUS "Could not build preview at chosen zoom level."
#define MSG_CANNOT_ZOOM_DEDE "Die Vorschau konnte nicht in dieser Vergrößerung erstellt werden."

#define MSG_CANNOT_ZOOM_MEMFULL_ID 8
#define MSG_CANNOT_ZOOM_MEMFULL_ENUS "Could not build preview at chosen zoom level. The image is too large for available memory. Try zooming in.\nIf that does not help, cancel and retry the filter."
#define MSG_CANNOT_ZOOM_MEMFULL_DEDE "Die Vorschau konnte nicht in dieser Vergrößerung erstellt werden, da das Bild zu groß für den verfügbaren Speicher ist. Versuchen Sie, die Vorschau zu vergrößern.\nWenn dies nicht hilft, brechen Sie den Filter ab und starten Sie ihn neu."

#define MSG_BUILT32_ID 9
#define MSG_BUILT32_ENUS "32 bit standalone filter was successfully created"
#define MSG_BUILT32_DEDE "32-Bit Filter wurde erfolgreich erzeugt"

#define MSG_BUILT64_ID 10
#define MSG_BUILT64_ENUS "64 bit standalone filter was successfully created"
#define MSG_BUILT64_DEDE "64-Bit Filter wurde erfolgreich erzeugt"

#define MSG_CANNOT_OPEN_FILE_ID 11
#define MSG_CANNOT_OPEN_FILE_ENUS "Could not open the file."
#define MSG_CANNOT_OPEN_FILE_DEDE "Kann Datei nicht öffnen."

#define MSG_CANNOT_CREATE_FILE_ID 12
#define MSG_CANNOT_CREATE_FILE_ENUS "Could not create the file."
#define MSG_CANNOT_CREATE_FILE_DEDE "Kann Datei nicht erstellen."

#define MSG_EXPRESSION1024_FOUND_ID 13
#define MSG_EXPRESSION1024_FOUND_ENUS "Found an expression longer than 1024 characters."
#define MSG_EXPRESSION1024_FOUND_DEDE "Es wurde eine Formel gefunden, die länger als 1024 Zeichen ist."

#define MSG_OUT_OF_MEMORY_ID 14
#define MSG_OUT_OF_MEMORY_ENUS "Not enough free memory to continue."
#define MSG_OUT_OF_MEMORY_DEDE "Nicht genügend Arbeitsspeicher zum Ausführen der Aktion."

#define MSG_FILTERS_UNLIMITED_WARNING_ID 15
#define MSG_FILTERS_UNLIMITED_WARNING_ENUS "Attention! You are loading a ""Filters Unlimited"" file. Please note that Filter Foundry only implements the basic Filter Factory functions. Therefore, most ""Filters Unlimited"" filters won't work with Filter Foundry."
#define MSG_FILTERS_UNLIMITED_WARNING_DEDE "Achtung! Sie laden ein Datei, die mit ""Filters Unlimited"" erstellt wurde. Bitte beachten Sie, dass Filter Foundry nur die grundlegenden Filter Factory-Funktionalitäten implementiert. Deswegen werden die meisten ""Filters Unlimited"" Filter nicht mit Filter Foundry funktionieren."

#define MSG_FORMULA_IR_1023_TRUNCATED_ID 16
#define MSG_FORMULA_IR_1023_TRUNCATED_ENUS "Attention! The formula for channel I/R was too long (longer than 1023 characters) and was truncated."
#define MSG_FORMULA_IR_1023_TRUNCATED_DEDE "Achtung! Die Formel für den Rot/Intro-Kanal ist zu lang (länger als 1023 Zeichen) und wurde deshalb abgeschnitten."

#define MSG_FORMULA_R_1023_TRUNCATED_ID 17
#define MSG_FORMULA_R_1023_TRUNCATED_ENUS "Attention! The formula for channel R was too long (longer than 1023 characters) and was truncated."
#define MSG_FORMULA_R_1023_TRUNCATED_DEDE "Achtung! Die Formel für den Rot-Kanal ist zu lang (länger als 1023 Zeichen) und wurde deshalb abgeschnitten."

#define MSG_FORMULA_G_1023_TRUNCATED_ID 18
#define MSG_FORMULA_G_1023_TRUNCATED_ENUS "Attention! The formula for channel G was too long (longer than 1023 characters) and was truncated."
#define MSG_FORMULA_G_1023_TRUNCATED_DEDE "Achtung! Die Formel für den Grün-Kanal ist zu lang (länger als 1023 Zeichen) und wurde deshalb abgeschnitten."

#define MSG_FORMULA_B_1023_TRUNCATED_ID 19
#define MSG_FORMULA_B_1023_TRUNCATED_ENUS "Attention! The formula for channel B was too long (longer than 1023 characters) and was truncated."
#define MSG_FORMULA_B_1023_TRUNCATED_DEDE "Achtung! Die Formel für den Blau-Kanal ist zu lang (länger als 1023 Zeichen) und wurde deshalb abgeschnitten."

#define MSG_FORMULA_A_1023_TRUNCATED_ID 20
#define MSG_FORMULA_A_1023_TRUNCATED_ENUS "Attention! The formula for channel A was too long (longer than 1023 characters) and was truncated."
#define MSG_FORMULA_A_1023_TRUNCATED_DEDE "Achtung! Die Formel für den Alpha-Kanal ist zu lang (länger als 1023 Zeichen) und wurde deshalb abgeschnitten."

#define MSG_CANNOT_CREATE_32BIT_FILTER_ID 21
#define MSG_CANNOT_CREATE_32BIT_FILTER_ENUS "Could not create 32 bit standalone filter"
#define MSG_CANNOT_CREATE_32BIT_FILTER_DEDE "Konnte keinen 32-Bit Filter erzeugen"

#define MSG_CANNOT_CREATE_64BIT_FILTER_ID 22
#define MSG_CANNOT_CREATE_64BIT_FILTER_ENUS "Could not create 64 bit standalone filter"
#define MSG_CANNOT_CREATE_64BIT_FILTER_DEDE "Konnte keinen 64-Bit Filter erzeugen"

#define MSG_CANNOT_SAVE_SETTINGS_ID 23
#define MSG_CANNOT_SAVE_SETTINGS_ENUS "Could not save settings."
#define MSG_CANNOT_SAVE_SETTINGS_DEDE "Einstellungen konnten nicht gespeichert werden."

#define MSG_CANNOT_LOAD_SETTINGS_ID 24
#define MSG_CANNOT_LOAD_SETTINGS_ENUS "Could not load settings."
#define MSG_CANNOT_LOAD_SETTINGS_DEDE "Einstellungen konnten nicht geladen werden."

#define MSG_SAVED_EXPR_ERR_ID 25
#define MSG_SAVED_EXPR_ERR_ENUS "Can't run this filter (there is a problem with the saved expressions)."
#define MSG_SAVED_EXPR_ERR_DEDE "Dieser Filter kann nicht ausgeführt werden, da es ein Problem mit den enthaltenen Formeln gibt."

#define MSG_CATEGORY_EMPTY_ERR_ID 26
#define MSG_CATEGORY_EMPTY_ERR_ENUS "Category must not be empty!"
#define MSG_CATEGORY_EMPTY_ERR_DEDE "Kategorie darf nicht leer sein!"

#define MSG_TITLE_EMPTY_ERR_ID 27
#define MSG_TITLE_EMPTY_ERR_ENUS "Title must not be empty!"
#define MSG_TITLE_EMPTY_ERR_DEDE "Titel darf nicht leer sein!"

#define MSG_UNICODE_DATA_WARNING_ID 28
#define MSG_UNICODE_DATA_WARNING_ENUS "The internal structures of Photoshop and Filter Factory are not compatible with Unicode characters. It is highly recommended that you only use characters of your current charset. Unicode characters will be converted into question mark symbols."
#define MSG_UNICODE_DATA_WARNING_DEDE "Die internen Strukturen von Photoshop und Filter Factory sind nicht kompatibel mit Unicode-Zeichen. Es wird sehr stark empfohlen, dass Sie nur Zeichen aus Ihrem aktuellen Zeichensatz verwenden. Unicode-Zeichen werden in Fragezeichen-Symbole umgewandelt werden."

#define MSG_EXTCHARSET_DATA_WARNING_ID 29
#define MSG_EXTCHARSET_DATA_WARNING_ENUS "You were using characters of an extended charset. The characters might look correct on your machine, but on a machine in a different country the characters might look wrong. Please consider using the ASCII character set only (i.e. Latin characters without accent marks)."
#define MSG_EXTCHARSET_DATA_WARNING_DEDE "Sie verwenden besondere Zeichen wie z.B. Umlaute. Diese Zeichen werden auf Ihrem System normal aussehen, jedoch werden sie auf einem fremdsprachigen Computer falsch dargestellt werden. Bitte ziehen Sie in Betracht, nur ASCII-Zeichen zu verwenden (A-Z, ohne Umlaute)."

#define MSG_SLIDER_UNAVAILABLE_ID 30
#define MSG_SLIDER_UNAVAILABLE_ENUS "Visual sliders are not available because neither PLUGIN.DLL (part of Photoshop), nor the Microsoft Trackbar Control (msctls_trackbar32) was found on your system."
#define MSG_SLIDER_UNAVAILABLE_DEDE "Schiebe-Regler sind nicht verfügbar, da weder PLUGIN.DLL (Bestandteil von Photoshop), noch die Microsoft Trackbar Control (msctls_trackbar32) auf Ihrem System gefunden wurden."

#define MSG_PICO_SAVED_ID 31
#define MSG_PICO_SAVED_ENUS "The file was successfully saved in the ""PluginCommander"" TXT format. The file will now be opened in a text editor, so that you can fill in the missing data: Category, Title, Copyright, Author, Filename, and Slider/Map names."
#define MSG_PICO_SAVED_DEDE "Die Datei wurde erfolgreich im ""PluginCommander"" TXT-Format gespeichert. Die Datei wird nun in einem Text-Editor geöffnet, sodass Sie die fehlenden Angaben ergänzen können: Kategorie, Titel, Copyright, Autor, Dateiname und Regler-Namen."

#define MSG_PLEASE_EDIT_MANUALLY_ID 32
#define MSG_PLEASE_EDIT_MANUALLY_ENUS "Please edit the file manually to enter the title, category, author, copyright, slider names, etc."
#define MSG_PLEASE_EDIT_MANUALLY_DEDE "Bitte bearbeiten Sie die Datei manuell und tragen Sie dort ein: Titel, Kategorie, Autor, Copyright, Regler-Namen, usw."

#define MSG_FIND_DOKU_HERE_ID 33
#define MSG_FIND_DOKU_HERE_ENUS "You can find the documentation here: https://github.com/danielmarschall/filter_foundry/tree/master/doc"
#define MSG_FIND_DOKU_HERE_DEDE "Sie können die englisch-sprachige Dokumentation hier finden: https://github.com/danielmarschall/filter_foundry/tree/master/doc"

#define MSG_ERROR_IN_EXPRESSION_CLICK_DETAILS_ID 34
#define MSG_ERROR_IN_EXPRESSION_CLICK_DETAILS_ENUS "Error in expression! Click to see details."
#define MSG_ERROR_IN_EXPRESSION_CLICK_DETAILS_DEDE "Fehler in der Formel! Klicken für mehr Details."

#define MSG_ZOOM_IN_ID 35
#define MSG_ZOOM_IN_ENUS "Zoom in"
#define MSG_ZOOM_IN_DEDE "Vergrößern"

#define MSG_ZOOM_OUT_ID 36
#define MSG_ZOOM_OUT_ENUS "Zoom out"
#define MSG_ZOOM_OUT_DEDE "Verkleinern"

#define MSG_FULLY_ZOOM_INOUT_ID 37
#define MSG_FULLY_ZOOM_INOUT_ENUS "Fully zoom in/out"
#define MSG_FULLY_ZOOM_INOUT_DEDE "Vollständig rein/raus-zoomen"

#define MSG_LOAD_FILTER_SETTINGS_TITLE_ID 38
#define MSG_LOAD_FILTER_SETTINGS_TITLE_ENUS "Load filter settings"
#define MSG_LOAD_FILTER_SETTINGS_TITLE_DEDE "Filter-Einstellungen laden"

#define MSG_SAVE_FILTER_SETTINGS_TITLE_ID 39
#define MSG_SAVE_FILTER_SETTINGS_TITLE_ENUS "Save filter settings"
#define MSG_SAVE_FILTER_SETTINGS_TITLE_DEDE "Filter-Einstellungen speichern"

#define MSG_MAKE_FILTER_SETTINGS_TITLE_ID 40
#define MSG_MAKE_FILTER_SETTINGS_TITLE_ENUS "Create standalone filter"
#define MSG_MAKE_FILTER_SETTINGS_TITLE_DEDE "Eigenständigen Filter erzeugen"

#define MSG_ALL_SUPPORTED_FILES_ID 41
#define MSG_ALL_SUPPORTED_FILES_ENUS "All supported files"
#define MSG_ALL_SUPPORTED_FILES_DEDE "Alle unterstützten Dateien"

#define MSG_ALL_FILES_ID 42
#define MSG_ALL_FILES_ENUS "All files"
#define MSG_ALL_FILES_DEDE "Alle Dateien"

#define MSG_OPEN_AFS_ID 43
#define MSG_OPEN_AFS_ENUS "Filter Factory Settings"
#define MSG_OPEN_AFS_DEDE "Filter Factory Einstellungen"

#define MSG_OPEN_TXT_ID 44
#define MSG_OPEN_TXT_ENUS "PluginCommander or FFDecomp TXT file"
#define MSG_OPEN_TXT_DEDE "PluginCommander oder FFDecomp Text-Datei"

#define MSG_OPEN_8BF_ID 45
#define MSG_OPEN_8BF_ENUS "Filter Factory for Windows, Standalone Filter"
#define MSG_OPEN_8BF_DEDE "Filter Factory für Windows, Eigenständiger Filter"

#define MSG_OPEN_PFF_ID 46
#define MSG_OPEN_PFF_ENUS "Premiere TF / FF Settings"
#define MSG_OPEN_PFF_DEDE "Premiere TF / FF Einstellungen"

#define MSG_OPEN_PRM_ID 47
#define MSG_OPEN_PRM_ENUS "Premiere TF/FF for Windows, Standalone Filter"
#define MSG_OPEN_PRM_DEDE "Premiere TF/FF für Windows, Eigenständiger Filter"

#define MSG_OPEN_RSRC_ID 48
#define MSG_OPEN_RSRC_ENUS "FilterFactory for MacOS, Standalone Filter"
#define MSG_OPEN_RSRC_DEDE "FilterFactory für MacOS, Eigenständiger Filter"

#define MSG_OPEN_FFX_ID 49
#define MSG_OPEN_FFX_ENUS """Filters Unlimited"" filter"
#define MSG_OPEN_FFX_DEDE """Filters Unlimited"" Filter"

#define MSG_SAVE_AFS_ID 50
#define MSG_SAVE_AFS_ENUS "Filter Factory Settings"
#define MSG_SAVE_AFS_DEDE "Filter Factory Einstellungen"

#define MSG_SAVE_PFF_ID 51
#define MSG_SAVE_PFF_ENUS "Premiere TF / FF Settings"
#define MSG_SAVE_PFF_DEDE "Premiere TF / FF Einstellungen"

#define MSG_SAVE_TXT_ID 52
#define MSG_SAVE_TXT_ENUS "PluginCommander TXT file"
#define MSG_SAVE_TXT_DEDE "PluginCommander Text-Datei"

#define MSG_MAKE_8BF_ID 53
#define MSG_MAKE_8BF_ENUS "Filter plugin file"
#define MSG_MAKE_8BF_DEDE "Eigenständiger Filter"

#define MSG_ABOUT_BY_ID 54
#define MSG_ABOUT_BY_ENUS " by "
#define MSG_ABOUT_BY_DEDE " von "

#define MSG_ABOUT_BUILT_USING_ID 55
#define MSG_ABOUT_BUILT_USING_ENUS "This plugin was built using Filter Foundry"
#define MSG_ABOUT_BUILT_USING_DEDE "Dieses Plugin wurde erstellt mit Filter Foundry"

#define MSG_ABOUT_LATEST_VERSION_FROM_ID 56
#define MSG_ABOUT_LATEST_VERSION_FROM_ENUS "Latest version available from:"
#define MSG_ABOUT_LATEST_VERSION_FROM_DEDE "Aktuelle Version hier herunterladen:"

#define MSG_ABOUT_CONTACT_AUTHOR_ID 57
#define MSG_ABOUT_CONTACT_AUTHOR_ENUS "Please contact the author with any bug reports,\nsuggestions or comments.\nIf you use this program and like it, please consider\nmaking a donation."
#define MSG_ABOUT_CONTACT_AUTHOR_DEDE "Bitte kontaktieren Sie den Autor für Fehlerberichte,\nAnregungen und Kommentare.\nWenn Sie dieses Programm mögen,\nerwägen Sie bitte eine kleine Spende."

#define MSG_OPEN_GUF_ID 58
#define MSG_OPEN_GUF_ENUS """GIMP UserFilter"" filter"
#define MSG_OPEN_GUF_DEDE """GIMP UserFilter"" Filter"

#define MSG_SAVE_GUF_ID 59
#define MSG_SAVE_GUF_ENUS """GIMP UserFilter"" filter"
#define MSG_SAVE_GUF_DEDE """GIMP UserFilter"" Filter"

#define MSG_INCOMPATIBLE_GUF_FILE_ID 60
#define MSG_INCOMPATIBLE_GUF_FILE_ENUS "Incompatible GIMP UserFilter File"
#define MSG_INCOMPATIBLE_GUF_FILE_DEDE "Inkompatible GIMP UserFilter Datei"

#define MSG_OPEN_FFL_ID 61
#define MSG_OPEN_FFL_ENUS "Filter Library"
#define MSG_OPEN_FFL_DEDE "Filter Bibliothek"

#define MSG_FFL_CONVERTED_ID 62
#define MSG_FFL_CONVERTED_ENUS "The FFL file was split to single TXT files. You can now open these TXT files."
#define MSG_FFL_CONVERTED_DEDE "Die FFL Datei wurde in einzelne TXT Dateien aufgesplitet. Sie können diese TXT Dateien nun öffnen."

#define MSG_INVALID_FILE_SIGNATURE_ID 63
#define MSG_INVALID_FILE_SIGNATURE_ENUS "Invalid file signature"
#define MSG_INVALID_FILE_SIGNATURE_DEDE "Ungültiger Dateianfang"

#define MSG_FFL_NO_FILTERS_DETECTED_ID 64
#define MSG_FFL_NO_FILTERS_DETECTED_DEDE "In this FFL file there are no filters"
#define MSG_FFL_NO_FILTERS_DETECTED_ENUS "In dieser FFL Datei befinden sich keine Filter"

#define MSG_ERROR_GENERATING_DATA_ID 65
#define MSG_ERROR_GENERATING_DATA_ENUS "Error generating the file contents"
#define MSG_ERROR_GENERATING_DATA_DEDE "Fehler beim Erstellen der Datei-Inhalte"

#define MSG_UNSUPPORTED_FILE_FORMAT_ID 66
#define MSG_UNSUPPORTED_FILE_FORMAT_DEDE "Not supported file format. Please take care to choose a valid filename extension"
#define MSG_UNSUPPORTED_FILE_FORMAT_ENUS "Nicht unterstütztes Dateiformat. Bitte achten Sie auf die korrekte Dateinamens-Erweiterung"

void strcpy_advance_id(TCHAR** str, int msgid);
int FF_GetMsg(TCHAR* ret, int MsgId);
TCHAR* FF_GetMsg_Cpy(int MsgId);
void FF_GetMsg_Free(TCHAR* str);

#endif

