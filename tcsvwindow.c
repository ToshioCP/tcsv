#include <gtk/gtk.h>
#include "tcsvwindow.h"
#include "tcsvalert.h"
#include "tcsv.h"
#include "tcsvrecdialog.h"
#include "tcsvmoddialog.h"
#include "tmodify.h"

struct _TCsvWindow {
  GtkApplicationWindow parent;
  GtkMenuButton *btnm;
  GtkColumnView *columnview;
  GtkSingleSelection *singleselection;
  GtkSortListModel *sortlist;
  GtkStringList *header;
  GListStore *body;
  GFile *file;
  gboolean saved;
  gboolean busy;
};

G_DEFINE_TYPE (TCsvWindow, t_csv_window, GTK_TYPE_APPLICATION_WINDOW)

/* Handlers used by closure tags in ui files or closure expressions */
static char *
record_get_nth_string (GtkStringList *record, int n) {
  const char *s;

  s = gtk_string_list_get_string (record, n);
  return s ? g_strdup (s) : NULL;
}

char *
get_nth_string (GtkListItem *item, GtkStringList *record, int n) {
  return   record ? g_strdup (gtk_string_list_get_string (record, n)) : NULL;
}

/* ----- Build/Remove ColumnViewColumn instances and sorters ----- */
/* ----- set/unset win->header and win->body ----- */
static void
remove_columns (TCsvWindow *win) {
  GListModel *columns;
  int j, n_items;
  GtkColumnViewColumn *column;

  columns = gtk_column_view_get_columns (win->columnview);
  n_items = g_list_model_get_n_items (columns);
  for (j=n_items-1; j>=0; --j) {
    column = GTK_COLUMN_VIEW_COLUMN (g_list_model_get_item (columns, j));
    gtk_column_view_remove_column (win->columnview, column);
    g_object_unref (column);
  }
  g_clear_object (&win->header);
  g_clear_object (&win->body);
}

/* The function takes the ownership of header and body. */
static void
build_columns (TCsvWindow *win, GtkStringList *header, GListStore *body) {
  int j, n_columns;
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *params[1];
  GtkExpression *expression;
  GtkSorter *sorter;
  char *ui_string;
  GBytes *bytes;
  const char *format = 
"<?xml version='1.0' encoding='UTF-8'?>"
"<interface>"
"  <template class='GtkListItem'>"
"    <property name='child'>"
"      <object class='GtkLabel'>"
"        <property name='ellipsize'>PANGO_ELLIPSIZE_END</property>"
"        <property name='width-chars'>16</property>"
"        <property name='hexpand'>TRUE</property>"
"        <property name='xalign'>0</property>"
"        <binding name='label'>"
"          <closure type='gchararray' function='get_nth_string'>"
"            <lookup name='item'>GtkListItem</lookup>"
"            <constant type='gint'>%d</constant>"
"          </closure>"
"        </binding>"
"      </object>"
"    </property>"
"  </template>"
"</interface>"
;

  if (win->header)
    remove_columns (win);
  win->header = header;
  win->body = body;
  /* gtk_sort_model_set_model unref the previous model */
  gtk_sort_list_model_set_model (win->sortlist, G_LIST_MODEL (body));

  n_columns = g_list_model_get_n_items (G_LIST_MODEL (header));
  for (j=0; j<n_columns; ++j) {
    ui_string = g_strdup_printf (format, j);
    bytes =  g_bytes_new_take (ui_string, (gsize) strlen (ui_string));
    factory = gtk_builder_list_item_factory_new_from_bytes (NULL, bytes);
    g_bytes_unref (bytes);

    column = gtk_column_view_column_new (gtk_string_list_get_string (header, j), factory); /* factory is owned by the column. */
 
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

static void
rec_dialog_response_cb (TCsvRecDialog *rec_dialog, int response_id, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  int position = t_csv_rec_dialog_get_position (rec_dialog);
  GtkStringList *s_array[1];

  if (response_id == GTK_RESPONSE_ACCEPT) {
    s_array[0] = t_csv_rec_dialog_get_record (rec_dialog);
    g_list_store_splice (win->body, position, 1, (void **) s_array, 1);
    g_object_unref (s_array[0]);
    win->saved = FALSE;
  }
  gtk_window_destroy (GTK_WINDOW (rec_dialog));
}

static void
columnview_activate_cb (GtkColumnView *columnview, guint position, TCsvWindow *win) {
  GtkWidget *rec_dialog;
  GtkStringList *record;

  record = GTK_STRING_LIST (g_list_model_get_item (G_LIST_MODEL (win->body), position));
  rec_dialog = t_csv_rec_dialog_new (GTK_WINDOW (win), win->header, record, position);
  g_object_unref (record);
  g_signal_connect (rec_dialog, "response", G_CALLBACK (rec_dialog_response_cb), win);
  gtk_window_present (GTK_WINDOW (rec_dialog));
}

/* ----- action activated handlers ----- */
static GtkStringList *
create_new_record (TCsvWindow *win) {
  GtkStringList *record;
  int j, n;

  if (! win->header)
    return NULL;
  record = gtk_string_list_new (NULL);
  n = g_list_model_get_n_items (G_LIST_MODEL (win->header));
  for (j=0; j<n; ++j)
    gtk_string_list_append (record, "");
  return record;
}

static void
append_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  int position, n_row;
  GtkStringList *record;
  GtkWidget *rec_dialog;

  if (! win->header)
    return;
  n_row = g_list_model_get_n_items (G_LIST_MODEL (win->body));
  if ( n_row > 0 && (position = gtk_single_selection_get_selected (win->singleselection)) == GTK_INVALID_LIST_POSITION)
    return;
  if (n_row == 0)
    position = -1;
  record = create_new_record (win);
  g_list_store_insert (win->body, position + 1, record);

  rec_dialog = t_csv_rec_dialog_new (GTK_WINDOW (win), win->header, record, position + 1);
  g_object_unref (record);
  g_signal_connect (rec_dialog, "response", G_CALLBACK (rec_dialog_response_cb), win);
  gtk_window_present (GTK_WINDOW (rec_dialog));
}

static void
remove_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  int position = gtk_single_selection_get_selected (win->singleselection);

  if ((position = gtk_single_selection_get_selected (win->singleselection)) == GTK_INVALID_LIST_POSITION)
    return;
  g_list_store_remove (win->body, position);
  win->saved = FALSE;
}

static void
insert_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  int position, n_row;
  GtkStringList *record;
  GtkWidget *rec_dialog;

  n_row = g_list_model_get_n_items (G_LIST_MODEL (win->body));
  if (n_row > 0 && (position = gtk_single_selection_get_selected (win->singleselection)) == GTK_INVALID_LIST_POSITION)
    return;
  if (n_row == 0)
    position = 0;
  record = create_new_record (win);
  g_list_store_insert (win->body, position, record);

  rec_dialog = t_csv_rec_dialog_new (GTK_WINDOW (win), win->header, record, position);
  g_object_unref (record);
  g_signal_connect (rec_dialog, "response", G_CALLBACK (rec_dialog_response_cb), win);
  gtk_window_present (GTK_WINDOW (rec_dialog));
}

/* open and read functions */
static void
t_csv_window_read (TCsvWindow *win, GFile *file) {
  GtkWidget *message_dialog;
  GError *err = NULL;
  int n_row, n_column;
  GtkStringList *header;
  GListStore *body;

  body = g_list_store_new (GTK_TYPE_STRING_LIST);
  if (! csv_read (body, file, &n_row, &n_column, &err)) {
    message_dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                            "%s.\n", err->message);
    g_signal_connect (message_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_widget_show (message_dialog);
    g_error_free (err);
    return;
  }
  header = GTK_STRING_LIST (g_list_model_get_item (G_LIST_MODEL (body), 0));
  g_list_store_remove (body, 0);
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
  int n_row, n_column;
  GError *err = NULL;
  GtkWidget *message_dialog;

  // busy is used because win->body is changed during the saving process.
  if (win->busy)
    return;
  else
    win->busy = TRUE;
  g_list_store_insert (win->body, 0, win->header);
  n_row = g_list_model_get_n_items (G_LIST_MODEL (win->body));
  n_column = g_list_model_get_n_items (G_LIST_MODEL (win->header));
  if (csv_write (win->body, file, n_row, n_column, &err))
    win->saved = TRUE;
  else {
    message_dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                            "%s.\n", err->message);
    g_signal_connect (message_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_widget_show (message_dialog);
    g_error_free (err);
  }
  g_list_store_remove (win->body, 0);
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
  GtkStringList *new_header;
  GListStore *new_body;
  GtkStringList *new_record, *record;
  GListStore *list_modify;
  TModify *modify;
  int i, n_row, j, n_modifies, n;
  const char *s;

  // mod_busy is used becuase columns and list model is changed during the following process.
  if (response_id == GTK_RESPONSE_ACCEPT) {
    list_modify = t_csv_mod_dialog_get_list_modify (mod_dialog);
    n_modifies = g_list_model_get_n_items (G_LIST_MODEL (list_modify));
    new_header = gtk_string_list_new (NULL);
    new_body = g_list_store_new (GTK_TYPE_STRING_LIST);
    n_row = win->body ? g_list_model_get_n_items (G_LIST_MODEL (win->body)) : 0;
    /* n <= the number of items to copy */
    for (n=i=0; i<n_modifies; ++i ) {
      modify = T_MODIFY (g_list_model_get_item (G_LIST_MODEL (list_modify), i));
      if (t_modify_get_old_position (modify) >= 0)
        ++n;
      g_object_unref (modify);
    }
    for (i=0; i<n_modifies; ++i) {
      modify = g_list_model_get_item (G_LIST_MODEL (list_modify), i);
      if (t_modify_look_new_string (modify) && *(t_modify_look_new_string (modify))) { /* not NULL nor "" */
        // append str to header
        gtk_string_list_append (new_header, t_modify_look_new_string (modify));
        if (n > 0) /* if new_body takes over the old body */
          for (j=0; j<n_row; ++j) {
            if ((new_record = GTK_STRING_LIST (g_list_model_get_item (G_LIST_MODEL (new_body), j))) == NULL) {
              new_record = gtk_string_list_new (NULL);
              g_list_store_append (new_body, new_record);
            }
            if (t_modify_get_old_position (modify)>= 0) {
              record = GTK_STRING_LIST (g_list_model_get_item (G_LIST_MODEL (win->body), j));
              s = gtk_string_list_get_string (record, t_modify_get_old_position (modify));
              g_object_unref (record);
            } else
              s = "";
            gtk_string_list_append (new_record, s);
            g_object_unref (new_record);
          }
      }
    }
    build_columns (win, new_header, new_body); /* The ownership of new_header and new_body is taken by vuild_column. */
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

/* --- TCsvWindow object construction/destruction --- */ 
static void
t_csv_window_dispose (GObject *gobject) {
  TCsvWindow *win = T_CSV_WINDOW (gobject);

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
  build = gtk_builder_new_from_resource ("/com/github/ToshioCP/tcsv/menu.ui");
  menu = G_MENU_MODEL (gtk_builder_get_object (build, "menu")); /* menu is owned by the bulder. */
  gtk_menu_button_set_menu_model (win->btnm, menu);
  g_object_unref (build);

/* ----- action ----- */
  const GActionEntry win_entries[] = {
    { "apprec", append_record_activated, NULL, NULL, NULL },
    { "rmrec", remove_record_activated, NULL, NULL, NULL },
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
  g_signal_connect (win->columnview, "activate", G_CALLBACK (columnview_activate_cb), win);
}

static void
t_csv_window_class_init (TCsvWindowClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->dispose = t_csv_window_dispose;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvwindow.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, singleselection);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, sortlist);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, btnm);
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
