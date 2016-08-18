#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include <include/capi/cef_app_capi.h>
#include <include/capi/cef_client_capi.h>

#include "defs.h"
#include "config.h"

State global_state =
{
	.mode = 0,
};

// app

void on_before_command_line_processing(cef_app_t*, const cef_string_t*, cef_command_line_t*);

cef_app_t app =
{
	.base = { .size = sizeof(cef_app_t) },
	.on_before_command_line_processing = on_before_command_line_processing,
};


// handlers

int on_file_dialog(cef_dialog_handler_t*, cef_browser_t*, cef_file_dialog_mode_t, const cef_string_t*, const cef_string_t*, cef_string_list_t, int, cef_file_dialog_callback_t*);
int on_pre_key_event(cef_keyboard_handler_t*, cef_browser_t*, const cef_key_event_t*, cef_event_handle_t, int*);
void on_before_close(cef_life_span_handler_t*, cef_browser_t*);
void on_before_download(cef_download_handler_t*, cef_browser_t*, cef_download_item_t*, const cef_string_t*, cef_before_download_callback_t*);
int on_request_geolocation_permission(cef_geolocation_handler_t*, cef_browser_t*, const cef_string_t*, int, cef_geolocation_callback_t*);
void on_load_start(cef_load_handler_t*, cef_browser_t*, cef_frame_t*);
void on_load_end(cef_load_handler_t*, cef_browser_t*, cef_frame_t*, int);
int on_jsdialog(cef_jsdialog_handler_t*, cef_browser_t*, const cef_string_t*, const cef_string_t*, cef_jsdialog_type_t, const cef_string_t*, const cef_string_t*, cef_jsdialog_callback_t*, int*);


cef_dialog_handler_t dialog_handler =
{
	.base = { .size = sizeof(cef_dialog_handler_t) },
	.on_file_dialog = on_file_dialog,
};

cef_download_handler_t download_handler =
{
	.base = { .size = sizeof(cef_download_handler_t) },
	.on_before_download = on_before_download,
};

cef_geolocation_handler_t geolocation_handler =
{
	.base = { .size = sizeof(cef_geolocation_handler_t) },
	.on_request_geolocation_permission = on_request_geolocation_permission,
};

cef_keyboard_handler_t keyboard_handler =
{
	.base = { .size = sizeof(cef_keyboard_handler_t) },
	.on_pre_key_event = on_pre_key_event,
};

cef_life_span_handler_t life_span_handler =
{
	.base = { .size = sizeof(cef_life_span_handler_t) },
	.on_before_close = on_before_close,
};

cef_load_handler_t load_handler =
{
	.base = { .size = sizeof(cef_load_handler_t) },
	.on_load_start = on_load_start,
	.on_load_end = on_load_end,
};

cef_jsdialog_handler_t jsdialog_handler =
{
	.base = { .size = sizeof(cef_jsdialog_handler_t) },
	.on_jsdialog = on_jsdialog,
};

cef_dialog_handler_t*      get_dialog_handler      (cef_client_t* self) { return &dialog_handler;      }
cef_download_handler_t*    get_download_handler    (cef_client_t* self) { return &download_handler;    }
cef_geolocation_handler_t* get_geolocation_handler (cef_client_t* self) { return &geolocation_handler; }
cef_keyboard_handler_t*    get_keyboard_handler    (cef_client_t* self) { return &keyboard_handler;    }
cef_life_span_handler_t*   get_life_span_handler   (cef_client_t* self) { return &life_span_handler;   }
cef_load_handler_t*        get_load_handler        (cef_client_t* self) { return &load_handler;        }
cef_jsdialog_handler_t*    get_jsdialog_handler    (cef_client_t* self) { return &jsdialog_handler;    }

cef_client_t client =
{
	.base = { .size = sizeof(cef_client_t) },
	.get_dialog_handler = get_dialog_handler,
	.get_download_handler = get_download_handler,
	.get_geolocation_handler = get_geolocation_handler,
	.get_keyboard_handler = get_keyboard_handler,
	.get_life_span_handler = get_life_span_handler,
	.get_load_handler = get_load_handler,
	.get_jsdialog_handler = get_jsdialog_handler,
};


// callback

void on_before_command_line_processing(cef_app_t* self, const cef_string_t* process_type, cef_command_line_t* command_line)
{
}

int on_file_dialog(cef_dialog_handler_t* self, cef_browser_t* browser, cef_file_dialog_mode_t mode, const cef_string_t* title, const cef_string_t* default_file_path, cef_string_list_t accept_filters, int selected_accept_filter, cef_file_dialog_callback_t* callback)
{
	callback->cont(callback, selected_accept_filter, NULL);

	return 1;
}

void on_before_download(cef_download_handler_t* self, cef_browser_t* browser, cef_download_item_t* download_item, const cef_string_t* suggested_name, cef_before_download_callback_t* callback)
{
}

int on_request_geolocation_permission(cef_geolocation_handler_t* self, cef_browser_t* browser, const cef_string_t* requesting_url, int request_id, cef_geolocation_callback_t* callback)
{
	return 0;
}

int on_pre_key_event(cef_keyboard_handler_t* self, cef_browser_t* browser, const cef_key_event_t* event, cef_event_handle_t os_event, int* is_keyboard_shortcut)
{
	State* state = &global_state;

	if (event->type == KEYEVENT_RAWKEYDOWN)
	{
		// handle keypress
		char key = event->unmodified_character;

		// check if key matches a bind
		for (size_t i = 0; i < sizeof(binds) / sizeof(Bind); i++)
		{
			Bind b = binds[i];
			if (b.mode == state->mode)
			{
				if (b.key == key)
				{
					b.func(browser, state, b.arg);
					return 1; // don't further process this key
				}
			}
		}
	}

	// default behavior
	return key_pass[state->mode] ? 0 : 1;
}

void on_before_close(cef_life_span_handler_t* self, cef_browser_t* browser)
{
	cef_quit_message_loop();
}

void on_load_start(cef_load_handler_t* self, cef_browser_t* browser, cef_frame_t* frame)
{
}

void on_load_end(cef_load_handler_t* self, cef_browser_t* browser, cef_frame_t* frame, int httpStatusCode)
{
	char js_str[1024];
	snprintf(js_str, sizeof(js_str), "document.body.style.background = '#000';");
	cef_string_t js = {};
	cef_string_utf8_to_utf16(js_str, strlen(js_str), &js);

//	frame->execute_java_script(frame, &js, frame->get_url(frame), 0);
}

int on_jsdialog(cef_jsdialog_handler_t* self, cef_browser_t* browser, const cef_string_t* origin_url, const cef_string_t* accept_lang, cef_jsdialog_type_t dialog_type, const cef_string_t* message_text, const cef_string_t* default_prompt_text, cef_jsdialog_callback_t* callback, int* suppress_message)
{
	printf("on_jsdialog: ");
	for (size_t i = 0; i < message_text->length; i++)
		printf("%c", message_text->str[i]);
	printf("\n");
	return 0;
}


// keybind functions
int spawn(const char* const *, FILE**, FILE**, FILE**);
size_t dmenu(char*, size_t, const char* const *, size_t);

void shell(cef_browser_t* browser, State* state, Arg arg)
{
	const char* argv[] =
	{
		shell_path,
		"-c",
		arg.string,
		NULL
	};
	spawn(argv, NULL, NULL, NULL);
}

void set_mode(cef_browser_t* browser, State* state, Arg arg)
{
	state->mode = arg.mode;
}

void command(cef_browser_t* browser, State* state, Arg arg)
{
	const size_t num_commands = sizeof(commands) / sizeof(Command);
	const char* items[num_commands];
	for (size_t i = 0; i < num_commands; i++)
		items[i] = commands[i].string;

	size_t sel = dmenu(NULL, 0, items, num_commands);

	commands[sel].func(browser, state, commands[sel].arg);
}

void quit(cef_browser_t* browser, State* state, Arg arg)
{
	cef_quit_message_loop();
}

void open_url(cef_browser_t* browser, State* state, Arg arg)
{
	char url_str[256];
	dmenu(url_str, sizeof(url_str), NULL, 0);
	cef_string_t url = {};
	cef_string_utf8_to_utf16(url_str, strlen(url_str), &url);

	cef_frame_t* frame = browser->get_main_frame(browser);
	frame->load_url(frame, &url);
}

void scroll(cef_browser_t* browser, State* state, Arg arg)
{
	char js_str[1024];
	snprintf(js_str, sizeof(js_str), "window.scrollBy(%i, %i);", arg.scroll.right, arg.scroll.down);
	cef_string_t js = {};
	cef_string_utf8_to_utf16(js_str, strlen(js_str), &js);

	cef_frame_t* frame = browser->get_focused_frame(browser);
	frame->execute_java_script(frame, &js, frame->get_url(frame), 0);
}

void back(cef_browser_t* browser, State* state, Arg arg)
{
	if (browser->can_go_back(browser))
		browser->go_back(browser);
}

void forwards(cef_browser_t* browser, State* state, Arg arg)
{
	if (browser->can_go_forward(browser))
		browser->go_forward(browser);
}

void encode_url(char* dest, char* str)
{
	size_t len_str = strlen(str);
	size_t idx = strlen(dest);
	for (size_t i = 0; i < len_str; i++)
	{
		char c = str[i];
		char special_chars[] = ";/?:@=& \"<>#%{}|\\^~[]`";
		int special = 0;
		if (c <= 0x1F || c >= 0x7F)
			special = 1;
		for (size_t j = 0; j < sizeof(special_chars) - 1; j++)
			if (c == special_chars[j])
				special = 1;
		if (special == 1)
		{
			sprintf(&dest[idx], "%%%02x", str[i]);
			idx += 3;
		}
		else dest[idx++] = str[i];
	}
	dest[idx++] = '\0';
}

void search(cef_browser_t* browser, State* state, Arg arg)
{
	char query[256];
	dmenu(query, sizeof(query), NULL, 0);

	char url_str[256];
	strcpy(url_str, arg.string);
	encode_url(&url_str[strlen(url_str)], query);
	cef_string_t url = {};
	cef_string_utf8_to_utf16(url_str, strlen(url_str), &url);

	cef_frame_t* frame = browser->get_main_frame(browser);
	frame->load_url(frame, &url);
}

void preference(cef_browser_t* browser, State* state, Arg arg)
{
	cef_browser_host_t* host = browser->get_host(browser);
	cef_request_context_t* context = host->get_request_context(host);

	const char* name_str = arg.preference.name;

	cef_string_t name = {};
	cef_string_utf8_to_utf16(name_str, strlen(name_str), &name);

	cef_value_t* value = cef_value_create();
	Value value_u = arg.preference.value;
	switch (arg.preference.vtype)
	{
		case 'b': value->set_bool(value, value_u.boolean); break;
		case 'i': value->set_int(value, value_u.integer); break;
		case 'f': value->set_double(value, value_u.float64); break;
		case 's':
		{
			cef_string_t str = {};
			cef_string_utf8_to_utf16(value_u.string, strlen(value_u.string), &str);
			value->set_string(value, &str);
			break;
		}
	}

	cef_string_t error = {};

	context->set_preference(context, &name, value, &error);
}

void reload(cef_browser_t* browser, State* state, Arg arg)
{
	browser->reload_ignore_cache(browser);
}

void stop(cef_browser_t* browser, State* state, Arg arg)
{
	browser->stop_load(browser);
}


int spawn(const char* const * argv, FILE** in, FILE** out, FILE** err)
{
	int pipe_in[2];
	int pipe_out[2];
	int pipe_err[2];
	pipe(pipe_in);
	pipe(pipe_out);
	pipe(pipe_err);

	pid_t pid = fork();
	if (pid < 0) return 1;
	else if (pid == 0)
	{
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);

		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_err[0]);

		execvp(argv[0], (char* const *)argv);

		exit(1);
	}
	else
	{
		if (in) *in = fdopen(pipe_in[1], "w");
		if (out) *out = fdopen(pipe_out[0], "r");
		if (err) *err = fdopen(pipe_err[0], "r");
		return 0;
	}
}

size_t dmenu(char* dest, size_t len_dest, const char* const * items, size_t num_items)
{
	char buffer[256];
	if (!dest)
	{
		dest = buffer;
		len_dest = sizeof(buffer);
	}

	const char* const argv[] = {shell_path, "-c", dmenu_path, NULL};

	FILE* in;
	FILE* out;

	if (spawn(argv, &in, &out, NULL) == 0)
	{
		for (size_t i = 0; i < num_items; i++)
			fprintf(in, "%s\n", items[i]);
		fclose(in);

		for (size_t i = 0; i < len_dest - 1; i++)
		{
			int c = fgetc(out);
			if (c == ' ' || c == '\n' || c == '\t' || c == EOF)
			{
				dest[i] = '\0';
				break;
			}
			dest[i] = c;
		}
		fclose(out);
		dest[len_dest - 1] = '\0';

		for (size_t i = 0; i < num_items; i++)
			if (strlen(dest) == strlen(items[i]) && strcmp(dest, items[i]) == 0)
				return i;
		return -1;
	}

	return -1;
}


int X_ErrorHandler(Display* display, XErrorEvent* event)
{
	fprintf(stderr, "X error: type %i, serial %lu, error_code %i, request_code %i, minor_code %i\n",
		event->type, event->serial, (int)event->error_code, (int)event->request_code, (int)event->minor_code
	);
	return 0;
}

int main(int argc, char** argv)
{
	cef_main_args_t args =
	{
		.argc = argc,
		.argv = argv,
	};

	int code = cef_execute_process(&args, &app, NULL);
	if (code >= 0) _exit(code);

	XSetErrorHandler(X_ErrorHandler);

	char cache_path_str[256] = "";
	if (argc >= 2) strncpy(cache_path_str, argv[1], sizeof(cache_path_str));
	cef_string_t cache_path = {};
	cef_string_utf8_to_utf16(cache_path_str, strlen(cache_path_str), &cache_path);

	cef_string_t log_file_cef = {};
	cef_string_utf8_to_utf16(log_file, strlen(log_file), &log_file_cef);

	cef_string_t user_agent_cef = {};
	cef_string_utf8_to_utf16(user_agent, strlen(user_agent), &user_agent_cef);

	cef_settings_t settings =
	{
		.size = sizeof(cef_settings_t),
		.command_line_args_disabled = STATE_ENABLED,
		.no_sandbox = STATE_ENABLED,
		.cache_path = cache_path,
		.log_file = log_file_cef,
		.log_severity = LOGSEVERITY_DEFAULT,
		.user_agent = user_agent_cef,
	};

	cef_initialize(&args, &settings, &app, NULL);

	cef_window_info_t windowInfo = {};

	char url[] = "https://www.google.com";
	cef_string_t cefUrl = {};
	cef_string_utf8_to_utf16(url, strlen(url), &cefUrl);
	
	cef_browser_settings_t browserSettings =
	{
		.size = sizeof(cef_browser_settings_t),
		.image_loading = enable_images ? STATE_ENABLED : STATE_DISABLED,
	};
	
	cef_browser_host_create_browser(&windowInfo, &client, &cefUrl, &browserSettings, NULL);

	cef_run_message_loop();

	cef_shutdown();

	return 0;
}

