#include "tcsvmoddialog.h"
#include "css.h"
#include "tstr.h"

/* ----- Start of the definition of TModify object -----*/
enum {
  PROP_0,
  PROP_POSITION,
  PROP_OLD,
  PROP_NEW,
  N_PROPERTIES
};

static GParamSpec *modify_properties[N_PROPERTIES] = {NULL, };

typedef struct _TModify {
  GObject parent;
  int position;
  char *old;
  char *new;
  GtkEntryBuffer *buffer;
} TModify;

G_DEFINE_TYPE(TModify, t_modify, G_TYPE_OBJECT)

static void
t_modify_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  TModify *self = T_MODIFY (object);

  if (property_id == PROP_POSITION)
    self->position = g_value_get_int (value);
  else if (property_id == PROP_OLD)
    self->old = g_strdup (g_value_get_string (value));
  else if (property_id == PROP_NEW)
    self->new = g_strdup (g_value_get_string (value));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_modify_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  TModify *self = T_MODIFY (object);

  if (property_id == PROP_POSITION)
    g_value_set_int (value, self->position);
  else if (property_id == PROP_OLD)
    g_value_set_string (value, g_strdup (self->old));
  else if (property_id == PROP_NEW)
    g_value_set_string (value, g_strdup (self->new));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_modify_init (TModify *self) {
  self->position = -1;
  self->old = self->new = NULL;
  self->buffer = NULL;
}

static void
t_modify_finalize (GObject *object) {
  TModify *self = T_MODIFY (object);

  if (self->old)
    g_clear_object (&self->old);
  if (self->new)
    g_clear_object (&self->new);
  G_OBJECT_CLASS (t_modify_parent_class)->finalize (object);
}

static void
t_modify_class_init (TModifyClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_modify_finalize;
  gobject_class->set_property = t_modify_set_property;
  gobject_class->get_property = t_modify_get_property;
  modify_properties[PROP_POSITION] = g_param_spec_int ("position", "pos", "old field position", -1, G_MAXINT, -1, G_PARAM_READWRITE);
  modify_properties[PROP_OLD] = g_param_spec_string ("old-field", "old", "old field", "", G_PARAM_READWRITE);
  modify_properties[PROP_NEW] = g_param_spec_string ("new-field", "new", "new field", "", G_PARAM_READWRITE);
  g_object_class_install_properties (gobject_class,N_PROPERTIES, modify_properties);
}

/* setter and getter */
void
t_modify_set_position (TModify *self, int pos) {
  g_object_set (self, "position", pos, NULL);
}

void
t_modify_set_old_field (TModify *self, const char *old) {
  g_object_set (self, "old-field", g_strdup (old), NULL);
}

void
t_modify_set_new_field (TModify *self, const char *new) {
  g_object_set (self, "new-field", g_strdup (new), NULL);
}

int
t_modify_get_position (TModify *self) {
  return self->position;
}

char *
t_modify_get_old_field (TModify *self) {
  return g_strdup (self->old);
}

char *
t_modify_get_new_field (TModify *self) {
  return g_strdup (self->new);
}

void
t_modify_set_buffer (TModify *self, GtkEntryBuffer *buffer) {
  g_return_if_fail (T_IS_MODIFY (self));
  g_return_if_fail (GTK_IS_ENTRY_BUFFER (buffer) || buffer == NULL);

  self->buffer = buffer;
}

GtkEntryBuffer *
t_modify_get_buffer (TModify *self) {
  g_return_val_if_fail (T_IS_MODIFY (self), NULL);

  return self->buffer;
}

/* create a new TModify instance */
TModify *
t_modify_new_with_data (int pos, char *old, char *new) {
  return T_MODIFY (g_object_new (T_TYPE_MODIFY, "position", pos, "old-field", old, "new-field", new, NULL));
}

TModify *
t_modify_new (void) {
  return T_MODIFY (g_object_new (T_TYPE_MODIFY, NULL));
}

/* ----- End of the definition of TModify object -----*/

struct _TCsvModDialog
{
  GtkDialog parent;
  GtkColumnView *columnview;
  GtkColumnViewColumn *column1;
  GtkColumnViewColumn *column4;
  GtkNoSelection *noselection;
  GListStore *liststore;
  GtkButton *btn_accept;
  GtkButton *btn_cancel;
  GSList *blist;
  GtkWindow *win;
};

G_DEFINE_TYPE (TCsvModDialog, t_csv_mod_dialog, GTK_TYPE_DIALOG);

GSList *
t_csv_mod_dialog_get_table (TCsvModDialog *mod_dialog) {
  GSList *table;
  new_field_config *config;
  int i, n_items;
  TModify *modify;
  char *new;

  table = NULL;
  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  for (i=0; i<n_items; ++i) {
    modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), i));
    new = t_modify_get_new_field (modify);
    if (strcmp (new, "") != 0) {
      config = g_new (new_field_config, 1);
      config->header = new;
      config->old_position = t_modify_get_position (modify);
      table = g_slist_append (table, config);
    } else
      g_free (new);
    g_object_unref (modify);
  }
  return table;;
}

GtkWindow *
t_csv_mod_dialog_get_window (TCsvModDialog *mod_dialog) {
  return mod_dialog->win;
}

/* "Clicked" signal handlers */
static int current_field = -1;

void
mod_ins_cb (GtkButton *btnins) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnins), T_TYPE_CSV_MOD_DIALOG));
  TModify *modify;
  int n_items;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  if (current_field >= n_items)
    return;
  modify = t_modify_new_with_data (-1, "", "");
  if (n_items  == 0)
    g_list_store_append (mod_dialog->liststore, modify);
  else if (current_field < 0)
    g_list_store_insert (mod_dialog->liststore, 0, modify);
  else
    g_list_store_insert (mod_dialog->liststore, current_field, modify);
}

void
mod_app_cb (GtkButton *btnapp) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnapp), T_TYPE_CSV_MOD_DIALOG));
  TModify *modify;
  int n_items;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  if (current_field >= n_items)
    return;
  modify = t_modify_new_with_data (-1, "", "");
  if (n_items  == 0 || current_field < 0)
    g_list_store_append (mod_dialog->liststore, modify);
  else
    g_list_store_insert (mod_dialog->liststore, current_field + 1, modify);
}

void
mod_rm_cb (GtkButton *btnrm) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btnrm), T_TYPE_CSV_MOD_DIALOG));
  TModify *modify;
  char *old;
  char *new;
  int n_items;
  GtkEntryBuffer *buffer;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (mod_dialog->liststore));
  if (n_items == 0 || current_field < 0 || current_field >= n_items)
    return;
  modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (mod_dialog->liststore), current_field));
  if (! T_MODIFY (modify))
    return;
  old = t_modify_get_old_field (modify);
  new = t_modify_get_new_field (modify);
  if (strcmp (old, "") != 0) {
    t_modify_set_new_field (modify, "");
    buffer = t_modify_get_buffer (modify);
    if (GTK_IS_ENTRY_BUFFER (buffer))
      gtk_entry_buffer_set_text (buffer, "", -1);
  } else
    g_list_store_remove (mod_dialog->liststore, current_field);
  g_free (old);
  g_free (new);
  g_object_unref (modify);
}

static void
mod_field_selected_cb (GtkButton *btn, gpointer user_data) {
  GtkListItem *listitem = GTK_LIST_ITEM (user_data);
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (btn), T_TYPE_CSV_MOD_DIALOG));
  GSList *slist;
  GtkWidget *button;
  gpointer data;

  current_field = gtk_list_item_get_position (listitem);
  for (slist = mod_dialog->blist; slist != NULL; ) {
    data = slist->data;
    slist = slist->next;
    if (GTK_IS_BUTTON (data)) {
      button = GTK_WIDGET (data);
      gtk_widget_remove_css_class (button, "selected");
    } else
      mod_dialog->blist = g_slist_remove (mod_dialog->blist, data);
  }
  gtk_widget_add_css_class (GTK_WIDGET (btn), "selected");
}

static void
setup1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  GtkWidget *button = gtk_button_new ();

  gtk_widget_add_css_class (button, "field-number");
  gtk_widget_set_focusable (button, FALSE);
  gtk_list_item_set_child (listitem, button);
  mod_dialog->blist = g_slist_append (mod_dialog->blist, button);
  g_signal_connect (button, "clicked", G_CALLBACK (mod_field_selected_cb), listitem);
}

static void
bind1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *button = gtk_list_item_get_child (listitem);
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));
  char *s;
  int pos;

  if (T_IS_MODIFY (modify) && GTK_IS_BUTTON (button)) {
    pos = t_modify_get_position (modify);
    if (pos >= 0)
      s = g_strdup_printf ("%2d", pos);
    else
      s = g_strdup ("");
    gtk_button_set_label (GTK_BUTTON (button), s);
    g_free (s);
  }
}

static void
unbind1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
}

static void
teardown1_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (user_data);
  GtkWidget *button = gtk_list_item_get_child (listitem);

  if (GTK_IS_BUTTON (button)) {
    mod_dialog->blist = g_slist_remove (mod_dialog->blist, button);
    gtk_list_item_set_child (listitem, NULL);
  }
}

char *
get_state (GtkListItem *listitem, char *old, char *new) {
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));

  if (! T_IS_MODIFY (modify) || old == NULL || new == NULL)
    return g_strdup ("");
  if (strcmp (old, "") == 0 && strcmp (new, "") != 0)
    return g_strdup ("Add");
  else if (strcmp (old, "") != 0 && strcmp (new, "") == 0)
    return g_strdup ("Remove");
  else if (strcmp (old, "") != 0 && strcmp (new, "") != 0 && strcmp (old, new) != 0)
    return g_strdup ("Changed");
  else
    return g_strdup ("");
}

static void
mod_deleted_text_cb (GtkEntryBuffer *buffer, guint position, guint n_chars, gpointer user_data) {
  TModify *modify = T_MODIFY (user_data);

  t_modify_set_new_field (modify, gtk_entry_buffer_get_text (buffer));
}

static void
mod_inserted_text_cb (GtkEntryBuffer *buffer, guint position, char *chars, guint n_chars, gpointer user_data) {
  TModify *modify = T_MODIFY (user_data);

  t_modify_set_new_field (modify, gtk_entry_buffer_get_text (buffer));
}

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
  char *new;

  if (T_IS_MODIFY (modify)) {
    buffer = gtk_text_get_buffer (GTK_TEXT (text));
    new = t_modify_get_new_field (modify);
    gtk_entry_buffer_set_text (buffer, new, -1);
    g_free (new);

    t_modify_set_buffer (modify, buffer);
    g_signal_connect_after (buffer, "deleted-text", G_CALLBACK (mod_deleted_text_cb), modify);
    g_signal_connect (buffer, "inserted-text", G_CALLBACK (mod_inserted_text_cb), modify);
  }
}

static void
unbind4_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_list_item_get_child (listitem);
  GtkEntryBuffer *buffer = gtk_text_get_buffer (GTK_TEXT (text));
  TModify *modify = T_MODIFY (gtk_list_item_get_item (listitem));

  if (T_IS_MODIFY (modify)) {
    t_modify_set_buffer (modify, NULL);
    g_signal_handlers_disconnect_by_func (buffer, mod_deleted_text_cb, modify);
    g_signal_handlers_disconnect_by_func (buffer, mod_inserted_text_cb, modify);
  }
}

static void
teardown4_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  gtk_list_item_set_child (listitem, NULL);
}

static void
t_csv_mod_dialog_finalize (GObject *object) {
  TCsvModDialog *mod_dialog = T_CSV_MOD_DIALOG (object);

  if (mod_dialog->blist)
    g_slist_free (mod_dialog->blist);
  G_OBJECT_CLASS (t_csv_mod_dialog_parent_class)->finalize (object);
}

static void
t_csv_mod_dialog_init (TCsvModDialog *mod_dialog) {
  gtk_widget_init_template (GTK_WIDGET (mod_dialog));
  GtkListItemFactory *factory;

  mod_dialog->blist =  NULL;
  factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (mod_dialog->column1, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup1_cb), mod_dialog);
  g_signal_connect (factory, "bind", G_CALLBACK (bind1_cb), NULL);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind1_cb), NULL);
  g_signal_connect (factory, "teardown", G_CALLBACK (teardown1_cb), mod_dialog);
  factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (mod_dialog->column4, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup4_cb), NULL);
  g_signal_connect (factory, "bind", G_CALLBACK (bind4_cb), NULL);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind4_cb), NULL);
  g_signal_connect (factory, "teardown", G_CALLBACK (teardown4_cb), NULL);
  set_css_for_display (GTK_WINDOW (mod_dialog),
  "text:focus {border: 1px solid gray;} button.field-number {color: black; background: #e8e8e8;} button.selected {background: lightblue;}");

}

static void
t_csv_mod_dialog_class_init (TCsvModDialogClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_csv_mod_dialog_finalize;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvmoddialog.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, column1);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, column4);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, noselection);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, liststore);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, btn_accept);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvModDialog, btn_cancel);
}

GtkWidget *
t_csv_mod_dialog_new (GtkWindow *win, GListStore *header) {
  g_return_val_if_fail (GTK_IS_WINDOW (win), NULL);
  g_return_val_if_fail (G_IS_LIST_STORE (header), NULL);

  TCsvModDialog *mod_dialog;
  int j, n_items;
  TModify *modify;
  TStr *str;
  char *old;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (header));
  if (n_items == 0)
    return NULL;

  mod_dialog = T_CSV_MOD_DIALOG (g_object_new (T_TYPE_CSV_MOD_DIALOG, "transient-for", win, NULL));
  for (j=0; j<n_items; ++j) {
    str = T_STR (g_list_model_get_item (G_LIST_MODEL (header), j));
    old = t_str_get_string (str);
    modify = t_modify_new_with_data (j, old, old);
    g_list_store_append (mod_dialog->liststore, modify);
    g_free (old);
    g_object_unref (str);
  }
  mod_dialog->win = win;

  return GTK_WIDGET (mod_dialog);
}

