#include <gtk/gtk.h>

#include "tcsvapplication.h"
#include "pfd2css.h"

struct _TCsvApplication {
  GtkApplication parent;
  GSettings *settings;
  GtkCssProvider *provider;
};

G_DEFINE_TYPE (TCsvApplication, t_csv_application, GTK_TYPE_APPLICATION)

/* gsettings changed::font signal handler */
static void
changed_font_cb (GSettings *settings, char *key, gpointer user_data) {
  TCsvApplication *app = T_CSV_APPLICATION (user_data);
  char *font, *s, *css;
  PangoFontDescription *pango_font_desc;

  font = g_settings_get_string (app->settings, "font");
  pango_font_desc = pango_font_description_from_string (font);
  g_free (font);
  s = pfd2css (pango_font_desc); // converts Pango Font Description into CSS style string
  css = g_strdup_printf ("label, text {%s}", s);
  gtk_css_provider_load_from_data (app->provider, css, -1);
  g_free (s);
  g_free (css);
}

// No default display exists when t_csv_application_init is called.
// It exists when the startup signal is emitted.
// Multiple handlers are possible.
static void
app_startup (GApplication *application) {
  TCsvApplication *app = T_CSV_APPLICATION (application);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
          GTK_STYLE_PROVIDER (app->provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void
t_csv_application_dispose (GObject *gobject) {
  TCsvApplication *app = T_CSV_APPLICATION (gobject);

  g_clear_object (&app->settings);
  g_clear_object (&app->provider);
  G_OBJECT_CLASS (t_csv_application_parent_class)->dispose (gobject);
}

static void
t_csv_application_init (TCsvApplication *app) {
  app->provider = gtk_css_provider_new ();
  app->settings = g_settings_new ("com.github.ToshioCP.tcsv");
  g_signal_connect (app->settings, "changed::font", G_CALLBACK (changed_font_cb), app);
  changed_font_cb(app->settings, "font", app);
  g_signal_connect (GTK_APPLICATION (app), "startup", G_CALLBACK (app_startup), NULL);
}

static void
t_csv_application_class_init (TCsvApplicationClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->dispose = t_csv_application_dispose;
}

TCsvApplication *
t_csv_application_new (const char* application_id, GApplicationFlags flags) {
  return T_CSV_APPLICATION (g_object_new (T_TYPE_CSV_APPLICATION, "application-id", application_id, "flags", flags, NULL));
}
