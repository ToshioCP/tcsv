#include <gtk/gtk.h>
#include "../tcsvwindow.c"

static void
app_activate (GApplication *application, gpointer user_data) {
  void  (*test) (TCsvWindow *) = (void (*)(TCsvWindow *)) user_data;
  GtkApplication *app = GTK_APPLICATION (application);
  GtkWidget *win;

  win = t_csv_window_new (app);
  (*test) (T_CSV_WINDOW (win));
  gtk_window_destroy (GTK_WINDOW (win));
}

static void
app_startup (GApplication *application) {
}

#define APPLICATION_ID "com.github.ToshioCP.csv"
void
run_app_with_test (int argc, char **argv, void test()) {
  GtkApplication *app;

  app = gtk_application_new (APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "startup", G_CALLBACK (app_startup), NULL);
  g_signal_connect (app, "activate", G_CALLBACK (app_activate), test);
  g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
}

void
test_list_model (TCsvWindow *win) {
  GFile *file;
  int n;
  const char *s;
  GtkStringList *stringlist;

  file = g_file_new_for_path ("sample1.csv");
  t_csv_window_read (T_CSV_WINDOW (win), file);
  if (! G_IS_LIST_STORE (win->liststore))
    g_print ("win->liststore isn't GListStore.\n");
  else {
    n = g_list_model_get_n_items (G_LIST_MODEL (win->liststore));
    if (n != win->n_row) {
      g_print ("Number f items of liststore != win->n_row\n");
      g_print ("Number f items is %d.\n", n);
      g_print ("win->n_row is %d\n", win->n_row);
    }
    stringlist = GTK_STRING_LIST (g_list_model_get_item (G_LIST_MODEL (win->liststore), 0));
    if (! GTK_IS_STRING_LIST (stringlist))
      g_print ("Item of liststore is not GtkStringList.\n");
    else if (strcmp ((s = gtk_string_list_get_string (stringlist, 0)), "0") != 0) {
      g_print ("(1,0) -- the data in the first record and first column is not an expected value.\n");
      g_print ("Expected value is 0\n");
      g_print ("Returned value is %s\n", s);
    }
  }
  if (! GTK_IS_STRING_LIST (win->header))
    g_print ("win->header is not GtkStringList\n");
}

int
main (int argc, char **argv) {
  g_print ("Test ListModel.\n");
  run_app_with_test (argc, argv, test_list_model);

}
