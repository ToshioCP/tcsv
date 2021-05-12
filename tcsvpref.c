#include "tcsvpref.h"

struct _TCsvPref
{
  GtkDialog parent;
  GSettings *settings;
  GtkFontButton *fontbtn;
};

G_DEFINE_TYPE (TCsvPref, t_csv_pref, GTK_TYPE_DIALOG);

static void
t_csv_pref_dispose (GObject *gobject) {
  TCsvPref *pref = T_CSV_PREF (gobject);

  g_clear_object (&pref->settings);
  G_OBJECT_CLASS (t_csv_pref_parent_class)->dispose (gobject);
}

static void
t_csv_pref_init (TCsvPref *pref) {
  gtk_widget_init_template (GTK_WIDGET (pref));
  pref->settings = g_settings_new ("com.github.ToshioCP.tcsv");
  g_settings_bind (pref->settings, "font", pref->fontbtn, "font", G_SETTINGS_BIND_DEFAULT);
}

static void
t_csv_pref_class_init (TCsvPrefClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->dispose = t_csv_pref_dispose;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvpref.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvPref, fontbtn);
}

GtkWidget *
t_csv_pref_new (GtkWindow *win) {
  return GTK_WIDGET (g_object_new (T_TYPE_CSV_PREF, "transient-for", win, NULL));
}

