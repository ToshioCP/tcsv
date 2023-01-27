#ifndef __T_CSV_WINDOW_H__
#define __T_CSV_WINDOW_H__

#include <gtk/gtk.h>

#define T_TYPE_CSV_WINDOW t_csv_window_get_type ()
G_DECLARE_FINAL_TYPE (TCsvWindow, t_csv_window, T, CSV_WINDOW, GtkApplicationWindow)

gboolean
t_csv_window_is_saved (TCsvWindow *win);

GtkWidget *
t_csv_window_new_with_file (GtkApplication *app, GFile *file);

GtkWidget *
t_csv_window_new (GtkApplication *app);

#endif /* __T_CSV_WINDOW_H__ */

