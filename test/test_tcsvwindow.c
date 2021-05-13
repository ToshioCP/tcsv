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

static void
test_list_model (TCsvWindow *win) {
  GFile *file;
  int n, i, j;
  const char *s;
  GtkStringList *stringlist;
/* The elements of sample1.csv */
  char *a[3][4] = {
    {"a", "bc", "def", "あいう"},
    {"0", "", "3", "6"},
    {"one", "二", "three", "and \"four\"."}
  };

  file = g_file_new_for_path ("sample1.csv");
  t_csv_window_read (T_CSV_WINDOW (win), file);
  if (win->n_row != 2 || win->n_column != 4) {
    g_print ("win->n_row or win->n_column is wrong.\n");
    g_print ("They should be (2, 4), but they are (%d, %d).\n", win->n_row, win->n_column);
    return;
  }
  if (! GTK_IS_STRING_LIST (win->header))
    g_print ("win->header isn't GTKSTRINGList.\n");
  else if ((n = g_list_model_get_n_items (G_LIST_MODEL (win->header))) != win->n_column) {
    g_print ("The number of culomns of the header and win->column are different.");
    g_print ("Number of culomns is %d.\n", n);
    g_print ("win->n_culomn is %d\n", win->n_column);
  } else
    for (j=0; j<win->n_column; ++j) {
      s = gtk_string_list_get_string (win->header, j);
      if (s == NULL)
        g_print ("column(%d) of the header is NULL, not a string.\n", j);
      else if (strcmp (s, a[0][j]) != 0)
        g_print ("column(%d) of the header should be '%s', but is '%s'.\n", j, a[0][j], s);
    }
  if (! G_IS_LIST_STORE (win->liststore))
    g_print ("win->liststore isn't GListStore.\n");
  else if ((n = g_list_model_get_n_items (G_LIST_MODEL (win->liststore))) != win->n_row) {
      g_print ("Number of items of liststore != win->n_row\n");
      g_print ("Number of items is %d.\n", n);
      g_print ("win->n_row is %d\n", win->n_row);
  } else
    for (i=0; i<win->n_row; ++i) {
      stringlist = GTK_STRING_LIST (g_list_model_get_item (G_LIST_MODEL (win->liststore), i));
      if (! GTK_IS_STRING_LIST (stringlist))
        g_print ("Item(%d) of liststore is not GtkStringList.\n", i);
      else if ((n = g_list_model_get_n_items (G_LIST_MODEL (stringlist))) != win->n_column) {
        g_print ("The number of culomns of the row(%d) in the liststore and win->column are different.", i);
        g_print ("Number of culomns is %d.\n", n);
        g_print ("win->n_column is %d\n", win->n_column);
      } else
        for (j=0; j<win->n_column; ++j) {
          s = gtk_string_list_get_string (stringlist, j);
          if (s == NULL)
            g_print ("The element (%d, %d) is NULL, not a string.\n", i, j);
          else if (strcmp (s, a[i+1][j]) != 0)
            g_print ("The element (%d, %d) should be '%s(, but is '%s'.\n", i, j, a[i+1][j], s);
        }
    }
}

int
main (int argc, char **argv) {
  g_print ("Test ListModel.\n");
  run_app_with_test (argc, argv, test_list_model);

}
