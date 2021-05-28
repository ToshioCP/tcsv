#ifndef __T_CSV_NEW_DIALOG_H__
#define __T_CSV_NEW_DIALOG_H__

#include <gtk/gtk.h>
#include "tcsvwindow.h"

#define T_TYPE_CSV_NEW_DIALOG t_csv_new_dialog_get_type ()
G_DECLARE_FINAL_TYPE (TCsvNewDialog, t_csv_new_dialog, T, CSV_NEW_DIALOG, GtkDialog)

GListStore *
t_csv_new_dialog_get_record (TCsvNewDialog *new_dialog);

GtkWindow *
t_csv_new_dialog_get_window (TCsvNewDialog *new_dialog);

GtkWidget *
t_csv_new_dialog_new (GtkWindow *win);

#endif /* __T_CSV_NEW_DIALOG_H__ */

