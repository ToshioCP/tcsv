#include <gtk/gtk.h>
#include "../tcsvmoddialog.h"
#include "../tmodify.h"
#include "../tcsvstr.h"
#include "../tcsvstringlist.h"

GtkWidget *
create_mod_dialog (GtkWindow *win) {
  const char *s_header[3] = {"one", "two", "three"};
  GListStore *header = g_list_store_new (T_TYPE_CSV_STR);
  int i;
  for (i=0; i<3; ++i) {
    sl_append_string (header, s_header[i]);
  }
  GtkWidget *mod_dialog = t_csv_mod_dialog_new (win, header);
  g_object_unref (header);
  return mod_dialog;
}

void
mod_dialog_response_cb (TCsvModDialog *mod_dialog, int response, gpointer user_data) {
  GtkWindow *win = GTK_WINDOW (user_data);
  int j, n_items;
  GListStore *list_modify;
  TModify *modify;

  list_modify = t_csv_mod_dialog_get_list_modify (mod_dialog);
  n_items = g_list_model_get_n_items (G_LIST_MODEL (list_modify));
  g_print ("The following is returned by the modify dialog,\n\n");
  for (j=0; j<n_items; ++j) {
    modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (list_modify), j));
    g_print ("%d, %s, %s\n",t_modify_get_old_position (modify), t_modify_look_old_string (modify), t_modify_look_new_string (modify));
    g_object_unref (modify);
  }
  g_object_unref (list_modify);
  gtk_window_destroy (GTK_WINDOW (mod_dialog));
  gtk_window_destroy (win);
}

static void
app_activate (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkWidget *win;
  GtkWidget *mod_dialog;

  win = GTK_WIDGET (gtk_application_get_active_window(app));
  mod_dialog = create_mod_dialog (GTK_WINDOW (win));
  g_signal_connect (mod_dialog, "response", G_CALLBACK (mod_dialog_response_cb), win);
  gtk_window_present (GTK_WINDOW (mod_dialog));
}

static void
app_startup (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkCssProvider *provider0;

  gtk_application_window_new (app);
  provider0 = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider0, "text:focus {border: 1px solid gray;} columnview listview row button.current {background: red;}", -1);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
          GTK_STYLE_PROVIDER (provider0), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider0);
}

#define APPLICATION_ID "com.github.ToshioCP.test_rec_dialog"

int
main (int argc, char **argv) {
  GtkApplication *app;
  int stat;

  g_print ("Test TCsvModDialog\n\n");
  g_print ("Type strings into entries and click on save/cancel button.\n");
  g_print ("The strings will be printed below. Check them to be the same as you type.\n");

  app = gtk_application_new (APPLICATION_ID, G_APPLICATION_HANDLES_OPEN);

  g_signal_connect (GTK_APPLICATION (app), "startup", G_CALLBACK (app_startup), NULL);
  g_signal_connect (GTK_APPLICATION (app), "activate", G_CALLBACK (app_activate), NULL);

  stat = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return stat;
}
