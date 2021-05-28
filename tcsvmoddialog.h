#ifndef __T_CSV_MOD_DIALOG_H__
#define __T_CSV_MOD_DIALOG_H__

#include <gtk/gtk.h>

#define T_TYPE_CSV_MOD_DIALOG t_csv_mod_dialog_get_type ()
G_DECLARE_FINAL_TYPE (TCsvModDialog, t_csv_mod_dialog, T, CSV_MOD_DIALOG, GtkDialog)

typedef struct _new_field_config new_field_config;
struct _new_field_config {
  char *header;
  int old_position;
};

GSList *
t_csv_mod_dialog_get_table (TCsvModDialog *mod_dialog);

GtkWindow *
t_csv_mod_dialog_get_window (TCsvModDialog *mod_dialog);

GtkWidget *
t_csv_mod_dialog_new (GtkWindow *win, GListStore *header);

#define T_TYPE_MODIFY (t_modify_get_type ())
G_DECLARE_FINAL_TYPE (TModify, t_modify, T, MODIFY, GObject)

#endif /* __T_CSV_MOD_DIALOG_H__ */

