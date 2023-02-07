#include <gtk/gtk.h>
#include "tcsvwindow.h"
#include "tcsvalert.h"
#include "tcsv.h"
#include "tcsvrecdialog.h"
#include "tcsvmoddialog.h"
#include "tmodify.h"
#include "tcsvstr.h"
#include "tcsvstringlist.h"

/* Deinition of TCsvRecord */
/* It includes a pointer to a GtkListItem and Gliststore */

#define T_TYPE_CSV_RECORD  (t_csv_record_get_type ())
G_DECLARE_FINAL_TYPE (TCsvRecord, t_csv_record, T, CSV_RECORD, GObject)

struct _TCsvRecord {
  GObject parent;
  GtkListItem *listitem;
  GListStore *liststore; /* tcsv_stringlist */
};

G_DEFINE_TYPE (TCsvRecord, t_csv_record, G_TYPE_OBJECT)

/* setter and getter */
void
t_csv_record_set_list_item (TCsvRecord *self, GtkListItem *listitem) {
  g_return_if_fail (T_IS_CSV_RECORD (self));
  g_return_if_fail (GTK_IS_LIST_ITEM (listitem) || listitem == NULL);
  if (self->listitem)
    g_object_unref (self->listitem);
  self->listitem = listitem ? g_object_ref (listitem) : NULL;
}

void
t_csv_record_set_list_store (TCsvRecord *self, GListStore *liststore) {
  g_return_if_fail (T_IS_CSV_RECORD (self));
  g_return_if_fail (sl_is_tcsv_stringlist (liststore) || liststore == NULL);
  if (self->liststore)
    g_object_unref (self->liststore);
  self->liststore = liststore ? g_object_ref (liststore) : NULL;
}

/* The return value is owned by the caller */
GtkListItem *
t_csv_record_get_list_item (TCsvRecord *self) {
  g_return_val_if_fail (T_IS_CSV_RECORD (self), NULL);
  return self->listitem ? g_object_ref (self->listitem) : NULL;
}

/* The return value is owned by the caller */
GListStore *
t_csv_record_get_list_store (TCsvRecord *self) {
  g_return_val_if_fail (T_IS_CSV_RECORD (self), NULL);
  return self->liststore ? g_object_ref (self->liststore) : NULL;
}

// This function interchanges the strings (properties) of the two records.
// It is used when you move up or down an item in the list.
// It is better than remove and insert because GtkColumnView scrolls the display.
void
t_csv_record_swap_strings (TCsvRecord *self, TCsvRecord *other) {
  g_return_if_fail (T_IS_CSV_RECORD (self));
  g_return_if_fail (T_IS_CSV_RECORD (other));
  GListStore *other_liststore = t_csv_record_get_list_store(other);

  sl_swap_strings (self->liststore, other_liststore);
}

static void
t_csv_record_dispose (GObject *object) {
  TCsvRecord *self = T_CSV_RECORD (object);
  if (self->listitem)
    g_clear_object (&self->listitem);
  if (self->liststore)
    g_clear_object (&self->liststore);
  G_OBJECT_CLASS (t_csv_record_parent_class)->dispose (object);
}

static void
t_csv_record_init (TCsvRecord *self) {
  self->listitem = NULL;
  self->liststore = NULL;
}

static void
t_csv_record_class_init (TCsvRecordClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->dispose = t_csv_record_dispose;
}

/* create a new TCsvRecord instance */
TCsvRecord *
t_csv_record_new_with_data (GtkListItem *listitem, GListStore *liststore) {
  g_return_val_if_fail (GTK_IS_LIST_ITEM (listitem) || listitem == NULL, NULL);
  g_return_val_if_fail (sl_is_tcsv_stringlist (liststore) || liststore == NULL, NULL);
  TCsvRecord *record;

  record = T_CSV_RECORD (g_object_new (T_TYPE_CSV_RECORD, NULL));
  t_csv_record_set_list_item (record, listitem);
  t_csv_record_set_list_store (record, liststore);
  return record;
}

TCsvRecord *
t_csv_record_new (void) {
  return T_CSV_RECORD (g_object_new (T_TYPE_CSV_RECORD, NULL));
}

/* ----- converter ----- */
/* TCsvRecord <=> GtkStringList */
// TCsvRecord is useful for TCsvWindow. It can be edited and have the current line.
// But it is a bit complicated.
// GtkStringList is simple and useful for interface to other objects.
GtkStringList *
r2sl (TCsvRecord *record) {
  g_return_val_if_fail (T_IS_CSV_RECORD (record), NULL);

  GListStore *tcsv_stringlist = t_csv_record_get_list_store (record);
  GtkStringList *stringlist = sl_to_gtk_string_list (tcsv_stringlist);
  g_object_unref (tcsv_stringlist);
  return stringlist;
}

TCsvRecord *
sl2r (GtkStringList *stringlist) {
  g_return_val_if_fail (GTK_IS_STRING_LIST (stringlist), NULL);

  TCsvRecord *record;
  GListStore *tcsv_stringlist;

  tcsv_stringlist = sl_new_with_gtk_string_list (stringlist);
  record = t_csv_record_new_with_data (NULL, tcsv_stringlist);
  g_object_unref (tcsv_stringlist);
  return record;
}

/* ----- TCsvWindow ----- */
struct _TCsvWindow {
  GtkApplicationWindow parent;
  GtkMenuButton *btnm;
  GtkColumnView *columnview;
  GtkSortListModel *sortlist;
  GListStore *header; /* tcsv_stringlist */
  GListStore *body;
  int *n_column;
  int current_position; /* it can be negative when no current line exists. */
  GFile *file;
  gboolean saved;
  gboolean busy;
};

G_DEFINE_TYPE (TCsvWindow, t_csv_window, GTK_TYPE_APPLICATION_WINDOW)

/* Update current position (current line) */
static void
update_current (TCsvWindow *win, int new) { /* new can be negative (when no current line exists) */
  TCsvRecord *record;
  int n_items = g_list_model_get_n_items (G_LIST_MODEL (win->body));
  GtkListItem *listitem;
  GtkButton *button;
  const char *non_current[1] = {NULL};
  const char *current[2] = {"current", NULL};

  if (new >= n_items)
    return;
  if (win->current_position >=0 && win->current_position <n_items) {
    record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), win->current_position));
    if ((listitem = t_csv_record_get_list_item (record)) != NULL) {
      button = GTK_BUTTON (gtk_list_item_get_child (listitem));
      gtk_widget_set_css_classes (GTK_WIDGET (button), non_current);
      g_object_unref (listitem);
    }
    g_object_unref (record);
  }
  win->current_position = new;
  if (win->current_position >=0) {
    record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), win->current_position));
    if ((listitem = t_csv_record_get_list_item (record)) != NULL) {
      button = GTK_BUTTON (gtk_list_item_get_child (listitem));
      gtk_widget_set_css_classes (GTK_WIDGET (button), current);
      g_object_unref (listitem);
    }
    g_object_unref (record);
  }
}

/* ----- Button "clicked" signal handlers ----- */
static void
win_up_cb (GtkButton *btn, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  TCsvRecord *record, *record_before;

  if (win->current_position <= 0)
    return;
  record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), win->current_position));
  record_before = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), win->current_position - 1));
  t_csv_record_swap_strings (record, record_before);
  g_object_unref (record);
  g_object_unref (record_before);
  update_current (win, win->current_position - 1);
  win->saved = FALSE;
  gtk_window_set_focus (GTK_WINDOW (win), NULL);
}

static void
win_down_cb (GtkButton *btn, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  TCsvRecord *record, *record_after;

  if (win->current_position < 0 || win->current_position >= g_list_model_get_n_items (G_LIST_MODEL (win->body)) - 1)
    return;
  record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), win->current_position));
  record_after = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), win->current_position + 1));
  t_csv_record_swap_strings (record, record_after);
  g_object_unref (record);
  g_object_unref (record_after);
  update_current (win, win->current_position + 1);
  win->saved = FALSE;
  gtk_window_set_focus (GTK_WINDOW (win), NULL);
}

static void
select_current_cb (GtkButton *btn, gpointer user_data) {
  GtkListItem *listitem = GTK_LIST_ITEM (user_data);
  TCsvWindow *win = T_CSV_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (btn), T_TYPE_CSV_WINDOW));

  update_current (win, gtk_list_item_get_position (listitem));
  gtk_window_set_focus (GTK_WINDOW (win), NULL);
}


/* Handlers used by closure tags in ui files or closure expressions */
static char *
record_get_nth_string (TCsvRecord *record, guint n) {
  char *s;
  GListStore *liststore;

  if ((liststore = t_csv_record_get_list_store (record)) == NULL)
    return NULL;
  s = sl_get_string (liststore, n);
  g_object_unref (liststore);
  return s;
}

/* ----- Handlers on GtkSignalListItemFacory ----- */
static void
setup1_cb (GtkListItemFactory *factory, GtkListItem *listitem) {
  GtkWidget *button = gtk_button_new ();
  gtk_list_item_set_child (listitem, button);
  gtk_widget_set_focusable (button, FALSE);
  g_signal_connect (button, "clicked", G_CALLBACK (select_current_cb), listitem);
}

static void
bind1_cb (GtkListItemFactory *factory, GtkListItem *listitem, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GtkWidget *button = gtk_list_item_get_child (listitem);
  TCsvRecord *record = T_CSV_RECORD (gtk_list_item_get_item (listitem));
  const char *non_current[1] = {NULL};
  const char *current[2] = {"current", NULL};

  t_csv_record_set_list_item (record, listitem);
  if (win->current_position == gtk_list_item_get_position (listitem))
    gtk_widget_set_css_classes (button, current);
  else
    gtk_widget_set_css_classes (button, non_current);
}

static void
unbind1_cb (GtkListItemFactory *factory, GtkListItem *listitem) {
  TCsvRecord *record = T_CSV_RECORD (gtk_list_item_get_item (listitem));
  t_csv_record_set_list_item (record, NULL);
}

static void
setup_cb (GtkListItemFactory *factory, GtkListItem *listitem) {
  GtkWidget *text = gtk_text_new ();
  gtk_list_item_set_child (listitem, GTK_WIDGET (text));
}

static void
bind_cb (GtkListItemFactory *factory, GtkListItem *listitem, gpointer user_data) {
  g_return_if_fail (T_IS_CSV_RECORD (gtk_list_item_get_item (listitem)));

  int *position = (int *) user_data;
  TCsvRecord *record = T_CSV_RECORD (gtk_list_item_get_item (listitem));
  GListStore *liststore = t_csv_record_get_list_store (record);
  if (liststore == NULL) {
    g_object_unref (record);
    g_warning ("liststore in a record is not GListStore.\n ");
    return;
  }
  TCsvStr *str = T_CSV_STR (g_list_model_get_item (G_LIST_MODEL (liststore), *position));
  GtkWidget *text = gtk_list_item_get_child (listitem);
  GtkEntryBuffer *buffer;
  GBinding *bind;
  const char *s;

  if (str) {
    buffer = gtk_text_get_buffer (GTK_TEXT (text));
    s = t_csv_str_look_string (str) ? t_csv_str_look_string (str) : "";
    gtk_entry_buffer_set_text (buffer, s, -1);
    bind = g_object_bind_property (buffer, "text", str, "string", G_BINDING_BIDIRECTIONAL);
  } else /* unexpected, though */
    bind = NULL;
  g_object_set_data (G_OBJECT (listitem), "bind", bind);
  g_object_unref (liststore);
  g_object_unref (str);
}

static void
unbind_cb (GtkListItemFactory *factory, GtkListItem *listitem) {
  GBinding *bind = G_BINDING (g_object_get_data (G_OBJECT (listitem), "bind"));

  if (bind)
    g_binding_unbind(bind);
  g_object_set_data (G_OBJECT (listitem), "bind", NULL);
}

/* ----- Build/Remove ColumnViewColumn instances and sorters ----- */
/* ----- set/unset win->header and win->body ----- */
static void
remove_columns (TCsvWindow *win) {
  GListModel *columns;
  int j, n_items;
  GtkColumnViewColumn *column;

  gtk_sort_list_model_set_model (win->sortlist, NULL);
  columns = gtk_column_view_get_columns (win->columnview);
  n_items = g_list_model_get_n_items (columns);
  for (j=n_items-1; j>=1; --j) {
    column = GTK_COLUMN_VIEW_COLUMN (g_list_model_get_item (columns, j));
    gtk_column_view_remove_column (win->columnview, column);
    g_object_unref (column);
  }
  g_clear_object (&win->header);
  g_clear_object (&win->body);
}

/* The function takes the ownership of header and body. */
static void
build_columns (TCsvWindow *win, GListStore *header, GListStore *body) {
  int j, n_columns;
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *params[1];
  GtkExpression *expression;
  GtkSorter *sorter;

  if (win->header) {
    remove_columns (win);
    g_free (win->n_column);
  }
  win->header = header;
  win->body = body;
  n_columns = g_list_model_get_n_items (G_LIST_MODEL (header));
  win->n_column = g_new (int, n_columns);
  /* gtk_sort_model_set_model unref the previous model */
  gtk_sort_list_model_set_model (win->sortlist, G_LIST_MODEL (body));

  for (j=0; j<n_columns; ++j) {
    *(win->n_column + j) = j; /* win-> column points an int array. Its elements are 0 to n_columns-1 */
    factory = gtk_signal_list_item_factory_new ();
    g_signal_connect (factory, "setup", G_CALLBACK (setup_cb), NULL);
    g_signal_connect (factory, "bind", G_CALLBACK (bind_cb), win->n_column + j);
    g_signal_connect (factory, "unbind", G_CALLBACK (unbind_cb), NULL);

    column = gtk_column_view_column_new (sl_look_string (header, j), factory); /* factory is owned by the column. */

    params[0] = gtk_constant_expression_new (G_TYPE_INT, j);
    expression = gtk_cclosure_expression_new (G_TYPE_STRING, NULL, 1, params, G_CALLBACK (record_get_nth_string), NULL, NULL);
    sorter = GTK_SORTER (gtk_string_sorter_new (expression)); /* sorter takes the ownership of the expression. */
    gtk_column_view_column_set_sorter (column, sorter);
    g_object_unref (sorter);

    gtk_column_view_append_column (win->columnview, column);
    g_object_unref (column);
  }
  win->saved = FALSE;
}

/* ----- action activated handlers ----- */
static TCsvRecord *
create_new_record (TCsvWindow *win) {
  TCsvRecord *record;
  GListStore *liststore;
  int j, n;

  if (! win->header)
    return NULL;
  liststore = g_list_store_new (T_TYPE_CSV_STR);
  record = t_csv_record_new_with_data (NULL, liststore);
  n = g_list_model_get_n_items (G_LIST_MODEL (win->header));
  for (j=0; j<n; ++j)
    g_list_store_append (liststore, t_csv_str_new ());
  g_object_unref (liststore);
  return record;
}

static void
append_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  TCsvRecord *record;

  if (! win->header)
    return;
  record = create_new_record (win);
  if (win->current_position >= 0 && win->current_position <g_list_model_get_n_items (G_LIST_MODEL (win->body))) {
    g_list_store_insert (win->body, win->current_position + 1, record);
    update_current (win, win->current_position + 1);
  } else {
    g_list_store_append (win->body, record);
    update_current (win, g_list_model_get_n_items (G_LIST_MODEL (win->body)) - 1);
  }
  g_object_unref (record);
  win->saved = FALSE;
}

static void
remove_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  int old_position;

  if (win->current_position >= 0 && win->current_position <g_list_model_get_n_items (G_LIST_MODEL (win->body))) {
    old_position = win->current_position;
    win->current_position = -1;
    g_list_store_remove (win->body, old_position);
    update_current (win, -1);
  }
  win->saved = FALSE;
}

static void
rec_dialog_response_cb (TCsvRecDialog *rec_dialog, int response_id, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GListStore *liststore;
  TCsvRecord *new_record, *old_record;

  if (response_id == GTK_RESPONSE_ACCEPT) {
    if ((liststore = t_csv_rec_dialog_get_liststore (rec_dialog)) == NULL)
      return;
    new_record = t_csv_record_new_with_data (NULL, liststore);
    g_object_unref (liststore);
    old_record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), win->current_position));
    t_csv_record_swap_strings (old_record, new_record);
    g_object_unref (new_record);
    g_object_unref (old_record);
    win->saved = FALSE;
  }
  gtk_window_destroy (GTK_WINDOW (rec_dialog));
  // update_current (win, win->current_position);
}

static void
edit_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GtkWidget *rec_dialog;
  GListStore *liststore;
  TCsvRecord *record;

  if (win->current_position < 0 || win->current_position >= g_list_model_get_n_items (G_LIST_MODEL (win->body)))
    return;
  if ((record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), win->current_position))) == NULL)
    return;
  if ((liststore = t_csv_record_get_list_store (record)) == NULL)
    return;
  g_object_unref (record);
  rec_dialog = t_csv_rec_dialog_new (GTK_WINDOW (win), win->header, liststore);
  g_object_unref (liststore);
  g_signal_connect (rec_dialog, "response", G_CALLBACK (rec_dialog_response_cb), win);
  gtk_window_present (GTK_WINDOW (rec_dialog));
}

static void
insert_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  TCsvRecord *record;

  if (! win->header)
    return;
  record = create_new_record (win);
  if (win->current_position >= 0 && win->current_position <g_list_model_get_n_items (G_LIST_MODEL (win->body))) {
    g_list_store_insert (win->body, win->current_position, record);
    win->current_position += 1; /* Current line has moved forward by one */
    update_current (win, win->current_position - 1);
  }
  g_object_unref (record);
  win->saved = FALSE;
}

/* open and read functions */
static void
t_csv_window_read (TCsvWindow *win, GFile *file) {
  GtkWidget *message_dialog;
  GError *err = NULL;
  GListStore *header, *tcsv_stringlist;
  GListStore *body, *liststore;
  TCsvRecord *record;
  int i, n_row, n_column;

  liststore = g_list_store_new (G_TYPE_LIST_STORE);
  if (! csv_read (liststore, file, &n_row, &n_column, &err)) {
    message_dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                            "%s.\n", err->message);
    g_signal_connect (message_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_widget_show (message_dialog);
    g_error_free (err);
    return;
  }

  header = G_LIST_STORE (g_list_model_get_item (G_LIST_MODEL (liststore), 0));
  g_list_store_remove (liststore, 0);
  body = g_list_store_new (T_TYPE_CSV_RECORD);
  n_row = g_list_model_get_n_items (G_LIST_MODEL (liststore));
  n_column = g_list_model_get_n_items (G_LIST_MODEL (header));
  for (i=0; i<n_row; ++i) {
    tcsv_stringlist = G_LIST_STORE (g_list_model_get_item (G_LIST_MODEL (liststore), i));
    record = t_csv_record_new_with_data (NULL, tcsv_stringlist);
    g_object_unref (tcsv_stringlist);
    g_list_store_append (body, record);
    g_object_unref (record);
  }
  g_object_unref (liststore);
  build_columns (win, header, body);
  win->file = g_object_ref (file);
  win->saved = TRUE;
}

static void
open_dialog_response(GtkWidget *dialog, gint response, TCsvWindow *win) {
  GFile *file;

  if (response == GTK_RESPONSE_ACCEPT
      && G_IS_FILE (file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog)))) {
    t_csv_window_read (win, file);
    g_object_unref (file);
  }
  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
open_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GtkWidget *dialog;

  if (win->header)
    return;
  dialog = gtk_file_chooser_dialog_new ("Open file", GTK_WINDOW (win), GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "Cancel", GTK_RESPONSE_CANCEL,
                                        "Open", GTK_RESPONSE_ACCEPT,
                                        NULL);
  g_signal_connect (dialog, "response", G_CALLBACK (open_dialog_response), win);
  gtk_window_present (GTK_WINDOW (dialog));
}

/* ----- save and saveas ----- */
static void
t_csv_window_write (TCsvWindow *win, GFile *file) {
  GError *err = NULL;
  GtkWidget *message_dialog;
  GListStore *liststore;
  GListStore *tcsv_stringlist;
  int i, n_row, n_column;
  TCsvRecord *record;

  // busy is used because win->body is changed during the saving process.
  if (win->busy)
    return;
  else
    win->busy = TRUE;
  liststore = g_list_store_new (G_TYPE_LIST_STORE);
  g_list_store_append (liststore, win->header);
  n_row = g_list_model_get_n_items (G_LIST_MODEL (win->body));
  for (i=0; i<n_row; ++i) {
    record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), i));
    tcsv_stringlist = t_csv_record_get_list_store (record);
    g_object_unref (record);
    g_list_store_append (liststore, tcsv_stringlist);
    g_object_unref (tcsv_stringlist);
  }
  n_row = g_list_model_get_n_items (G_LIST_MODEL (liststore));
  n_column = g_list_model_get_n_items (G_LIST_MODEL (win->header));
  if (csv_write (liststore, file, n_row, n_column, &err))
    win->saved = TRUE;
  else {
    message_dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                            "%s.\n", err->message);
    g_signal_connect (message_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_widget_show (message_dialog);
    g_error_free (err);
  }
  g_object_unref (liststore);
  win->busy = FALSE;
}

static void
saveas_dialog_response (GtkWidget *dialog, gint response, TCsvWindow *win) {
  GFile *file;

  if (response == GTK_RESPONSE_ACCEPT
      && G_IS_FILE (file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog)))) {
    t_csv_window_write (win, file);
    win->file = file;
  }
  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
t_csv_window_saveas (TCsvWindow *win) {
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new ("Save file", GTK_WINDOW (win), GTK_FILE_CHOOSER_ACTION_SAVE,
                                      "Cancel", GTK_RESPONSE_CANCEL,
                                      "Save", GTK_RESPONSE_ACCEPT,
                                      NULL);
  g_signal_connect (dialog, "response", G_CALLBACK (saveas_dialog_response), win);
  gtk_window_present (GTK_WINDOW (dialog));
}

static void
save_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  if (G_IS_FILE (win->file))
    t_csv_window_write (win, win->file);
  else
    t_csv_window_saveas (win);
}

/* ----- close -- just removes the columns and list model ----- */
static void
t_csv_window_close (TCsvWindow *win) {
  remove_columns (win);
  if (win->file)
    g_clear_object (&win->file);
  win->saved = TRUE;
  win->current_position = -1;
}

static void
close_alert_response_cb (GtkDialog *alert, int response_id, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  gtk_window_destroy (GTK_WINDOW (alert));
  if (response_id == GTK_RESPONSE_ACCEPT)
    t_csv_window_close (win);
}

// When the close button (top right x-shaped button) is clicked on, the closing process starts.
// Before the process, this handler is called.
// If the handler returns FALSE, the process goes on.
// If it returns TRUE, the process stops.
// In the closing process, t_csv_window_close isn't called.
// Instead, t_csv_window_dispose releases objects except widgets.
// Widgets are automatically released.
static gboolean
before_close (GtkWindow *window) {
  TCsvWindow *win = T_CSV_WINDOW (window);
  TCsvAlert *alert;

  if (win->saved)
    return FALSE;
  else {
    alert = T_CSV_ALERT (t_csv_alert_new (GTK_WINDOW (win)));
    t_csv_alert_set_message (alert, "Contents aren't saved yet.\nAre you sure to close?");
    t_csv_alert_set_button_label (alert, "Close");
    g_signal_connect (GTK_DIALOG (alert), "response", G_CALLBACK (close_alert_response_cb), win);
    gtk_window_present (GTK_WINDOW (alert));
  }
  return TRUE;
}

static void
close_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  if (! before_close (GTK_WINDOW (win)))
    t_csv_window_close (win);
}

static void
mod_dialog_response_cb (TCsvModDialog *mod_dialog, int response_id, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GListStore *new_header;
  GListStore *new_body;
  TCsvRecord *new_record, *record;
  GListStore *tcsv_stringlist;
  const char *s;
  GListStore *list_modify;
  TModify *modify;
  int i, n_row, j, n_modifies, n;

  if (response_id == GTK_RESPONSE_ACCEPT) {
    list_modify = t_csv_mod_dialog_get_list_modify (mod_dialog);
    n_modifies = g_list_model_get_n_items (G_LIST_MODEL (list_modify));
    new_header = g_list_store_new (T_TYPE_CSV_STR);
    new_body = g_list_store_new (T_TYPE_CSV_RECORD);
    n_row = win->body ? g_list_model_get_n_items (G_LIST_MODEL (win->body)) : 0;
    /* n <= the number of items to copy */
    for (n=j=0; j<n_modifies; ++j ) {
      modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (list_modify), j));
      if (t_modify_get_old_position (modify) >= 0)
        ++n;
      g_object_unref (modify);
    }
    for (j=0; j<n_modifies; ++j) {
      modify = g_list_model_get_item (G_LIST_MODEL (list_modify), j);
      if (t_modify_look_new_string (modify) && *(t_modify_look_new_string (modify))) { /* not NULL nor "" */
        // append str to header
        sl_append_string (new_header, t_modify_look_new_string (modify));
        if (n > 0) /* if new_body takes over the old body */
          for (i=0; i<n_row; ++i) {
            if ((new_record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (new_body), i))) == NULL) {
              tcsv_stringlist = g_list_store_new (T_TYPE_CSV_STR);
              new_record = t_csv_record_new_with_data (NULL, tcsv_stringlist);
              g_object_unref (tcsv_stringlist);
              g_list_store_append (new_body, new_record);
            }
            if (t_modify_get_old_position (modify)>= 0) {
              record = T_CSV_RECORD (g_list_model_get_item (G_LIST_MODEL (win->body), i));
              tcsv_stringlist = t_csv_record_get_list_store (record);
              s = sl_look_string (tcsv_stringlist, t_modify_get_old_position (modify));
              if (s == NULL)
                s = "";
              g_object_unref (tcsv_stringlist);
              g_object_unref (record);
            } else
              s = "";
            tcsv_stringlist = t_csv_record_get_list_store (new_record);
            sl_append_string (tcsv_stringlist, s);
            g_object_unref (tcsv_stringlist);
            g_object_unref (new_record);
          }
      }
      g_object_unref (modify);
    }
    build_columns (win, new_header, new_body); /* The ownership of new_header and new_body is taken by build_column. */
  }
  win->busy = FALSE;
  gtk_window_destroy (GTK_WINDOW (mod_dialog));
}

static void
modify_field_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GtkWidget *mod_dialog;

  if (win->header == NULL)
    return;
  win->busy = TRUE;
  mod_dialog = t_csv_mod_dialog_new (GTK_WINDOW (win), win->header);
  g_signal_connect (mod_dialog, "response", G_CALLBACK (mod_dialog_response_cb), win);
  gtk_window_present (GTK_WINDOW (mod_dialog));
}

static void
reset_sort_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  gtk_column_view_sort_by_column (win->columnview, NULL, GTK_SORT_ASCENDING);
}

static void
new_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GtkWidget *mod_dialog;

  if (win->busy || win->header)
    return;
  win->busy = TRUE;
  mod_dialog = t_csv_mod_dialog_new (GTK_WINDOW (win), NULL); /* Header is NULL. It means no csv data in the window. */
  if (mod_dialog == NULL)
    return;
  g_signal_connect (mod_dialog, "response", G_CALLBACK (mod_dialog_response_cb), win);
  gtk_window_present (GTK_WINDOW (mod_dialog));
}

static void
saveas_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  t_csv_window_saveas (win);
}

static void
adjustment_value_changed_cb (GtkAdjustment *adjustment, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  gtk_window_set_focus (GTK_WINDOW (win), NULL);
}

/* --- TCsvWindow object construction/destruction --- */ 
static void
t_csv_window_dispose (GObject *gobject) {
  TCsvWindow *win = T_CSV_WINDOW (gobject);

  /* this function is available since GTK 4.8 */
  gtk_widget_dispose_template (GTK_WIDGET (win), T_TYPE_CSV_WINDOW);

  g_clear_object (&win->header);
  // win->body is owned by the column view.
  g_clear_object (&win->file);
  G_OBJECT_CLASS (t_csv_window_parent_class)->dispose (gobject);
}

static void
t_csv_window_init (TCsvWindow *win) {
  GtkBuilder *build;
  GMenuModel *menu;

  gtk_widget_init_template (GTK_WIDGET (win));
  win->header = NULL;
  win->file = NULL;
  win->saved = TRUE;
  win->busy = FALSE;
  win->current_position = -1;
  build = gtk_builder_new_from_resource ("/com/github/ToshioCP/tcsv/menu.ui");
  menu = G_MENU_MODEL (gtk_builder_get_object (build, "menu")); /* menu is owned by the bulder. */
  gtk_menu_button_set_menu_model (win->btnm, menu);
  g_object_unref (build);

/* ----- action ----- */
  const GActionEntry win_entries[] = {
    { "apprec", append_record_activated, NULL, NULL, NULL },
    { "rmrec", remove_record_activated, NULL, NULL, NULL },
    { "edrec", edit_record_activated, NULL, NULL, NULL },
    { "insrec", insert_record_activated, NULL, NULL, NULL },
    { "modify-field", modify_field_activated, NULL, NULL, NULL },
    { "reset-sort", reset_sort_activated, NULL, NULL, NULL },
    { "new", new_activated, NULL, NULL, NULL },
    { "open", open_activated, NULL, NULL, NULL },
    { "save", save_activated, NULL, NULL, NULL },
    { "close", close_activated, NULL, NULL, NULL },
    { "saveas", saveas_activated, NULL, NULL, NULL }
  };
  g_action_map_add_action_entries (G_ACTION_MAP (win), win_entries, G_N_ELEMENTS (win_entries), win);

  g_signal_connect (win, "close-request", G_CALLBACK (before_close), NULL);
}

static void
t_csv_window_class_init (TCsvWindowClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->dispose = t_csv_window_dispose;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvwindow.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, sortlist);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, btnm);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), win_up_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), win_down_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), setup1_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), bind1_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), unbind1_cb);
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), adjustment_value_changed_cb);
}

/* ----- functions ----- */
gboolean
t_csv_window_is_saved (TCsvWindow *win) {
  return win->saved;
}

GtkWidget *
t_csv_window_new_with_file (GtkApplication *app, GFile *file) {
  g_return_val_if_fail (GTK_IS_APPLICATION (app), NULL);
  g_return_val_if_fail (G_IS_FILE (file), NULL);

  GtkWidget *win;

  win = t_csv_window_new (app);
  t_csv_window_read (T_CSV_WINDOW (win), file);
  return win;
}

GtkWidget *
t_csv_window_new (GtkApplication *app) {
  g_return_val_if_fail (GTK_IS_APPLICATION (app), NULL);

  return GTK_WIDGET (g_object_new (T_TYPE_CSV_WINDOW, "application", app, NULL));
}
