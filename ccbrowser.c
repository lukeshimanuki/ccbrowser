#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <X11/Xlib.h>

#include <include/capi/cef_app_capi.h>
#include <include/capi/cef_client_capi.h>

#include "defs.h"
#include "config.h"

State global_state =
{
	.mode = 0,
};

FILE* history_cache = NULL;

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
	cef_string_t* url = frame->get_url(frame);
	size_t beg = strnlen(history, sizeof(history));
	size_t i = 0;
	while (i < url->length && beg + i < sizeof(history) - 2)
	{
		history[beg + i] = url->str[i];
		fprintf(history_cache, "%c", url->str[i]);
		i++;
	}
	history[beg + i] = '\n';
	history[beg + i + 1] = '\0';
	fprintf(history_cache, "\n");
	fflush(history_cache);
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
size_t dmenu(char*, size_t, const char* const *, size_t, const char*, int);
void run_js(cef_frame_t*, const char*);

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

	size_t sel = dmenu(NULL, 0, items, num_commands, "", 0);

	commands[sel].func(browser, state, commands[sel].arg);
}

void quit(cef_browser_t* browser, State* state, Arg arg)
{
	cef_quit_message_loop();
}

void open_url(cef_browser_t* browser, State* state, Arg arg)
{
	char url_str[1024];
	dmenu(url_str, sizeof(url_str), NULL, 0, arg.string, 20);

	if (strnlen(url_str, sizeof(url_str)) > 0)
	{
		cef_string_t url = {};
		cef_string_utf8_to_utf16(url_str, strlen(url_str), &url);

		cef_frame_t* frame = browser->get_main_frame(browser);
		frame->load_url(frame, &url);
	}
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

int fork_window(State* state, char* cache_path, char* url)
{
	pid_t pid = fork();
	if (pid < 0) return 1;
	else if (pid == 0)
	{
		setsid();
		signal(SIGHUP, SIG_IGN);

		const char* argv[] =
		{
			state->exe,
			cache_path,
			url,
			NULL
		};

		execvp(argv[0], (char* const *)argv);

		exit(1);
	}
	else return 0;
}

void duplicate(cef_browser_t* browser, State* state, Arg arg)
{
	cef_frame_t* frame = browser->get_main_frame(browser);
	cef_string_t* url_cef = frame->get_url(frame);
	char url[1024];
	for (size_t i = 0; i < url_cef->length + 1; i++)
		url[i] = url_cef->str[i];
	fork_window(state, state->cache_path, url);
}

void javascript(cef_browser_t* browser, State* state, Arg arg)
{
	run_js(browser->get_main_frame(browser), arg.string);
}

void stylesheet(cef_browser_t* browser, State* state, Arg arg)
{
	char js[4096];
	snprintf(js, sizeof(js), "if(typeof __custom_stylesheet__!=='undefined'){document.head.removeChild(__custom_stylesheet__);}__custom_stylesheet__=document.createElement('style');__custom_stylesheet__.innerHTML='%s';document.head.appendChild(__custom_stylesheet__);", arg.string);
	run_js(browser->get_main_frame(browser), js);
}

void pop_style(cef_browser_t* browser, State* state, Arg arg)
{
	char js[256];
	snprintf(js, sizeof(js), "if(typeof __custom_stylesheet__!=='undefined'){document.head.removeChild(__custom_stylesheet__);delete __custom_stylesheet__}");
	run_js(browser->get_main_frame(browser), js);
}

void show_hints(cef_browser_t* browser, State* state, Arg arg)
{
	char js[4096];

	cef_frame_t* frame = browser->get_main_frame(browser);

	snprintf(js, sizeof(js),
		R"JS(div = document.body.querySelector('div[id=__ccbrowser_link_hints__]');)JS"
		R"JS(if(div!=undefined){document.body.removeChild(div);})JS"
		R"JS(chars = '%s';)JS"
		R"JS(function idx2str(idx){return idx==0?'':chars.charAt(idx%%chars.length)+idx2str(idx/chars.length|0);})JS"
		R"JS(elems_list = [];)JS"
		R"JS(wt = window.scrollY;)JS"
		R"JS(wb = wt + window.innerHeight;)JS"
		R"JS(wl = window.scrollX;)JS"
		R"JS(wr = wl + window.innerWidth;)JS"
		R"JS(elems = document.body.querySelectorAll('a, input:not([type=hidden]), textarea, select, button');)JS"
		R"JS(div = document.createElement('div');)JS"
		R"JS(div.setAttribute('id', '__ccbrowser_link_hints__');)JS"
		R"JS(document.body.appendChild(div);)JS"
		R"JS(for (i = 0; i < elems.length; i++) {)JS"
		R"JS(	elem = elems[i];)JS"
		R"JS(	pos = elem.getBoundingClientRect();)JS"
		R"JS(	if (pos.height==0 || pos.width==0))JS"
		R"JS(		continue;)JS"
		R"JS(	et = wt + pos.top;)JS"
		R"JS(	eb = wt + pos.bottom;)JS"
		R"JS(	el = wl + pos.left;)JS"
		R"JS(	er = wl + pos.left;)JS" 
		R"JS(	if (eb >= wt && et <= wb) {)JS"
		R"JS(		elems_list.push(elem);)JS"
		R"JS(		span = document.createElement('span');)JS"
		R"JS(		span.style.cssText = [)JS"
		R"JS(			'color: white;',)JS"
		R"JS(			'background-color: green;',)JS"
		R"JS(			'font-size: 12px;',)JS"
		R"JS(			'position: absolute;',)JS"
		R"JS(			'left: ', el, 'px;',)JS"
		R"JS(			'top: ', et, 'px;',)JS"
		R"JS(			'padding: 1px 2px;',)JS"
		R"JS(		].join('');)JS"
		R"JS(		span.innerHTML = idx2str(elems_list.length);)JS"
		R"JS(		div.appendChild(span);)JS"
		R"JS(	})JS"
		R"JS(})JS"
		, arg.string
	);
	run_js(frame, js);
}

void hide_hints(cef_browser_t* browser, State* state, Arg arg)
{
	char js[4096];

	cef_frame_t* frame = browser->get_main_frame(browser);

	snprintf(js, sizeof(js),
		R"JS(div = document.body.querySelector('div[id=__ccbrowser_link_hints__]');)JS"
		R"JS(if(div!=undefined){document.body.removeChild(div);})JS"
	);
	run_js(frame, js);
}

int str2int(const char* digits, const char* str)
{
	char c = *str;
	if (c == '\0') return 0;
	int recurse = str2int(digits, str + 1);
	if (recurse == -1) return -1;
	int base = strlen(digits);
	for (int i = 0; i < base; i++)
		if (c == digits[i])
			return recurse * base + i;
	return -1;
}

void select_hint(cef_browser_t* browser, State* state, Arg arg)
{
	char js[4096];

	cef_frame_t* frame = browser->get_main_frame(browser);

	char selection[256];
	dmenu(selection, sizeof(selection), NULL, 0, "", 0);
	size_t len = strlen(selection);
	for (size_t i = 0; i < len; i++)
	{
		char c = selection[i];
		if ((c < 'a' || c > 'z') && (c < '0' || c > '9')) // only allow alphanumeric
			selection[i] = ' ';
	}

	snprintf(js, sizeof(js),
		R"JS(idx = %i;)JS"
		R"JS(if (idx > 0 && idx <= elems_list.length) {)JS"
		R"JS(	elem = elems_list[idx - 1];)JS"
		R"JS(	if (elem != undefined) {)JS"
		R"JS(		tag = elem.tagName.toLowerCase();)JS"
		R"JS(		type = elem.type ? elem.type.toLowerCase() : '';)JS"
		R"JS(		if (tag == 'a' && elem.href != ''))JS"
		R"JS(			location.href=elem.href;)JS"
		R"JS(		if (tag == 'input' && (type == 'submit' || type == 'button' || type == 'reset')))JS"
		R"JS(			elem.click();)JS"
		R"JS(		if (tag == 'input' || tag == 'textarea') {)JS"
		R"JS(			elem.focus();)JS"
		R"JS(			elem.setSelectionRange(elem.value.length, elem.value.length);)JS"
		R"JS(		})JS"
		R"JS(		if (tag == 'input' && (type == 'radio' || type == 'checkbox')))JS"
		R"JS(			elem.checked = !elem.checked;)JS"
		R"JS(	})JS"
		R"JS(})JS"
		, strlen(selection) > 0 ? str2int(arg.string, selection) : -1
	);
	run_js(frame, js);
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
	dmenu(query, sizeof(query), NULL, 0, "", 0);

	char url_str[256];
	strcpy(url_str, arg.string);
	encode_url(&url_str[strlen(url_str)], query);
	cef_string_t url = {};
	cef_string_utf8_to_utf16(url_str, strlen(url_str), &url);

	cef_frame_t* frame = browser->get_main_frame(browser);
	frame->load_url(frame, &url);
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
		setsid();
		signal(SIGHUP, SIG_IGN);

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

size_t dmenu(char* dest, size_t len_dest, const char* const * items, size_t num_items, const char* initial_args, int lines)
{
	char buffer[256];
	if (!dest)
	{
		dest = buffer;
		len_dest = sizeof(buffer);
	}

	char lines_arg[32] = "";
	snprintf(lines_arg, sizeof(lines_arg), "%s -l %i", dmenu_path, lines);

	const char* const argv[] = {shell_path, "-c", lines > 0 ? lines_arg : dmenu_path, NULL};

	FILE* in;
	FILE* out;

	if (spawn(argv, &in, &out, NULL) == 0)
	{
		fprintf(in, "%s\n", initial_args);
		for (size_t i = 0; i < num_items; i++)
			fprintf(in, "%s\n", items[i]);
		fclose(in);

		for (size_t i = 0; i < len_dest - 1; i++)
		{
			dest[i] = '\0';
			int c = fgetc(out);
			if (c == '\n' || c == EOF) break;
			dest[i] = c;
			dest[i + 1] = '\0';
		}
		fclose(out);

		for (size_t i = 0; i < num_items; i++)
			if (strlen(dest) == strlen(items[i]) && strcmp(dest, items[i]) == 0)
				return i;
		return -1;
	}

	return -1;
}

void run_js(cef_frame_t* frame, const char* js_str)
{
	cef_string_t js = {};
	cef_string_utf8_to_utf16(js_str, strlen(js_str), &js);

	frame->execute_java_script(frame, &js, frame->get_url(frame), 0);
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
	global_state.cache_path = cache_path_str;

	strncpy(cache_path_str, argv[1], sizeof(cache_path_str));
	cef_string_t cache_path = {};
	cef_string_utf8_to_utf16(cache_path_str, strlen(cache_path_str), &cache_path);

	char history_path[256] = "";
	strncpy(history_path, cache_path_str, sizeof(history_path));
	strcat(history_path, "/history");
	FILE* history_cache_in = fopen(history_path, "r");
	if (history_cache_in != NULL)
	{
		int c;
		size_t idx = 0;
		while ((c = fgetc(history_cache_in)) != EOF)
			history[idx++] = c;
		history[idx] = '\0';
	}
	history_cache = fopen(history_path, "aw");

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

	// in child processes, argv[0] contains all arguments delimited by spaces
	char arg_arr[3][256];
	sscanf(argv[0], " %s %s %s", arg_arr[0], arg_arr[1], arg_arr[2]);

	global_state.exe = arg_arr[0];
	strncpy(cache_path_str, arg_arr[1], sizeof(cache_path_str));
	char url[256];
	strncpy(url, arg_arr[2], sizeof(url));

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

