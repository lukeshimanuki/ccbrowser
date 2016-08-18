#ifndef __DEFS_H__
#define __DEFS_H__

#include <include/capi/cef_app_capi.h>
#include <include/capi/cef_client_capi.h>

typedef enum { false, true } bool;

typedef enum
{
	M_NONE  = 0,
	M_CTRL  = 1<<0,
	M_ALT   = 1<<1,
	M_SUPER = 1<<2,
} Mod;

typedef struct
{
	uint8_t mode;
	char* exe;
	char* cache_path;
} State;

typedef union
{
	const char* string;
	uint8_t mode;
	struct {int down; int right;} scroll;
	bool flag;
} Arg;


void shell      (cef_browser_t*, State*, Arg);
void set_mode   (cef_browser_t*, State*, Arg);
void command    (cef_browser_t*, State*, Arg);
void quit       (cef_browser_t*, State*, Arg);
void open_url   (cef_browser_t*, State*, Arg);
void scroll     (cef_browser_t*, State*, Arg);
void back       (cef_browser_t*, State*, Arg);
void forwards   (cef_browser_t*, State*, Arg);
void search     (cef_browser_t*, State*, Arg);
void reload     (cef_browser_t*, State*, Arg);
void stop       (cef_browser_t*, State*, Arg);
void javascript (cef_browser_t*, State*, Arg);
void link_hints (cef_browser_t*, State*, Arg);
void duplicate  (cef_browser_t*, State*, Arg);


typedef struct
{
	uint8_t mode;
	Mod mods;
	char key;
	void (*func)(cef_browser_t*, State*, Arg);
	Arg arg;
} Bind;

typedef struct
{
	char* string;
	void (*func)(cef_browser_t*, State*, Arg);
	Arg arg;
} Command;

#endif // __DEFS_H__

