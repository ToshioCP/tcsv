#include <gtk/gtk.h>
#include "../tcsvrecdialog.c"
#include "../css.h"
#include "../tstr.h"
#include "../tcsvwindow.h"

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
  g_free (t1);
  g_free (t2);
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
  g_free (t1);
  g_free (t2);
}

const char *s_header[4] = {"one", "two", "three", NULL};
const char *s_record[4] = {"1", "2", "3", NULL};

void
test_rec_dialog_new (void) {
  GListStore *header = g_list_store_new (T_TYPE_STR);
  GListStore *record = g_list_store_new (T_TYPE_STR);
  TStr *str;
  int j, n_items;
  n_items = 3;
  for (j=0; j<n_items; ++j) {
    str = t_str_new_with_string (s_header[j]);
    g_list_store_append (header, str);
  }
  for (j=0; j<n_items; ++j) {
    str = t_str_new_with_string (s_record[j]);
    g_list_store_append (record, str);
  }
  gtk_init ();
  GtkWidget *win = gtk_window_new ();
  TCsvRecDialog *rec_dialog = T_CSV_REC_DIALOG (t_csv_rec_dialog_new (GTK_WINDOW (win), header, record, 0));
  TCouple *couple;
  char *s1, *s2;

  if (g_list_model_get_n_items (G_LIST_MODEL (rec_dialog->noselection)) != n_items)
    g_print ("The number of items of rec_dialog->liststore is wrong.\n");
  else {
    for (j=0; j>n_items; ++j) {
      couple = T_COUPLE (g_list_model_get_item (G_LIST_MODEL (rec_dialog->noselection), j));
      t_couple_get_strings (couple, &s1, &s2);
      if (strcmp (s1, s_header[j]) != 0) {
        g_print ("%dth string (header) is different.\n", j);
        g_print ("Expected string is %s\n", s_header[j]);
        g_print ("Returned string is %s\n", s1);
      }
      if (strcmp (s2, s_record[j]) != 0) {
        g_print ("%dth string (record) is different.\n", j);
        g_print ("Expected string is %s\n", s_record[j]);
        g_print ("Returned string is %s\n", s2);
      }
      g_free (s1);
      g_free (s2);
      g_object_unref (couple);
    } 
  }
  if (win != t_csv_rec_dialog_get_window (rec_dialog))
    g_print ("The winodw dialog returned is wrong.\n");
  if (t_csv_rec_dialog_get_position (rec_dialog) != 0)
    g_print ("The position dialog returned is wrong.\n");
  GListStore *record2 = t_csv_rec_dialog_get_record (rec_dialog);
  for (j=0; j<n_items; ++j) {
    str = g_list_model_get_item (G_LIST_MODEL (record2), j);
    s2 = t_str_get_string (str);
    if (strcmp (s2, s_record[j]) != 0) {
      g_print ("%dth string (record) is different.\n", j);
      g_print ("Expected string is %s\n", s_record[j]);
      g_print ("Returned string is %s\n", s2);
    }
    g_free (s2);
    g_object_unref (str);
  }
  gtk_window_destroy (GTK_WINDOW (rec_dialog));
  gtk_window_destroy (GTK_WINDOW (win));
  g_object_unref (record);
  g_object_unref (header);
  g_object_unref (record2);
}

int
main (int argc, char **argv) {
  g_print ("Test TCouple\n");
  test_t_couple ();
  g_print ("Test t_cdv_rec_dialog_new\n");
  test_rec_dialog_new ();
}


