#include "internal.h"



#ifdef YED_DO_ASSERTIONS
void yed_assert_fail(const char *msg, const char *fname, int line, const char *cond_str) {
    volatile int *trap;

    yed_term_exit();

    fprintf(stderr, "Assertion failed -- %s\n"
                    "at  %s :: line %d\n"
                    "    Condition: '%s'\n",
                    msg, fname, line, cond_str);

    trap = 0;
    (void)*trap;
}
#endif




uint64_t next_power_of_2(uint64_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}



char * pretty_bytes(uint64_t n_bytes) {
    uint64_t    s;
    double      count;
    char       *r;
    const char *suffixes[]
        = { " B", " KB", " MB", " GB", " TB", " PB", " EB" };

    s     = 0;
    count = (double)n_bytes;

    while (count >= 1024 && s < 7) {
        s     += 1;
        count /= 1024;
    }

    r = calloc(64, 1);

    if (count - floor(count) == 0.0) {
        sprintf(r, "%d", (int)count);
    } else {
        sprintf(r, "%.2f", count);
    }

    strcat(r, suffixes[s]);

    return r;
}



void yed_add_new_buff(void) {
    yed_buffer *new_buff;

    new_buff  = malloc(sizeof(*new_buff));
    *new_buff = yed_new_buff();
    array_push(ys->buff_list, new_buff);
}

void yed_init_output_stream(void) {
    ys->output_buffer = array_make_with_cap(char, 4096);
}

int output_buff_len(void) { return array_len(ys->output_buffer); }

void append_n_to_output_buff(char *s, int n) {
    int i;

    for (i = 0; i < n; i += 1) {
        array_push(ys->output_buffer, s[i]);
    }
}

void append_to_output_buff(char *s) {
    append_n_to_output_buff(s, strlen(s));
}

void append_int_to_output_buff(int i) {
    char s[16];

    sprintf(s, "%d", i);

    append_to_output_buff(s);
}

void flush_output_buff(void) {
    write(1, array_data(ys->output_buffer), array_len(ys->output_buffer));
    array_clear(ys->output_buffer);
}

void yed_service_reload(void) {
    tree_reset_fns(int,                yed_key_binding_ptr_t, ys->key_seq_map,      NULL);
    tree_reset_fns(yed_frame_id_t,     yed_frame_ptr_t,       ys->frames,           strcmp);
    tree_reset_fns(yed_command_name_t, yed_command,           ys->commands,         strcmp);
    tree_reset_fns(yed_command_name_t, yed_command,           ys->default_commands, strcmp);
    tree_reset_fns(yed_plugin_name_t,  yed_plugin_ptr_t,      ys->plugins,          strcmp);

    yed_set_default_commands();
    yed_reload_plugins();

    ys->small_message = "* reload serviced *";
}

int s_to_i(const char *s) {
    int i;

    sscanf(s, "%d", &i);

    return i;
}
