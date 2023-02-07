#include <gtk/gtk.h>
#include <stdlib.h>
#include "tcsvmoddialog.h"
#include "tmodify.h"
#include "tcsvstringlist.h"

struct _TCsvModDialog
{
  GtkDialog parent;
  GtkColumnView *columnview;
  GListStore *liststore; /* list of tmodify */
  int current_position;
};

G_DEFINE_TYPE (TCsvModDialog, t_csv_mod_dialog, GTK_TYPE_DIALOG);

static void
update_current (TCsvModDialog *mod_dialog, int new) { /* new can be negative (when no current line exists) */
  TModify *modify;
  int n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  GtkListItem *listitem;
  GtkButton *button;
  const char *non_current[1] = {NULL};
  const char *current[2] = {"current", NULL};

  if (new >= n_items)
    return;
  if (mod_dialog->current_position >=0 && mod_dialog->current_position <n_items) {
    modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), mod_dialog->current_position));
    if ((listitem = t_modify_get_list_item (modify)) != NULL) {
      button = GTK_BUTTON (gtk_list_item_get_child (listitem));
      gtk_widget_set_css_classes (GTK_WIDGET (button), non_current);
      g_object_unref (listitem);
    }
    g_object_unref (modify);
  }
  mod_dialog->current_position = new;
  if (mod_dialog->current_position >=0) {
    modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), mod_dialog->current_position));
    if ((listitem = t_modify_get_list_item (modify)) != NULL) {
      button = GTK_BUTTON (gtk_list_item_get_child (listitem));
      gtk_widget_set_css_classes (GTK_WIDGET (button), current);
      g_object_unref (listitem);
    }
    g_object_unref (modify);
  }
}

/* ----- Button "clicked" signal handlers ----- */
void
mod_up_cb (GtkButton *btn, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  TModify *modify, *modify_before;

  if (mod_dialog->current_position <= 0)
    return;
  modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), mod_dialog->current_position));
  modify_before = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), mod_dialog->current_position - 1));
  t_modify_swap_data (modify, modify_before);
  g_object_unref (modify);
  g_object_unref (modify_before);
  update_current (mod_dialog, mod_dialog->current_position - 1);
  gtk_window_set_focus (GTK_WINDOW (mod_dialog), NULL);
}

void
mod_down_cb (GtkButton *btn, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  TModify *modify, *modify_after;

  if (mod_dialog->current_position < 0 || mod_dialog->current_position >= g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore)) - 1)
    return;
  modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), mod_dialog->current_position));
  modify_after = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), mod_dialog->current_position + 1));
  t_modify_swap_data (modify, modify_after);
  g_object_unref (modify);
  g_object_unref (modify_after);
  update_current (mod_dialog, mod_dialog->current_position + 1);
  gtk_window_set_focus (GTK_WINDOW (mod_dialog), NULL);
}

void
mod_app_cb (GtkButton *btnapp, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  TModify *modify;
  int n_items, position;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  position = mod_dialog->current_position < 0 ? n_items - 1 : mod_dialog->current_position;
  modify = t_modify_new_with_data (-1, -1, "", "");
  g_list_store_insert (mod_dialog->liststore, position + 1, modify);
  update_current (mod_dialog, position + 1);
  g_object_unref (modify);
}

void
mod_ins_cb (GtkButton *btnins, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  TModify *modify;
  int position;

  position = mod_dialog->current_position < 0 ? 0 : mod_dialog->current_position;
  modify = t_modify_new_with_data (-1, -1, "", "");
  g_list_store_insert (mod_dialog->liststore, position, modify);
  update_current (mod_dialog, position);
  g_object_unref (modify);
}

void
mod_rm_cb (GtkButton *btnrm, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  TModify *modify;
  int n_items;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  if (mod_dialog->current_position < 0 || mod_dialog->current_position >= n_items)
    return;
  modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), mod_dialog->current_position));
  if (t_modify_get_old_position (modify) == -1
          && g_strcmp0 (t_modify_look_old_string (modify), "") == 0) {
    g_list_store_remove (mod_dialog->liststore, mod_dialog->current_position);
    update_current (mod_dialog, -1);
  } else {
    g_object_set (modify, "new-string", "", NULL);
  }
  g_object_unref (modify);
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
select_current_cb (GtkButton *btn, gpointer user_data) {
  GtkListItem *listitem = GTK_LIST_ITEM (user_data);
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btn), T_TYPE_CSV_MOD_DIALOG));

  update_current (mod_dialog, gtk_list_item_get_position (listitem));
  gtk_window_set_focus (GTK_WINDOW (mod_dialog), NULL);
}

static void
setup1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *button = gtk_button_new ();
  gtk_list_item_set_child (listitem, button);
  gtk_widget_set_focusable (button, FALSE);
  g_signal_connect (button, "clicked", G_CALLBACK (select_current_cb), listitem);
}

static void
bind1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  GtkWidget *button = gtk_list_item_get_child (listitem);
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));
  const char *non_current[1] = {NULL};
  const char *current[2] = {"current", NULL};

  t_modify_set_list_item (modify, listitem);
  if (mod_dialog->current_position == gtk_list_item_get_position (listitem))
    gtk_widget_set_css_classes (button, current);
  else
    gtk_widget_set_css_classes (button, non_current);

}
static void
unbind1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));
  t_modify_set_list_item (modify, NULL);
}

static void
setup2_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_text_new ();
  gtk_list_item_set_child (listitem, text);
}

static void
bind2_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_list_item_get_child (listitem);
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));
  GtkEntryBuffer *buffer;
  GBinding *bind;

  buffer = gtk_text_get_buffer (GTK_TEXT (text));
  gtk_entry_buffer_set_text (buffer, t_modify_look_new_string (modify), -1);
  bind = g_object_bind_property (buffer, "text", modify, "new-string", G_BINDING_BIDIRECTIONAL);
  g_object_set_data (G_OBJECT (listitem), "bind", bind);
}

static void
unbind2_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GBinding *bind = G_BINDING (g_object_get_data (G_OBJECT (listitem), "bind"));

  g_binding_unbind(bind);
  g_object_set_data (G_OBJECT (listitem), "bind", NULL);
}

static void
t_csv_mod_dialog_dispose (GObject *gobject) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (gobject);

  /* this function is available since GTK 4.8 */
  gtk_widget_dispose_template (GTK_WIDGET (mod_dialog), T_TYPE_CSV_MOD_DIALOG);

  G_OBJECT_CLASS (t_csv_mod_dialog_parent_class)->dispose (gobject);
}

static void
t_csv_mod_dialog_init (TCsvModDialog *mod_dialog) {
  gtk_widget_init_template (GTK_WIDGET (mod_dialog));
  mod_dialog->current_position = -1;
}

static void
t_csv_mod_dialog_class_init (TCsvModDialogClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->dispose = t_csv_mod_dialog_dispose;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvmoddialog.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, liststore);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), setup1_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), bind1_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), unbind1_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), setup2_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), bind2_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), unbind2_cb);
}

/* ----- Return list store of modify object ----- */
// Returned value is owned by the caller.
// Returned value can be NULL.
GListStore *
t_csv_mod_dialog_get_list_modify (TCsvModDialog *mod_dialog) {
  return g_object_ref (mod_dialog->liststore);
}

GtkWidget *
t_csv_mod_dialog_new (GtkWindow *win, GListStore *header) {
  g_return_val_if_fail (GTK_IS_WINDOW (win), NULL);
  g_return_val_if_fail (sl_is_tcsv_stringlist (header) || header == NULL, NULL);

  TCsvModDialog *mod_dialog;
  int j, n_items;
  TModify *modify;
  const char *old;

  mod_dialog = T_CSV_MOD_DIALOG (g_object_new (T_TYPE_CSV_MOD_DIALOG, "transient-for", win, NULL));
  if (header) {
    n_items = g_list_model_get_n_items (G_LIST_MODEL (header));
    for (j=0; j<n_items; ++j) {
      old = sl_look_string (header, j);
      modify = t_modify_new_with_data (j, -1, old, old);
      g_list_store_append (mod_dialog->liststore, modify);
      g_object_unref (modify);
    }
  }
  return GTK_WIDGET (mod_dialog);
}
