#include "tcsvflddialog.h"
#include "css.h"
#include "tstr.h"

struct _TCsvFldDialog
{
  GtkDialog parent;
  GtkColumnView *columnview;
  GtkColumnViewColumn *column1;
  GtkColumnViewColumn *column2;
  GtkNoSelection *noselection;
  GListStore *liststore;
  GtkButton *btn_accept;
  GtkButton *btn_cancel;
  GSList *slist;
  GSList *blist;
  GtkWindow *win;
  int position;
};

G_DEFINE_TYPE (TCsvFldDialog, t_csv_fld_dialog, GTK_TYPE_DIALOG);

GListStore *
t_csv_fld_dialog_get_record (TCsvFldDialog *fld_dialog) {
  g_return_val_if_fail (T_IS_CSV_FLD_DIALOG (fld_dialog), NULL);

  GListStore *record;
  int j, n_items;
  TStr *str;
  char *s;

  record = g_list_store_new (T_TYPE_STR);
  n_items = g_list_model_get_n_items (G_LIST_MODEL (fld_dialog->liststore));
  for (j=0; j<n_items; ++j) {
    str = T_STR (g_list_model_get_item (G_LIST_MODEL (fld_dialog->liststore), j));
    /* str is the same object in fld->liststore. it's just g_object_ref */
    /* It is necessary to create new TStr object. not to g_object_ref */
    s = t_str_get_string (str);
    g_object_unref (str);
    str = t_str_new_with_string (s);
    g_free (s);
    g_list_store_append (record, str);
  }
  return record;
}

GtkWindow *
t_csv_fld_dialog_get_window (TCsvFldDialog *fld_dialog) {
  g_return_val_if_fail (T_IS_CSV_FLD_DIALOG (fld_dialog), NULL);

  return fld_dialog->win;
}

static int current_field = -1;

void
ins_cb (GtkButton *btnins) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnins), T_TYPE_CSV_FLD_DIALOG));
  int n_items;
  TStr *str;

  if ((n_items = g_list_model_get_n_items (G_LIST_MODEL (fld_dialog->liststore))) == 0) {
    str = t_str_new_with_string ("");
    g_list_store_append (fld_dialog->liststore, str);
  } else if (current_field < 0 || current_field >= n_items)
    return;
  else {
    str = t_str_new_with_string ("");
    g_list_store_insert (fld_dialog->liststore, current_field, str);
  }
}

void
app_cb (GtkButton *btnapp) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnapp), T_TYPE_CSV_FLD_DIALOG));
  int n_items;
  TStr *str;

  if ((n_items = g_list_model_get_n_items (G_LIST_MODEL (fld_dialog->liststore))) == 0) {
    str = t_str_new_with_string ("");
    g_list_store_append (fld_dialog->liststore, str);
  } else if (current_field < 0 || current_field >= n_items)
    return;
  else {
    str = t_str_new_with_string ("");
    g_list_store_insert (fld_dialog->liststore, current_field + 1, str);
  }
}

void
rm_cb (GtkButton *btnrm) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnrm), T_TYPE_CSV_FLD_DIALOG));
  int n_items;

  if ((n_items = g_list_model_get_n_items (G_LIST_MODEL (fld_dialog->liststore))) == 0)
    return;
  else if (current_field < 0 || current_field >= n_items)
    return;
  else
    g_list_store_remove (fld_dialog->liststore, current_field);
}

void
field_selected_cb (GtkButton *btn) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btn), T_TYPE_CSV_FLD_DIALOG));
  const char *s;
  GSList *slist;
  GtkWidget *button;
  gpointer data;

  s = gtk_button_get_label (btn);
  sscanf (s, "%d", &current_field);
  for (slist = fld_dialog->blist; slist != NULL; ) {
    data = slist->data;
    slist = slist->next;
    if (GTK_IS_BUTTON (data)) {
      button = GTK_WIDGET (data);
      gtk_widget_remove_css_class (button, "selected");
    } else
      fld_dialog->blist = g_slist_remove (fld_dialog->blist, data);
  }
  gtk_widget_add_css_class (GTK_WIDGET (btn), "selected");
}

static void
setup1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (user_data);
  GtkWidget *button = gtk_button_new ();

  gtk_widget_add_css_class (button, "field-number");
  gtk_widget_set_focusable (button, FALSE);
  gtk_list_item_set_child (listitem, button);
  fld_dialog->blist = g_slist_append (fld_dialog->blist, button);
  g_signal_connect (button, "clicked", G_CALLBACK (field_selected_cb), NULL);
}

static void
bind1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *button = gtk_list_item_get_child (listitem);
  TStr *str = T_STR (gtk_list_item_get_item (listitem));
  char *s;

  if (T_IS_STR (str) && GTK_IS_BUTTON (button)) {
    s = g_strdup_printf ("%2d", gtk_list_item_get_position (listitem));
    gtk_button_set_label (GTK_BUTTON (button), s);
    g_free (s);
  }
}

static void
unbind1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *button = gtk_list_item_get_child (listitem);
  TStr *str = T_STR (gtk_list_item_get_item (listitem));

  if (T_IS_STR (str) && GTK_IS_BUTTON (button))
    gtk_button_set_label (GTK_BUTTON (button), "-1");
}

static void
teardown1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (user_data);
  GtkWidget *button = gtk_list_item_get_child (listitem);

  if (GTK_IS_BUTTON (button)) {
    fld_dialog->blist = g_slist_remove (fld_dialog->blist, button);
    gtk_list_item_set_child (listitem, NULL);
  }
}

typedef struct _Handlertbl Handlertbl;
struct _Handlertbl {
  GtkListItem *listitem;
  gulong handler_id;
};

static void
text_activate_cb (GtkText *text, gpointer user_data) {
  TStr *str = T_STR (user_data);
  GtkEntryBuffer *buffer = gtk_text_get_buffer (text);

  t_str_set_string (str, gtk_entry_buffer_get_text (buffer));
}

static void
setup_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_text_new ();

  gtk_list_item_set_child (listitem, text);
}

static void
bind_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (user_data);
  GtkWidget *text = gtk_list_item_get_child (listitem);
  TStr *str = T_STR (gtk_list_item_get_item (listitem));
  GtkEntryBuffer *buffer;
  char *s;
  Handlertbl *table;

  if (T_IS_STR (str)) {
    buffer = gtk_text_get_buffer (GTK_TEXT (text));
    s = t_str_get_string (str);
    if (s) {
      gtk_entry_buffer_set_text (buffer, s, -1);
      g_free (s);
    } else
      gtk_entry_buffer_set_text (buffer, "", -1);
    table = g_new (Handlertbl, 1);
    table->listitem = listitem;
    table->handler_id = g_signal_connect (GTK_TEXT (text), "activate", G_CALLBACK (text_activate_cb), str);
    fld_dialog->slist = g_slist_append (fld_dialog->slist, table);
  }
}

static void
unbind_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (user_data);
  GtkWidget *text = gtk_list_item_get_child (listitem);
  GSList *slist;
  Handlertbl *table;

  for (slist = fld_dialog->slist; slist != NULL; slist = slist->next) {
    table = (Handlertbl *) slist->data;
    if (table->listitem == listitem) {
      g_signal_handler_disconnect (GTK_TEXT (text), table->handler_id);
      break;
    }
  }
  if (slist != NULL) {
    fld_dialog->slist = g_slist_remove (fld_dialog->slist, table);
    g_free (table);
  }
}

static void
teardown_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  gtk_list_item_set_child (listitem, NULL);
}

static void
t_csv_fld_dialog_finalize (GObject *object) {
  TCsvFldDialog *fld_dialog = T_CSV_FLD_DIALOG (object);

  if (fld_dialog->slist)
    g_slist_free_full (fld_dialog->slist, g_free);
  G_OBJECT_CLASS (t_csv_fld_dialog_parent_class)->finalize (object);
}

static void
t_csv_fld_dialog_init (TCsvFldDialog *fld_dialog) {
  gtk_widget_init_template (GTK_WIDGET (fld_dialog));
  fld_dialog->slist = fld_dialog->blist =  NULL;
  GtkListItemFactory *factory;
  factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (fld_dialog->column1, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup1_cb), fld_dialog);
  g_signal_connect (factory, "bind", G_CALLBACK (bind1_cb), NULL);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind1_cb), NULL);
  g_signal_connect (factory, "teardown", G_CALLBACK (teardown1_cb), fld_dialog);
  factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (fld_dialog->column2, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup_cb), NULL);
  g_signal_connect (factory, "bind", G_CALLBACK (bind_cb), fld_dialog);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind_cb), fld_dialog);
  g_signal_connect (factory, "teardown", G_CALLBACK (teardown_cb), NULL);
  set_css_for_display (GTK_WINDOW (fld_dialog),
  "text:focus {border: 1px solid gray;} button.field-number {color: black; background: #e8e8e8;} button.selected {background: lightblue;}");
}

static void
t_csv_fld_dialog_class_init (TCsvFldDialogClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_csv_fld_dialog_finalize;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvflddialog.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvFldDialog, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvFldDialog, column1);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvFldDialog, column2);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvFldDialog, noselection);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvFldDialog, liststore);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvFldDialog, btn_accept);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvFldDialog, btn_cancel);
}

GtkWidget *
t_csv_fld_dialog_new (GtkWindow *win) {
  g_return_val_if_fail (GTK_IS_WINDOW (win), NULL);
  GtkWidget *fld_dialog;

  fld_dialog = GTK_WIDGET (g_object_new (T_TYPE_CSV_FLD_DIALOG, "transient-for", win, NULL));
  (T_CSV_FLD_DIALOG (fld_dialog))->win = win;

  return fld_dialog;
}

