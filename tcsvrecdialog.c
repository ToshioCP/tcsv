#include "tcsvrecdialog.h"
#include "tmodify.h"

struct _TCsvRecDialog
{
  GtkDialog parent;
  GtkColumnViewColumn *column2;
  GListStore *liststore;
  int position;
};

G_DEFINE_TYPE (TCsvRecDialog, t_csv_rec_dialog, GTK_TYPE_DIALOG);

GtkStringList *
t_csv_rec_dialog_get_record (TCsvRecDialog *rec_dialog) {
  g_return_val_if_fail (T_IS_CSV_REC_DIALOG (rec_dialog), NULL);

  GtkStringList *record;
  int i, n_items;
  TModify *modify;

  record = gtk_string_list_new (NULL);
  n_items = g_list_model_get_n_items (G_LIST_MODEL (rec_dialog->liststore));
  for (i=0; i<n_items; ++i) {
    modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (rec_dialog->liststore), i));
    gtk_string_list_append (record, t_modify_look_new_string (modify));
    g_object_unref (modify);
  }
  return record;
}

int
t_csv_rec_dialog_get_position (TCsvRecDialog *rec_dialog) {
  g_return_val_if_fail (T_IS_CSV_REC_DIALOG (rec_dialog), -1);

  return rec_dialog->position;
}

static void
setup_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_text_new ();
  gtk_list_item_set_child (listitem, text);
}

static void
bind_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_list_item_get_child (listitem);
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));
  GtkEntryBuffer *buffer;
  GBinding *bind;

  if (! modify)
    return;
  buffer = gtk_text_get_buffer (GTK_TEXT (text));
  gtk_entry_buffer_set_text (buffer, t_modify_look_new_string (modify), -1);
  bind = g_object_bind_property (buffer, "text", modify, "new-string", G_BINDING_DEFAULT);
  g_object_set_data (G_OBJECT (listitem), "bind", bind);
}

static void
unbind_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GBinding *bind = G_BINDING (g_object_get_data (G_OBJECT (listitem), "bind"));
  if (G_IS_BINDING (bind)) {
    g_binding_unbind(bind);
    g_object_set_data (G_OBJECT (listitem), "bind", NULL);
  }
}

static void
t_csv_rec_dialog_init (TCsvRecDialog *rec_dialog) {
  gtk_widget_init_template (GTK_WIDGET (rec_dialog));

  GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (rec_dialog->column2, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup_cb), NULL);
  g_signal_connect (factory, "bind", G_CALLBACK (bind_cb), NULL);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind_cb), NULL);
}

static void
t_csv_rec_dialog_class_init (TCsvRecDialogClass *class) {
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvrecdialog.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, column2);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, liststore);
}

GtkWidget *
t_csv_rec_dialog_new (GtkWindow *win, GtkStringList *header, GtkStringList *record, int position) {
  g_return_val_if_fail (GTK_IS_WINDOW (win) || win == NULL, NULL);
  g_return_val_if_fail (GTK_IS_STRING_LIST (header), NULL);
  g_return_val_if_fail (GTK_IS_STRING_LIST (record), NULL);

  TCsvRecDialog *rec_dialog;
  int j, n_items;
  TModify *modify;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (header));
  if (n_items != g_list_model_get_n_items (G_LIST_MODEL (record)))
    return NULL;

  T_TYPE_MODIFY;
  rec_dialog = T_CSV_REC_DIALOG (g_object_new (T_TYPE_CSV_REC_DIALOG, "transient-for", win, NULL));
  for (j=0; j<n_items; ++j) {
    modify = t_modify_new_with_data (-1, -1, gtk_string_list_get_string (header, j), gtk_string_list_get_string (record, j));
    g_list_store_append (rec_dialog->liststore, modify);
    g_object_unref (modify);
  }
  rec_dialog->position = position;
  return GTK_WIDGET (rec_dialog);
}
