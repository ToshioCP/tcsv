#pragma once

#include <gtk/gtk.h>

/* TCsvStr is a wrapper object of a string */
#define T_TYPE_CSV_STR  (t_csv_str_get_type ())
G_DECLARE_FINAL_TYPE (TCsvStr, t_csv_str, T, CSV_STR, GObject)

/* setter and getter */

// s must not NULL for two following functions
void
t_csv_str_set_string (TCsvStr *self, const char *s);

void
t_csv_str_take_string (TCsvStr *self, char *s);

char *
t_csv_str_get_string (TCsvStr *self);

const char *
t_csv_str_look_string (TCsvStr *self);

/* create a new TCsvStr instance */

// s must not NULL
TCsvStr *
t_csv_str_new_with_string (const char *s);

TCsvStr *
t_csv_str_new (void);
