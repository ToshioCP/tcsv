#pragma once

/* tcsvstringlist.h */

// This file provides functions for GListStore of TCsvStr.
// It defines no onjects.
// The GListStore object is called "tcsv_stringlist" or shortly "sl" here.

/* type check */
gboolean
sl_is_tcsv_stringlist (GListStore *self);

/* setter and getter */
// The return value can be NULL when i is greater than or equal to the number of items.
char *
sl_get_string (GListStore *self, guint i);

const char *
sl_look_string (GListStore *self, guint i);

void
sl_set_string (GListStore *self, guint i, const char *s);

void
sl_take_string (GListStore *self, guint i, char *s);

/* append, insert */
void
sl_append_string (GListStore *self, const char *s);

void
sl_insert_string (GListStore *self, guint i, const char *s);

/* swap strings between two tcsv_stringlist*/
void
sl_swap_strings (GListStore *self, GListStore *other);

/* ----- converter ----- */
/* tcsv_stringlist <=> GtkStringList */

GtkStringList *
sl_to_gtk_string_list (GListStore *self);

GListStore *
sl_new_with_gtk_string_list (GtkStringList *stringlist);
