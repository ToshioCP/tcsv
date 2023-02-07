#include <gtk/gtk.h>
#include "tcsvstr.h"

/* tcsvstringlist.c */

// This file provides functions for GListStore of TCsvStr.
// It defines no onjects.
// The GListStore object is called "tcsv_stringlist" or shortly "sl" here.

/* type check */
gboolean
sl_is_tcsv_stringlist (GListStore *self) {
  if (G_IS_LIST_STORE (self) && g_list_model_get_item_type (G_LIST_MODEL (self)) == T_TYPE_CSV_STR)
    return TRUE;
  else
    return FALSE;

}

/* setter and getter */
// The return value can be NULL when i is greater than or equal to the number of items.
char *
sl_get_string (GListStore *self, guint i) {
  g_return_val_if_fail (sl_is_tcsv_stringlist (self), NULL);

  TCsvStr *str = T_CSV_STR (g_list_model_get_item (G_LIST_MODEL (self), i));
  if (str == NULL)
    return NULL;
  char *s = t_csv_str_get_string (str);
  g_object_unref (str);
  return s;
}

const char *
sl_look_string (GListStore *self, guint i) {
  g_return_val_if_fail (sl_is_tcsv_stringlist (self), NULL);

  TCsvStr *str = T_CSV_STR (g_list_model_get_item (G_LIST_MODEL (self), i));
  if (str == NULL)
    return NULL;
  const char *s = t_csv_str_look_string (str);
  g_object_unref (str);
  return s;
}

void
sl_set_string (GListStore *self, guint i, const char *s) {
  g_return_if_fail (sl_is_tcsv_stringlist (self));
  g_return_if_fail (i < g_list_model_get_n_items (G_LIST_MODEL (self)));

  TCsvStr *str = T_CSV_STR (g_list_model_get_item (G_LIST_MODEL (self), i));
  t_csv_str_set_string (str, s);
  g_object_unref (str);
}

void
sl_take_string (GListStore *self, guint i, char *s) {
  g_return_if_fail (sl_is_tcsv_stringlist (self));
  g_return_if_fail (i < g_list_model_get_n_items (G_LIST_MODEL (self)));

  TCsvStr *str = T_CSV_STR (g_list_model_get_item (G_LIST_MODEL (self), i));
  t_csv_str_take_string (str, s);
  g_object_unref (str);
}

void
sl_append_string (GListStore *self, const char *s) {
  g_return_if_fail (sl_is_tcsv_stringlist (self));

  TCsvStr *str = t_csv_str_new_with_string (s);
  g_list_store_append (self, str);
  g_object_unref (str);
}

void
sl_insert_string (GListStore *self, guint i, const char *s) {
  g_return_if_fail (sl_is_tcsv_stringlist (self));
  g_return_if_fail (i <= g_list_model_get_n_items (G_LIST_MODEL (self)));

  TCsvStr *str = t_csv_str_new_with_string (s);
  g_list_store_insert (self, i, str);
  g_object_unref (str);
}

void
sl_swap_strings (GListStore *self, GListStore *other) {
  g_return_if_fail (sl_is_tcsv_stringlist (self));
  g_return_if_fail (sl_is_tcsv_stringlist (other));
  g_return_if_fail (g_list_model_get_n_items (G_LIST_MODEL(self)) == g_list_model_get_n_items (G_LIST_MODEL(other)));

  TCsvStr *str_self, *str_other;
  int n_items = g_list_model_get_n_items (G_LIST_MODEL(self));
  int i;
  char *s;

  for (i=0; i<n_items; ++i) {
    str_self = T_CSV_STR (g_list_model_get_item (G_LIST_MODEL (self), i));
    str_other = T_CSV_STR (g_list_model_get_item (G_LIST_MODEL (other), i));
    s = t_csv_str_get_string (str_self);
    g_object_set (str_self, "string", t_csv_str_look_string (str_other), NULL);
    g_object_set (str_other, "string", s, NULL);
    g_free (s);
    g_object_unref (str_self);
    g_object_unref (str_other);
  }
}

/* ----- converter ----- */
/* tcsv_stringlist <=> GtkStringList */

GtkStringList *
sl_to_gtk_string_list (GListStore *self) {
  g_return_val_if_fail (sl_is_tcsv_stringlist (self), NULL);

  GtkStringList *stringlist;
  TCsvStr *str;
  int i, n_items;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (self));
  stringlist = gtk_string_list_new (NULL);
  for (i=0; i<n_items; ++i) {
    str = g_list_model_get_item (G_LIST_MODEL (self), i);
    gtk_string_list_append (stringlist, t_csv_str_look_string (str));
    g_object_unref (str);
  }
  return stringlist;
}

GListStore *
sl_new_with_gtk_string_list (GtkStringList *stringlist) {
  g_return_val_if_fail (GTK_IS_STRING_LIST (stringlist), NULL);

  GListStore *sl;
  TCsvStr *str;
  int i, n_items;

  sl = g_list_store_new (T_TYPE_CSV_STR);
  n_items = g_list_model_get_n_items (G_LIST_MODEL (stringlist));
  for (i=0; i<n_items; ++i) {    
    str = t_csv_str_new_with_string (gtk_string_list_get_string (stringlist, i));
    g_list_store_append (sl, str);
    g_object_unref (str);
  }
  return sl;
}
