#include "tcsvnewdialog.h"
#include "css.h"
#include "tstr.h"


/* ----- Start of the definition of TNewf object -----*/
#define T_TYPE_NEWF (t_newf_get_type ())
G_DECLARE_FINAL_TYPE (TNewf, t_newf, T, NEWF, GObject)

typedef struct _TNewf {
  GObject parent;
  char *field;
  GtkText *text;
} TNewf;

G_DEFINE_TYPE(TNewf, t_newf, G_TYPE_OBJECT)

static void
t_newf_init (TNewf *self) {
  self->field = NULL;
  self->text = NULL;
}

static void
t_newf_finalize (GObject *object) {
  TNewf *self = T_NEWF (object);

  if (self->field)
    g_clear_object (&self->field);
  G_OBJECT_CLASS (t_newf_parent_class)->finalize (object);
}

static void
t_newf_class_init (TNewfClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_newf_finalize;
}

/* setter and getter */
void
t_newf_set_field (TNewf *self, const char *s) {
  g_return_if_fail (T_IS_NEWF (self));

  if (self->field)
    g_free (self->field);
  if (s)
    self->field = g_strdup (s);
  else
    self->field = NULL;
}

void
t_newf_set_text (TNewf *self, GtkText *text) {
  g_return_if_fail (T_IS_NEWF (self));
  g_return_if_fail (GTK_IS_TEXT (text) || text == NULL);

  self->text = text;
}

char *
t_newf_look_field (TNewf *self) {
  g_return_val_if_fail (T_IS_NEWF (self), NULL);

  return self->field;
}

char *
t_newf_get_field (TNewf *self) {
  g_return_val_if_fail (T_IS_NEWF (self), NULL);

  if (self->field)
    return g_strdup (self->field);
  else
    return NULL;
}

GtkText *
t_newf_get_text (TNewf *self) {
  g_return_val_if_fail (T_IS_NEWF (self), NULL);

  return self->text;
}

/* create a new TNewf instance */
TNewf *
t_newf_new_with_field (char *s) {
  TNewf *newf = T_NEWF (g_object_new (T_TYPE_NEWF, NULL));

  t_newf_set_field (newf, s);
  return newf;
}

TNewf *
t_newf_new (void) {
  return T_NEWF (g_object_new (T_TYPE_NEWF, NULL));
}

/* ----- End of the definition of TNewf object -----*/


struct _TCsvNewDialog
{
  GtkDialog parent;
  GtkColumnView *columnview;
  GtkColumnViewColumn *column1;
  GtkColumnViewColumn *column2;
  GtkNoSelection *noselection;
  GListStore *liststore;
  GtkButton *btn_accept;
  GtkButton *btn_cancel;
  GSList *blist;
  GtkWindow *win;
  int current_field;
};

G_DEFINE_TYPE (TCsvNewDialog, t_csv_new_dialog, GTK_TYPE_DIALOG);

GListStore *
t_csv_new_dialog_get_record (TCsvNewDialog *new_dialog) {
  g_return_val_if_fail (T_IS_CSV_NEW_DIALOG (new_dialog), NULL);

  GListStore *record;
  int i, n_items;
  TNewf *newf;
  TStr *str;

  record = g_list_store_new (T_TYPE_STR);
  n_items = g_list_model_get_n_items (G_LIST_MODEL (new_dialog->liststore));
  for (i=0; i<n_items; ++i) {
    newf = T_NEWF (g_list_model_get_item (G_LIST_MODEL (new_dialog->liststore), i));
    str = t_str_new_with_string (t_newf_look_field (newf));
    g_object_unref (newf);
    g_list_store_append (record, str);
  }
  return record;
}

GtkWindow *
t_csv_new_dialog_get_window (TCsvNewDialog *new_dialog) {
  g_return_val_if_fail (T_IS_CSV_NEW_DIALOG (new_dialog), NULL);

  return new_dialog->win;
}

static void
color_button (TCsvNewDialog *new_dialog) {
  GSList *slist;
  gpointer data;
  GtkWidget *button;
  int number;
  const char *s;

  if (new_dialog->current_field < 0)
    return;
  for (slist = new_dialog->blist; slist != NULL; ) {
    data = slist->data;
    slist = slist->next;
    if (! GTK_IS_BUTTON (data))
      continue;
    button = GTK_WIDGET (data);
    s = gtk_button_get_label (GTK_BUTTON (button));
    if (s == NULL || sscanf (s, "%d", &number) == EOF)
      continue;
    if (number == new_dialog->current_field && ! gtk_widget_has_css_class (button, "selected"))
      gtk_widget_add_css_class (button, "selected");
    else if  (number != new_dialog->current_field && gtk_widget_has_css_class (button, "selected"))
      gtk_widget_remove_css_class (button, "selected");
  }
}

void
ins_cb (GtkButton *btnins) {
  TCsvNewDialog *new_dialog = T_CSV_NEW_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnins), T_TYPE_CSV_NEW_DIALOG));
  int n_items;
  TNewf *newf;
  GtkText *text;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (new_dialog->liststore));
  if (new_dialog->current_field >= n_items)
    return;
  newf = t_newf_new_with_field ("");
  if (n_items == 0 || new_dialog->current_field < 0) {
    g_list_store_insert (new_dialog->liststore, 0, newf);
    new_dialog->current_field = 0;
  } else
    g_list_store_insert (new_dialog->liststore, new_dialog->current_field, newf);
  color_button (new_dialog);
  if (GTK_IS_WIDGET (text = t_newf_get_text (newf)))
    gtk_widget_grab_focus (GTK_WIDGET (text));
}

void
app_cb (GtkButton *btnapp) {
  TCsvNewDialog *new_dialog = T_CSV_NEW_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnapp), T_TYPE_CSV_NEW_DIALOG));
  int n_items;
  TNewf *newf;
  GtkText *text;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (new_dialog->liststore));
  if (new_dialog->current_field >= n_items)
    return;
  newf = t_newf_new_with_field ("");
  if (n_items == 0 || new_dialog->current_field < 0) {
    g_list_store_append (new_dialog->liststore, newf);
    new_dialog->current_field = n_items;
  } else {
    g_list_store_insert (new_dialog->liststore, new_dialog->current_field + 1, newf);
    ++new_dialog->current_field;
  }
  color_button (new_dialog);
  if (GTK_IS_WIDGET (text = t_newf_get_text (newf)))
    gtk_widget_grab_focus (GTK_WIDGET (text));
}

void
rm_cb (GtkButton *btnrm) {
  TCsvNewDialog *new_dialog = T_CSV_NEW_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnrm), T_TYPE_CSV_NEW_DIALOG));
  int n_items;

  if ((n_items = g_list_model_get_n_items (G_LIST_MODEL (new_dialog->liststore))) == 0)
    return;
  else if (new_dialog->current_field < 0 || new_dialog->current_field >= n_items)
    return;
  else {
    g_list_store_remove (new_dialog->liststore, new_dialog->current_field);
    new_dialog->current_field = -1;
  }    
  /* no need to call color_button */
}

void
field_selected_cb (GtkButton *btn) {
  TCsvNewDialog *new_dialog = T_CSV_NEW_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btn), T_TYPE_CSV_NEW_DIALOG));
  const char *s;
  TNewf *newf;
  GtkText *text;

  s = gtk_button_get_label (btn);
  if (sscanf (s, "%d", &new_dialog->current_field) == EOF)
    return;
  color_button (new_dialog);
  if (new_dialog->current_field < 0 || new_dialog->current_field >= g_list_model_get_n_items (G_LIST_MODEL (new_dialog->liststore)))
    return;
  newf = T_NEWF (g_list_model_get_item (G_LIST_MODEL (new_dialog->liststore), new_dialog->current_field));
  if (GTK_IS_WIDGET (text = t_newf_get_text (newf)))
    gtk_widget_grab_focus (GTK_WIDGET (text));
}

static void
setup1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvNewDialog *new_dialog = T_CSV_NEW_DIALOG (user_data);
  GtkWidget *button = gtk_button_new ();

  gtk_widget_add_css_class (button, "field-number");
  gtk_widget_set_focusable (button, FALSE);
  gtk_list_item_set_child (listitem, button);
  new_dialog->blist = g_slist_append (new_dialog->blist, button);
  g_signal_connect (button, "clicked", G_CALLBACK (field_selected_cb), NULL);
}

static void
bind1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *button = gtk_list_item_get_child (listitem);
  TNewf *newf = T_NEWF (gtk_list_item_get_item (listitem));
  char *s;

  if (T_IS_NEWF (newf)) {
    s = g_strdup_printf ("%2d", gtk_list_item_get_position (listitem));
    gtk_button_set_label (GTK_BUTTON (button), s);
    g_free (s);
  }
}

static void
unbind1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
}

static void
teardown1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvNewDialog *new_dialog = T_CSV_NEW_DIALOG (user_data);
  GtkWidget *button = gtk_list_item_get_child (listitem);

  if (GTK_IS_BUTTON (button)) {
    new_dialog->blist = g_slist_remove (new_dialog->blist, button);
    gtk_list_item_set_child (listitem, NULL);
  }
}

static void
new_deleted_text_cb (GtkEntryBuffer *buffer, guint position, guint n_chars, gpointer user_data) {
  TNewf *newf = T_NEWF (user_data);

  t_newf_set_field (newf, gtk_entry_buffer_get_text (buffer));
}

static void
new_inserted_text_cb (GtkEntryBuffer *buffer, guint position, char *chars, guint n_chars, gpointer user_data) {
  TNewf *newf = T_NEWF (user_data);

  t_newf_set_field (newf, gtk_entry_buffer_get_text (buffer));
}

static void
setup2_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_text_new ();

  gtk_list_item_set_child (listitem, text);
}

static void
bind2_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_list_item_get_child (listitem);
  GtkEntryBuffer *buffer = gtk_text_get_buffer (GTK_TEXT (text));
  TNewf *newf = T_NEWF (gtk_list_item_get_item (listitem));
  char *s;

  if (T_IS_NEWF (newf)) {
    s = t_newf_look_field (newf);
    if (s)
      gtk_entry_buffer_set_text (buffer, s, -1);
    else
      gtk_entry_buffer_set_text (buffer, "", -1);
    g_signal_connect_after (buffer, "deleted-text", G_CALLBACK (new_deleted_text_cb), newf);
    g_signal_connect (buffer, "inserted-text", G_CALLBACK (new_inserted_text_cb), newf);
    t_newf_set_text (newf, GTK_TEXT (text));
  }
}

static void
unbind2_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_list_item_get_child (listitem);
  GtkEntryBuffer *buffer = gtk_text_get_buffer (GTK_TEXT (text));
  TNewf *newf = T_NEWF (gtk_list_item_get_item (listitem));

  if (T_IS_NEWF (newf)) {
    g_signal_handlers_disconnect_by_func (buffer, new_deleted_text_cb, newf);
    g_signal_handlers_disconnect_by_func (buffer, new_inserted_text_cb, newf);
    t_newf_set_text (newf, NULL);
  }
}

static void
teardown2_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  gtk_list_item_set_child (listitem, NULL);
}

static void
t_csv_new_dialog_finalize (GObject *object) {
  TCsvNewDialog *new_dialog = T_CSV_NEW_DIALOG (object);

  if (new_dialog->blist)
    g_slist_free (new_dialog->blist);
  G_OBJECT_CLASS (t_csv_new_dialog_parent_class)->finalize (object);
}

static void
t_csv_new_dialog_init (TCsvNewDialog *new_dialog) {
  gtk_widget_init_template (GTK_WIDGET (new_dialog));
  GtkListItemFactory *factory;

  new_dialog->liststore = g_list_store_new (T_TYPE_NEWF);
  gtk_no_selection_set_model (new_dialog->noselection, G_LIST_MODEL (new_dialog->liststore));
  new_dialog->blist =  NULL;
  factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (new_dialog->column1, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup1_cb), new_dialog);
  g_signal_connect (factory, "bind", G_CALLBACK (bind1_cb), NULL);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind1_cb), NULL);
  g_signal_connect (factory, "teardown", G_CALLBACK (teardown1_cb), new_dialog);
  factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (new_dialog->column2, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup2_cb), NULL);
  g_signal_connect (factory, "bind", G_CALLBACK (bind2_cb), NULL);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind2_cb), NULL);
  g_signal_connect (factory, "teardown", G_CALLBACK (teardown2_cb), NULL);
  new_dialog->current_field = -1;
  set_css_for_display (GTK_WINDOW (new_dialog),
  "text:focus {border: 1px solid gray;} button.field-number {color: black; background: #e8e8e8;} button.selected {background: lightblue;}");
}

static void
t_csv_new_dialog_class_init (TCsvNewDialogClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_csv_new_dialog_finalize;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvnewdialog.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvNewDialog, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvNewDialog, column1);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvNewDialog, column2);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvNewDialog, noselection);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvNewDialog, btn_accept);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvNewDialog, btn_cancel);
}

GtkWidget *
t_csv_new_dialog_new (GtkWindow *win) {
  g_return_val_if_fail (GTK_IS_WINDOW (win), NULL);
  GtkWidget *new_dialog;

  new_dialog = GTK_WIDGET (g_object_new (T_TYPE_CSV_NEW_DIALOG, "transient-for", win, NULL));
  (T_CSV_NEW_DIALOG (new_dialog))->win = win;

  return new_dialog;
}

