#include <gtk/gtk.h>
#include "tcsvapplication.h"
#include "tcsvwindow.h"
#include "tcsvpref.h"
#include "tcsvalert.h"

static void
pref_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvApplication *app = T_CSV_APPLICATION (user_data);
  GtkWidget *pref;

  pref = t_csv_pref_new (gtk_application_get_active_window (GTK_APPLICATION (app)));
  gtk_window_present (GTK_WINDOW (pref));
}

/* alert response signal handler */
static void
quit_alert_response_cb (GtkDialog *alert, int response_id, gpointer user_data) {
  TCsvApplication *app = T_CSV_APPLICATION (user_data);

  gtk_window_destroy (GTK_WINDOW (alert));
  if (response_id == GTK_RESPONSE_ACCEPT)
    g_application_quit (G_APPLICATION (app));
}

static void
quit_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvApplication *app = T_CSV_APPLICATION (user_data);
  TCsvWindow *win;
  GList *list;
  TCsvAlert *alert;

  for (list = gtk_application_get_windows (GTK_APPLICATION (app)); list; list = list->next) {
    win = T_CSV_WINDOW (list->data);
    if (! t_csv_window_is_saved (win)) {
      alert = T_CSV_ALERT (t_csv_alert_new (GTK_WINDOW (win)));
      t_csv_alert_set_message (alert, "Contents aren't saved yet.\nAre you sure to quit?");
      t_csv_alert_set_button_label (alert, "Quit");
      g_signal_connect (GTK_DIALOG (alert), "response", G_CALLBACK (quit_alert_response_cb), app);
      gtk_window_present (GTK_WINDOW (alert));
      return;
    }
  }
  g_application_quit (G_APPLICATION (app));
}

static void
app_activate (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkWidget *win;

  win = t_csv_window_new (app);
  gtk_window_present (GTK_WINDOW (win));
}

static void
app_open (GApplication *application, GFile ** files, gint n_files, const gchar *hint) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkWidget *win;

  win = t_csv_window_new_with_file (app, files[0]);
  gtk_window_present (GTK_WINDOW (win));
}

static void
app_startup (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkCssProvider *provider0;
  int i;

/* ---- actions ---- */
  const GActionEntry app_entries[] = {
    { "pref", pref_activated, NULL, NULL, NULL },
    { "quit", quit_activated, NULL, NULL, NULL },
  };
  g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);

/* ----- accelerator ----- */ 
  struct {
    const char *action;
    const char *accels[2];
  } action_accels[] = {
    { "win.apprec", { "<Control>d", NULL } },
    { "win.rmrec", { "<Control>r", NULL } },
    { "win.edrec", { "<Control>e", NULL } },
    { "win.new", { "<Control>n", NULL } },
    { "win.open", { "<Control>o", NULL } },
    { "win.save", { "<Control>s", NULL } },
    { "win.close", { "<Control>w", NULL } },
    { "win.saveas", { "<Shift><Control>s", NULL } },
    { "app.pref", { "<Control>p", NULL } },
    { "app.quit", { "<Control>q", NULL } },
  };

  for (i = 0; i < G_N_ELEMENTS(action_accels); i++)
    gtk_application_set_accels_for_action(GTK_APPLICATION(app), action_accels[i].action, action_accels[i].accels);

  provider0 = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider0, "text:focus {border: 1px solid gray;} columnview listview row button.current {background: red;}", -1);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
          GTK_STYLE_PROVIDER (provider0), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider0);
}

#define APPLICATION_ID "com.github.ToshioCP.csv"

int
main (int argc, char **argv) {
  TCsvApplication *app;
  int stat;

  if (argc > 2) {
    g_printerr ("Tcsv: too many arguments.\n");
    g_printerr ("Usage; tcsv [csv_file]\n");
    return 1;
  }
 
  app = t_csv_application_new (APPLICATION_ID, G_APPLICATION_HANDLES_OPEN);

  g_signal_connect (GTK_APPLICATION (app), "startup", G_CALLBACK (app_startup), NULL);
  g_signal_connect (GTK_APPLICATION (app), "activate", G_CALLBACK (app_activate), NULL);
  g_signal_connect (GTK_APPLICATION (app), "open", G_CALLBACK (app_open), NULL);

  stat =g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
  return stat;
}

