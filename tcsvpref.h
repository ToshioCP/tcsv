#ifndef __T_CSV_PREF_H__
#define __T_CSV_PREF_H__

#include <gtk/gtk.h>

#define T_TYPE_CSV_PREF t_csv_pref_get_type ()
G_DECLARE_FINAL_TYPE (TCsvPref, t_csv_pref, T, CSV_PREF, GtkDialog)

GtkWidget *
t_csv_pref_new (GtkWindow *win);

#endif /* __T_CSV_PREF_H__ */

