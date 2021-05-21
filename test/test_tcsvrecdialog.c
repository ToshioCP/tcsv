#include <gtk/gtk.h>
#include "../tcsvrecdialog.c"

void
test_t_couple (void) {
  char *s1 = "Hello\n";
  char *s2 = "Goodby\n";
  TCouple *couple = t_couple_new_with_strings (s1, s2);
  char *t1, *t2;

  t_couple_get_strings (couple, &t1, &t2);
  if (strcmp (s1, t1) != 0) {
    g_print ("Tcouple returned wrong pointer.\n");
    g_print ("Expected pointer (string) is %s\n", s1);
    g_print ("Returned pointer1 (string) is %s\n", t1);
  }
  if (strcmp (s2, t2) != 0) {
    g_print ("Tcouple returned wrong pointer.\n");
    g_print ("Expected pointer (string) is %s\n", s2);
    g_print ("Returned pointer2 (string) is %s\n", t2);
  }
  g_object_get (couple, "first-string", &t1, "second-string", &t2, NULL);
  if (strcmp (s1, t1) != 0) {
    g_print ("Tcouple returned wrong pointer.\n");
    g_print ("Expected pointer (string) is %s\n", s1);
    g_print ("Returned pointer1 (string) is %s\n", t1);
  }
  if (strcmp (s2, t2) != 0) {
    g_print ("Tcouple returned wrong pointer.\n");
    g_print ("Expected pointer (string) is %s\n", s2);
    g_print ("Returned pointer2 (string) is %s\n", t2);
  }
}

const char *s_header[4] = {"one", "two", "three", NULL};
const char *s_stringlist[4] = {"1", "2", "3", NULL};

void
test_rec_dialog_new (void) {
  GtkStringList *header = gtk_string_list_new (s_header);
  GtkStringList *stringlist = gtk_string_list_new (s_stringlist);
  gtk_init ();
  GtkWidget *win = gtk_window_new ();
  TCsvRecDialog *rec_dialog = T_CSV_REC_DIALOG (t_csv_rec_dialog_new (GTK_WINDOW (win), header, stringlist));
  int j, n_items;
  TCouple *couple;
  char *s1, *s2;

  n_items = 3;
/*  if (g_list_model_get_n_items (G_LIST_MODEL (rec_dialog->liststore)) != n_items)*/
  if (g_list_model_get_n_items (G_LIST_MODEL (rec_dialog->singleselection)) != n_items)
    g_print ("The number of items of rec_dialog->liststore is wront.\n");
  else {
    for (j=0; j>n_items; ++j) {
/*      couple = T_COUPLE (g_list_model_get_item (G_LIST_MODEL (rec_dialog->liststore), j));*/
      couple = T_COUPLE (g_list_model_get_item (G_LIST_MODEL (rec_dialog->singleselection), j));
/*      t_couple_get_strings (couple, &s1, &s2);*/
      g_object_get (couple, "first-string", &s1, "second-string", &s2, NULL);
      if (strcmp (s1, s_header[j]) != 0) {
        g_print ("%dth string (header) is different.\n", j);
        g_print ("Expected string is %s\n", s_header[j]);
        g_print ("Returned string is %s\n", s1);
      }
      if (strcmp (s2, s_stringlist[j]) != 0) {
        g_print ("%dth string (stringlist) is different.\n", j);
        g_print ("Expected string is %s\n", s_stringlist[j]);
        g_print ("Returned string is %s\n", s2);
      }
    } 
  }
  gtk_window_destroy (GTK_WINDOW (rec_dialog));
  gtk_window_destroy (GTK_WINDOW (win));
  g_object_unref (stringlist);
  g_object_unref (header);
}

int
main (int argc, char **argv) {
  g_print ("Test TCouple\n");
  test_t_couple ();
  g_print ("Test t_cdv_rec_dialog_new\n");
  test_rec_dialog_new ();
}


