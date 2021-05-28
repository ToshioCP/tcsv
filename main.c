#include <gtk/gtk.h>
#include "tcsvwindow.h"

static void
app_activate (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkWidget *win;

  win = t_csv_window_new (app);
  gtk_widget_show (GTK_WIDGET (win));
}

static void
app_open (GApplication *application, GFile ** files, gint n_files, const gchar *hint) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkWidget *win;

  win = t_csv_window_new_with_file (app, files[0]);
  gtk_widget_show (win);
}

static void
app_startup (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  int i;

/* ----- accelerator ----- */ 
  struct {
    const char *action;
    const char *accels[2];
  } action_accels[] = {
    { "win.apprec", { "<Control>e", NULL } },
    { "win.rmrec", { "<Control>r", NULL } },
    { "win.new", { "<Control>n", NULL } },
    { "win.open", { "<Control>o", NULL } },
    { "win.save", { "<Control>s", NULL } },
    { "win.close", { "<Control>w", NULL } },
    { "win.saveas", { "<Shift><Control>s", NULL } },
    { "win.pref", { "<Control>p", NULL } },
    { "win.quit", { "<Control>q", NULL } },
  };

  for (i = 0; i < G_N_ELEMENTS(action_accels); i++)
    gtk_application_set_accels_for_action(GTK_APPLICATION(app), action_accels[i].action, action_accels[i].accels);

}

#define APPLICATION_ID "com.github.ToshioCP.csv"

int
main (int argc, char **argv) {
  GtkApplication *app;
  int stat;

  if (argc >1) {
    g_printerr ("Tcsv: too many arguments.\n");
    g_printerr ("Usage; tcsv [csv_file]\n");
    return 1;
  }
 
  app = gtk_application_new (APPLICATION_ID, G_APPLICATION_HANDLES_OPEN);

  g_signal_connect (app, "startup", G_CALLBACK (app_startup), NULL);
  g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
  g_signal_connect (app, "open", G_CALLBACK (app_open), NULL);

  stat =g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
  return stat;
}

