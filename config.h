#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "defs.h"

const char user_agent[] = "";

const char log_file[] = "/dev/null";

const char shell_path[] = "sh";
const char dmenu_path[] = "dmenu";

const int enable_images = 1;

// modes
enum
{
	NORMAL = 0,
	INSERT = 1,
};

Bind binds[] =
{
	// mode   mods    keys    function  args

	// normal mode
	{ NORMAL, M_NONE, 'i',    set_mode, { .mode = INSERT } },

	{ NORMAL, M_NONE, ':',    command,  { } },

	{ NORMAL, M_NONE, 'h',    scroll,   { .scroll = { 0, -20 } } },
	{ NORMAL, M_NONE, 'j',    scroll,   { .scroll = { +20, 0 } } },
	{ NORMAL, M_NONE, 'k',    scroll,   { .scroll = { -20, 0 } } },
	{ NORMAL, M_NONE, 'l',    scroll,   { .scroll = { 0, +20 } } },

	{ NORMAL, M_NONE, 'q',    quit,     { } },

	{ NORMAL, M_NONE, 'r',    reload,   { } },
	{ NORMAL, M_NONE, 'x',    stop,     { } },
	{ NORMAL, M_NONE, 'o',    open_url, { } },

	{ NORMAL, M_NONE, 'b',    back,     { } },
	{ NORMAL, M_NONE, 'f',    forwards, { } },

	// insert mode
	{ INSERT, M_NONE, 0x1B,   set_mode, { .mode = NORMAL } },
};

Command commands[] =
{
	// string  function    args
	{  "ddg",  search,     { .string = "duckduckgo.com/?q="       } },
	{  "gg",   search,     { .string = "www.google.com/search?q=" } },

	{  "ie",   preference, { .preference = {"profile.default_content_setting_values.images", {.integer = 0 }, 'i'} } },
	{  "id",   preference, { .preference = {"profile.default_content_setting_values.images", {.integer = 2 }, 'i'} } },
};

// if unbound keypresses should be sent to the renderer
bool key_pass[] = { false, true };

#endif // __CONFIG_H__

