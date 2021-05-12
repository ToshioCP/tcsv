#include "tcsvalert.h"

struct _TCsvAlert
{
  GtkDialog parent;
  GtkLabel *lb_alert;
  GtkButton *btn_accept;
};

G_DEFINE_TYPE (TCsvAlert, t_csv_alert, GTK_TYPE_DIALOG);

void
t_csv_alert_set_message (TCsvAlert *alert, const char *message) {
  gtk_label_set_text (alert->lb_alert, message);
}

void
t_csv_alert_set_button_label (TCsvAlert *alert, const char *label) {
  gtk_button_set_label (alert->btn_accept, label);
}

static void
t_csv_alert_init (TCsvAlert *alert) {
  gtk_widget_init_template (GTK_WIDGET (alert));
}

static void
t_csv_alert_class_init (TCsvAlertClass *class) {
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvalert.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvAlert, lb_alert);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvAlert, btn_accept);
}

GtkWidget *
t_csv_alert_new (GtkWindow *win) {
  return GTK_WIDGET (g_object_new (T_TYPE_CSV_ALERT, "transient-for", win, NULL));
}

