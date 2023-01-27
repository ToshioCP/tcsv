#include <gtk/gtk.h>
#include <stdlib.h>
#include "tcsvmoddialog.h"
#include "tmodify.h"

struct _TCsvModDialog
{
  GtkDialog parent;
  GtkColumnView *columnview;
  GtkColumnViewColumn *column4;
  GListStore *liststore; /* list of tmodify */
  GtkEntryBuffer *entry_buffer;
};

G_DEFINE_TYPE (TCsvModDialog, t_csv_mod_dialog, GTK_TYPE_DIALOG);

/* ----- Button "clicked" signal handlers ----- */
void
mod_up_cb (GtkButton *btn, gpointer user_data) {
  GtkListItem *listitem = GTK_LIST_ITEM (user_data);
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btn), T_TYPE_CSV_MOD_DIALOG));
  TModify *modify;
  int i;

  i = gtk_list_item_get_position (listitem);
  if (i == GTK_INVALID_LIST_POSITION || i == 0)
    return;
  modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), i));
  g_list_store_remove (mod_dialog->liststore, i);
  g_list_store_insert (mod_dialog->liststore, i-1, modify);
  g_object_unref (modify);
}

void
mod_down_cb (GtkButton *btn, gpointer user_data) {
  GtkListItem *listitem = GTK_LIST_ITEM (user_data);
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btn), T_TYPE_CSV_MOD_DIALOG));
  TModify *modify;
  int i, n_items;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  i = gtk_list_item_get_position (listitem);
  if (i == GTK_INVALID_LIST_POSITION || i == n_items-1)
    return;
  modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), i));
  g_list_store_insert (mod_dialog->liststore, i+2, modify);
  g_list_store_remove (mod_dialog->liststore, i);
  g_object_unref (modify);
}

void
mod_app_cb (GtkButton *btnapp, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  TModify *modify;

  modify = t_modify_new_with_data (-1, -1, "", "");
  g_list_store_append (mod_dialog->liststore, modify);
  g_object_unref (modify);
}

void
mod_ins_cb (GtkButton *btnins, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  TModify *modify;
  int n_items, position;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  position = atoi (gtk_entry_buffer_get_text (mod_dialog->entry_buffer));
  position = position < 0 ? 0 : position;
  position = position >= n_items ? n_items - 1 : position;
  modify = t_modify_new_with_data (-1, -1, "", "");
  g_list_store_insert (mod_dialog->liststore, position, modify);
  g_object_unref (modify);
}

void
mod_clean_cb (GtkButton *btnrm, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  TModify *modify;
  int j, n_items;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  for (j=n_items-1; j>=0; --j) {
    modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), j));
    if (t_modify_get_old_position (modify) == -1
            && g_strcmp0 (t_modify_look_old_string (modify), "") == 0
            && g_strcmp0 (t_modify_look_new_string (modify), "") == 0)
      g_list_store_remove (mod_dialog->liststore, j);
    g_object_unref (modify);
  }
}

/* ----- Handler for the closure tag ----- */
char *
get_state (GtkListItem *listitem, char *old_field, char *new_field) {
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));
  gboolean old, new;

  if (! T_IS_MODIFY (modify)) /* though unexpected */
    return NULL;
  old = old_field && *old_field; /* not NULL nor "" */
  new = new_field && *new_field;
  if ((! old) && (! new))
    return NULL;
  else if ((! old) && new)
    return g_strdup ("Added");
  else if (old && (! new))
    return g_strdup ("Removed");
  else if (old && new && strcmp (old_field, new_field) != 0)
    return g_strdup ("Changed");
  else /* old && new && strcmp (old_field, new_field) == 0 */
    return NULL;
}

/* ----- setup, bind and unbind for GtkSignalListItemFactory ----- */
static void
setup4_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_text_new ();
  gtk_list_item_set_child (listitem, text);
}

static void
bind4_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_list_item_get_child (listitem);
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));
  GtkEntryBuffer *buffer;
  GBinding *bind;

  buffer = gtk_text_get_buffer (GTK_TEXT (text));
  gtk_entry_buffer_set_text (buffer, t_modify_look_new_string (modify), -1);
  bind = g_object_bind_property (buffer, "text", modify, "new-string", G_BINDING_DEFAULT);
  g_object_set_data (G_OBJECT (listitem), "bind", bind);
}

static void
unbind4_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GBinding *bind = G_BINDING (g_object_get_data (G_OBJECT (listitem), "bind"));

  g_binding_unbind(bind);
  g_object_set_data (G_OBJECT (listitem), "bind", NULL);
}

static void
t_csv_mod_dialog_init (TCsvModDialog *mod_dialog) {
  gtk_widget_init_template (GTK_WIDGET (mod_dialog));
  GtkListItemFactory *factory;

  factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (mod_dialog->column4, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup4_cb), NULL);
  g_signal_connect (factory, "bind", G_CALLBACK (bind4_cb), NULL);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind4_cb), NULL);
}

static void
t_csv_mod_dialog_class_init (TCsvModDialogClass *class) {
  // GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvmoddialog.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, column4);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, liststore);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, entry_buffer);
}

/* ----- Return list store of modify object ----- */
// Returned value is owned by the caller.
// Returned value can be NULL.
GListStore *
t_csv_mod_dialog_get_list_modify (TCsvModDialog *mod_dialog) {
  return g_object_ref (mod_dialog->liststore);
}

GtkWidget *
t_csv_mod_dialog_new (GtkWindow *win, GtkStringList *header) {
  g_return_val_if_fail (GTK_IS_WINDOW (win), NULL);
  g_return_val_if_fail (GTK_IS_STRING_LIST (header) || header == NULL, NULL);

  TCsvModDialog *mod_dialog;
  int j, n_items;
  TModify *modify;
  const char *old;

  mod_dialog = T_CSV_MOD_DIALOG (g_object_new (T_TYPE_CSV_MOD_DIALOG, "transient-for", win, NULL));
  if (header) {
    n_items = g_list_model_get_n_items (G_LIST_MODEL (header));
    for (j=0; j<n_items; ++j) {
      old = gtk_string_list_get_string (header, j);
      modify = t_modify_new_with_data (j, -1, old, old);
      g_list_store_append (mod_dialog->liststore, modify);
      g_object_unref (modify);
    }
  }
  return GTK_WIDGET (mod_dialog);
}
