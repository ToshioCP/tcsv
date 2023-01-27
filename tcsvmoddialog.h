#ifndef __T_CSV_MOD_DIALOG_H__
#define __T_CSV_MOD_DIALOG_H__

#include <gtk/gtk.h>

#define T_TYPE_CSV_MOD_DIALOG t_csv_mod_dialog_get_type ()
G_DECLARE_FINAL_TYPE (TCsvModDialog, t_csv_mod_dialog, T, CSV_MOD_DIALOG, GtkDialog)

GListStore *
t_csv_mod_dialog_get_list_modify (TCsvModDialog *mod_dialog);

GtkWidget *
t_csv_mod_dialog_new (GtkWindow *win, GtkStringList *header);

#endif /* __T_CSV_MOD_DIALOG_H__ */
