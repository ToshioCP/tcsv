#include <gtk/gtk.h>
#include "../tcsvrecdialog.h"
#include "../tcsvstr.h"
#include "../tcsvstringlist.h"


GtkWidget *
create_rec_dialog (GtkWindow *win) {
  const char *s_header[4] = {"one", "two", "three"};
  const char *s_record[4] = {"1", "2", "3"};
  GListStore *header = g_list_store_new (T_TYPE_CSV_STR);
  GListStore *record = g_list_store_new (T_TYPE_CSV_STR);
  int i;
  for (i=0; i<3; ++i) {
    sl_append_string (header, s_header[i]);
    sl_append_string (record, s_record[i]);
  }
  GtkWidget *rec_dialog = t_csv_rec_dialog_new (win, header, record);
  g_object_unref (header);
  g_object_unref (record);
  return rec_dialog;
}

void
rec_dialog_response_cb (TCsvRecDialog *rec_dialog, int response, gpointer user_data) {
  int j, n_items;
  GListStore *record;
  GtkWindow *win = GTK_WINDOW (user_data);

  record = t_csv_rec_dialog_get_liststore (rec_dialog);
  n_items = g_list_model_get_n_items (G_LIST_MODEL (record));
  g_print ("The following is the string returned by the record dialog,\n\n");
  for (j=0; j<n_items; ++j)
    g_print ("%s\n", sl_look_string (record, j));
  g_object_unref (record);
  gtk_window_destroy (GTK_WINDOW (rec_dialog));
  gtk_window_destroy (win);
}

static void
app_activate (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkWidget *win;
  GtkWidget *rec_dialog;

  win = GTK_WIDGET (gtk_application_get_active_window(app));
  rec_dialog = create_rec_dialog (GTK_WINDOW (win));
  g_signal_connect (rec_dialog, "response", G_CALLBACK (rec_dialog_response_cb), win);
  gtk_window_present (GTK_WINDOW (rec_dialog));
}

static void
app_startup (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkCssProvider *provider0;

  gtk_application_window_new (app);
  provider0 = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider0, "text:focus {border: 1px solid gray;}", -1);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
          GTK_STYLE_PROVIDER (provider0), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider0);
}

#define APPLICATION_ID "com.github.ToshioCP.test_rec_dialog"

int
main (int argc, char **argv) {
  GtkApplication *app;
  int stat;

  g_print ("Test TCsvRecDialog\n\n");
  g_print ("Type strings into entries and click on save/cancel button.\n");
  g_print ("The strings will be printed below. Check them to be the same as you type.\n");

  app = gtk_application_new (APPLICATION_ID, G_APPLICATION_HANDLES_OPEN);

  g_signal_connect (GTK_APPLICATION (app), "startup", G_CALLBACK (app_startup), NULL);
  g_signal_connect (GTK_APPLICATION (app), "activate", G_CALLBACK (app_activate), NULL);

  stat = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return stat;
}
