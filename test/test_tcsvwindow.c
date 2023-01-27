#include <gtk/gtk.h>
#include "../tcsvwindow.c"

static void
test_t_csv_window_read (TCsvWindow *win) {
  GFile *file;
  int i, j;
  GtkStringList *record;
/* The elements of sample1.csv */
  char *a[3][4] = {
    {"a", "bc", "def", "あいう"},
    {"0", "", "3", "6"},
    {"one", "二", "three", "and \"four\"."}
  };

  file = g_file_new_for_path ("sample1.csv");
  t_csv_window_read (T_CSV_WINDOW (win), file);
  g_object_unref (file);
  for (j=0; j<4; ++j)
    if (g_strcmp0 (gtk_string_list_get_string (win->header, j), a[0][j]) != 0)
      g_print ("column(%d) of the header should be '%s', but is '%s'.\n", j, a[0][j], gtk_string_list_get_string (win->header, j));
  for (i=0; i<2; ++i) {
    record = GTK_STRING_LIST (g_list_model_get_item (G_LIST_MODEL (win->body), i));
    for (j=0; j<4; ++j)
      if (g_strcmp0 (gtk_string_list_get_string (record, j), a[i+1][j]) != 0)
        g_print ("The element (%d, %d) should be '%s(, but is '%s'.\n", i, j, a[i+1][j], gtk_string_list_get_string (record, j));
    g_object_unref (record);
  }
}

static void
app_activate (GApplication *application) {
  GtkApplication *app = GTK_APPLICATION (application);
  GtkWidget *win;

  win = t_csv_window_new (app);
  test_t_csv_window_read (T_CSV_WINDOW (win));
  gtk_window_destroy (GTK_WINDOW (win));

}

#define APPLICATION_ID "com.github.ToshioCP.test_win"

int
main (int argc, char **argv) {
  GtkApplication *app;
  int stat;

  g_print ("Test t_csv_window_read.\n");

  app = gtk_application_new (APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (GTK_APPLICATION (app), "activate", G_CALLBACK (app_activate), NULL);
  stat =g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
  return stat;
}
