#ifndef __T_CSV_ALERT_H__
#define __T_CSV_ALERT_H__

#include <gtk/gtk.h>

#define T_TYPE_CSV_ALERT t_csv_alert_get_type ()
G_DECLARE_FINAL_TYPE (TCsvAlert, t_csv_alert, T, CSV_ALERT, GtkDialog)

void
t_csv_alert_set_message (TCsvAlert *alert, const char *message);

void
t_csv_alert_set_button_label (TCsvAlert *alert, const char *label);

GtkWidget *
t_csv_alert_new (GtkWindow *win);

#endif /* __T_CSV_ALERT_H__ */

