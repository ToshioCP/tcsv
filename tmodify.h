#pragma once

#include <gtk/gtk.h>

#define T_TYPE_MODIFY (t_modify_get_type ())
G_DECLARE_FINAL_TYPE (TModify, t_modify, T, MODIFY, GObject)

/* setter and getter */
void
t_modify_set_list_item (TModify *self, GtkListItem *listitem);

void
t_modify_set_old_position (TModify *self, int pos);

void
t_modify_set_new_position (TModify *self, int pos);

void
t_modify_set_old_string (TModify *self, const char *old);

void
t_modify_set_new_string (TModify *self, const char *new);

GtkListItem *
t_modify_get_list_item (TModify *self);

int
t_modify_get_old_position (TModify *self);

int
t_modify_get_new_position (TModify *self);

char *
t_modify_get_old_string (TModify *self);

char *
t_modify_get_new_string (TModify *self);

const char *
t_modify_look_old_string (TModify *self);

const char *
t_modify_look_new_string (TModify *self);

void
t_modify_swap_data (TModify *self, TModify *other);

TModify *
t_modify_new_with_data (int old_position, int new_position, const char *old_string, const char *new_string);

TModify *
t_modify_new (void);
