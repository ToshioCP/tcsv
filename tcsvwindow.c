#include <gtk/gtk.h>
#include "tcsvwindow.h"
#include "tcsvalert.h"
#include "tcsvpref.h"
#include "tcsv.h"
#include "css.h"
#include "tcsvrecdialog.h"
#include "tcsvflddialog.h"
#include "tstr.h"

struct _TCsvWindow {
  GtkApplicationWindow parent;
  GtkColumnView *columnview;
  GtkSingleSelection *singleselection;
  GtkSortListModel *sortlist;
  GListStore *liststore;
  GtkMenuButton *btnm;

  GListStore *header;
  int n_row;
  int n_column;
  GFile *file;
  gboolean saved;
  GSettings *settings;
};

G_DEFINE_TYPE (TCsvWindow, t_csv_window, GTK_TYPE_APPLICATION_WINDOW);

static GListStore *
create_new_record (TCsvWindow *win) {
  GListStore *liststore;
  TStr *str;
  int j;

  liststore = g_list_store_new (T_TYPE_STR);
  if (win->n_column <= 0)
    return NULL;
  for (j=0; j<win->n_column; ++j) {
    str = t_str_new_with_string ("");
    g_list_store_append (liststore, str);
    g_object_unref (str);
  }
  return liststore;
}

/* alert response signal handler */
static void
alert_response_cb (GtkDialog *alert, int response_id, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  gtk_window_destroy (GTK_WINDOW (alert));
  if (response_id == GTK_RESPONSE_ACCEPT)
    gtk_window_destroy (GTK_WINDOW (win));
}

static gboolean
before_close (GtkWindow *window, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (window);
  TCsvAlert *alert;

  if (win->saved)
    return FALSE;
  else {
    alert = T_CSV_ALERT (t_csv_alert_new (GTK_WINDOW (win)));
    t_csv_alert_set_message (alert, "Contents aren't saved yet.\nAre you sure to close?");
    t_csv_alert_set_button_label (alert, "Close");
    g_signal_connect (GTK_DIALOG (alert), "response", G_CALLBACK (alert_response_cb), win);
    gtk_widget_show (GTK_WIDGET (alert));
  }
  return TRUE;
}

static gboolean busy = FALSE;

static void
rec_dialog_response_cb (TCsvRecDialog *rec_dialog, int response_id, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (t_csv_rec_dialog_get_window (rec_dialog));
  int position = t_csv_rec_dialog_get_position (rec_dialog);
  GListStore *s_array[2];

  s_array[1] = NULL;
  if (response_id == GTK_RESPONSE_ACCEPT) {
    s_array[0] = t_csv_rec_dialog_get_record (rec_dialog);
    g_list_store_splice (win->liststore, position, 1, (void **) s_array, 1);
    g_object_unref (s_array[0]);
  }
  gtk_window_destroy (GTK_WINDOW (rec_dialog));
  busy = FALSE;
}

static void
columnview_activate_cb (GtkColumnView *columnview, guint position, TCsvWindow *win) {
  GtkWidget *rec_dialog;
  GListStore *record;

  if (busy)
    return;
  else
    busy = TRUE;
  record = G_LIST_STORE (g_list_model_get_item (G_LIST_MODEL (win->liststore), position));
  rec_dialog = t_csv_rec_dialog_new (GTK_WINDOW (win), win->header, record, position);
  g_object_unref (record);
  g_signal_connect (rec_dialog, "response", G_CALLBACK (rec_dialog_response_cb), NULL);
  gtk_widget_show (rec_dialog);
}

char *
get_string0 (GListStore *record, int j) {
  TStr *str;
  char *s;

  if (G_IS_LIST_STORE (record) && j >= 0 && j < g_list_model_get_n_items (G_LIST_MODEL (record))) {
    str = T_STR (g_list_model_get_item (G_LIST_MODEL (record), j));
    s = t_str_get_string (str);
    g_object_unref (str);
  } else
    s = NULL;
  return s;
}

char *
get_string (GtkListItem *item, GListStore *record, int j) {
  return get_string0 (record, j);
}

static GBytes *
get_bytes (int position) {
  char *data;
  gsize size;

  char *format = 
"<?xml version='1.0' encoding='UTF-8'?>"
"<interface>"
  "<template class='GtkListItem'>"
    "<property name='child'>"
      "<object class='GtkLabel'>"
        "<property name='ellipsize'>PANGO_ELLIPSIZE_END</property>"
        "<property name='width-chars'>16</property>"
        "<property name='hexpand'>TRUE</property>"
        "<property name='xalign'>0</property>"
        "<binding name='label'>"
          "<closure type='gchararray' function='get_string'>"
            "<lookup name='item'>GtkListItem</lookup>"
            "<constant type='gint'>%d</constant>"
          "</closure>"
        "</binding>"
      "</object>"
    "</property>"
  "</template>"
"</interface>";

  data = g_strdup_printf (format, position);
  size = (gsize) strlen (data);
  return g_bytes_new_take (data, size);
}

/* Read csv file, build listmodel and columnview */
static void
t_csv_window_read (TCsvWindow *win, GFile *file) {
  GtkWidget *message_dialog;
  GError *err = NULL;
  GtkColumnViewColumn *column;
  GtkListItemFactory *factory;
  GtkSorter *sorter;
  GtkExpression *params[1];
  GtkExpression *expression;
  int j;
  TStr *str;
  char *s;

  if (! csv_read (win->liststore, file, &win->n_row, &win->n_column, &err)) {
    message_dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                            "%s.\n", err->message);
    g_signal_connect (message_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_widget_show (message_dialog);
    g_error_free (err);
    return;
  }
  win->file = file;
  win->header = G_LIST_STORE (g_list_model_get_item (G_LIST_MODEL (win->liststore), 0));
  g_list_store_remove (win->liststore, 0);
  --win->n_row;
  for (j=0; j<win->n_column; ++j) {
    factory = gtk_builder_list_item_factory_new_from_bytes (NULL, get_bytes (j));
    str = T_STR (g_list_model_get_item (G_LIST_MODEL (win->header), j));
    s = t_str_get_string (str);
    column = gtk_column_view_column_new (s, factory);
    g_free (s);
    g_object_unref (str);
 
    params[0] = gtk_constant_expression_new (G_TYPE_INT, j);
    expression = gtk_cclosure_expression_new (G_TYPE_STRING, NULL, 1, params, G_CALLBACK (get_string0), NULL, NULL);
    sorter = GTK_SORTER (gtk_string_sorter_new (expression));
    gtk_column_view_column_set_sorter (column, sorter);

    gtk_column_view_append_column (win->columnview, column);
  }
}

/* Close and unset columnview. remove listmodel */
static void
t_csv_window_close (TCsvWindow *win) {
  GListModel *columns;
  GtkColumnViewColumn *column;
  int j;

  columns = gtk_column_view_get_columns (win->columnview);
  for (j=win->n_column-1; j>=0; --j) {
    column = GTK_COLUMN_VIEW_COLUMN (g_list_model_get_item (columns, j));
    gtk_column_view_remove_column (win->columnview, column);
    g_object_unref (column);
  }
  g_clear_object (&win->header);
  g_list_store_remove_all (win->liststore);
  win->n_row = win->n_column = 0;
  g_clear_object (&win->file);
  win->saved = TRUE;
}

/* ----- read/write functions ----- */
static void
open_dialog_response(GtkWidget *dialog, gint response, TCsvWindow *win) {
  GFile *file;

  if (response == GTK_RESPONSE_ACCEPT
      && G_IS_FILE (file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog))))
    t_csv_window_read (win, file);
  /* file is taken to win->file, so don't unref it. */
  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
t_csv_window_write (TCsvWindow *win, GFile *file) {
  GError *err = NULL;
  GtkWidget *message_dialog;

  g_list_store_insert (win->liststore, 0, win->header);
  if (csv_write (win->liststore, file, win->n_row + 1, win->n_column, &err))
    win->saved = TRUE;
  else {
    message_dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                            "%s.\n", err->message);
    g_signal_connect (message_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_widget_show (message_dialog);
    g_error_free (err);
  }
  g_list_store_remove (win->liststore, 0);
}

static void
saveas_dialog_response (GtkWidget *dialog, gint response, TCsvWindow *win) {
  GFile *file;

  if (response == GTK_RESPONSE_ACCEPT
      && G_IS_FILE (file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog))))
    t_csv_window_write (win, file);
  win->file = file;
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
  gtk_widget_show (dialog);
}

/* ----- action activated handlers ----- */
static void
add_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  int position;
  GListStore *record;
  GtkWidget *rec_dialog;

  if (win->n_column <= 0)
    return;
  position = gtk_single_selection_get_selected (win->singleselection);
  if (position == GTK_INVALID_LIST_POSITION)
    return;
  record = create_new_record (win);
  if (position == win->n_row - 1)
    g_list_store_append (win->liststore, record);
  else
    g_list_store_insert (win->liststore, position + 1, record);
  ++win->n_row;

  rec_dialog = t_csv_rec_dialog_new (GTK_WINDOW (win), win->header, record, position + 1);
  g_object_unref (record);
  g_signal_connect (rec_dialog, "response", G_CALLBACK (rec_dialog_response_cb), NULL);
  gtk_widget_show (rec_dialog);
}

static void
remove_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  int position = gtk_single_selection_get_selected (win->singleselection);

  if (position == GTK_INVALID_LIST_POSITION || win->n_row <= 0)
    return;
  g_list_store_remove (win->liststore, position);
  --win->n_row;
}

static void
insert_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  int position;
  GListStore *record;
  GtkWidget *rec_dialog;

  if (win->n_column <= 0)
    return;
  position = gtk_single_selection_get_selected (win->singleselection);
  if (position == GTK_INVALID_LIST_POSITION)
    return;
  record = create_new_record (win);
  g_list_store_insert (win->liststore, position, record);
  ++win->n_row;

  rec_dialog = t_csv_rec_dialog_new (GTK_WINDOW (win), win->header, record, position);
  g_object_unref (record);
  g_signal_connect (rec_dialog, "response", G_CALLBACK (rec_dialog_response_cb), NULL);
  gtk_widget_show (rec_dialog);
}

static void
modify_field_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) { }

static void
open_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GtkWidget *dialog;

  if (win->file)
    return;
  dialog = gtk_file_chooser_dialog_new ("Open file", GTK_WINDOW (win), GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "Cancel", GTK_RESPONSE_CANCEL,
                                        "Open", GTK_RESPONSE_ACCEPT,
                                        NULL);
  g_signal_connect (dialog, "response", G_CALLBACK (open_dialog_response), win);
  gtk_widget_show (dialog);
}

static void
save_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  if (G_IS_FILE (win->file))
    t_csv_window_write (win, win->file);
  else
    t_csv_window_saveas (win);
}

static void
close_alert_response_cb (GtkDialog *alert, int response_id, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  gtk_window_destroy (GTK_WINDOW (alert));
  if (response_id == GTK_RESPONSE_ACCEPT)
    t_csv_window_close (win);
}

static void
close_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  TCsvAlert *alert;

  if (win->saved)
    t_csv_window_close (win);
  else {
    alert = T_CSV_ALERT (t_csv_alert_new (GTK_WINDOW (win)));
    t_csv_alert_set_message (alert, "Contents aren't saved yet.\nAre you sure to close?");
    t_csv_alert_set_button_label (alert, "Close");
    g_signal_connect (GTK_DIALOG (alert), "response", G_CALLBACK (close_alert_response_cb), win);
    gtk_widget_show (GTK_WIDGET (alert));
  }
}

static void
fld_dialog_response_cb (TCsvFldDialog *fld_dialog, int response_id, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (t_csv_fld_dialog_get_window (fld_dialog));
  GListStore *record;
  GtkColumnViewColumn *column;
  GtkListItemFactory *factory;
  GtkSorter *sorter;
  GtkExpression *params[1];
  GtkExpression *expression;
  int j;
  TStr *str;
  char *s;

  if (response_id == GTK_RESPONSE_ACCEPT) {
    win->header = t_csv_fld_dialog_get_record (fld_dialog);
    win->n_column = g_list_model_get_n_items (G_LIST_MODEL (win->header));
    record = create_new_record (win);
    g_list_store_append (win->liststore, record);
    g_object_unref (record);
    win->n_row = 1;
    win->file = NULL;
    win->saved = FALSE;
    for (j=0; j<win->n_column; ++j) {
      factory = gtk_builder_list_item_factory_new_from_bytes (NULL, get_bytes (j));
      str = g_list_model_get_item (G_LIST_MODEL (win->header), j);
      s = t_str_get_string (str);
      column = gtk_column_view_column_new (s, factory);
      g_free (s);
      g_object_unref (str);
 
      params[0] = gtk_constant_expression_new (G_TYPE_INT, j);
      expression = gtk_cclosure_expression_new (G_TYPE_STRING, NULL, 1, params, G_CALLBACK (get_string0), NULL, NULL);
      sorter = GTK_SORTER (gtk_string_sorter_new (expression));
      gtk_column_view_column_set_sorter (column, sorter);

      gtk_column_view_append_column (win->columnview, column);
    }
  }
  gtk_window_destroy (GTK_WINDOW (fld_dialog));
  busy = FALSE;
}

static void
new_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GtkWidget *fld_dialog;

  if (win->header)
    return;
  fld_dialog = t_csv_fld_dialog_new (GTK_WINDOW (win));
  g_signal_connect (fld_dialog, "response", G_CALLBACK (fld_dialog_response_cb), NULL);
  gtk_widget_show (fld_dialog);
}

static void
saveas_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);

  t_csv_window_saveas (win);
}

static void
pref_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  GtkWidget *pref;

  pref = t_csv_pref_new (GTK_WINDOW (win));
  gtk_widget_show (pref);
}

static void
quit_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  TCsvWindow *win = T_CSV_WINDOW (user_data);
  TCsvAlert *alert;

  if (win->saved)
    gtk_window_destroy (GTK_WINDOW (win));
  else {
    alert = T_CSV_ALERT (t_csv_alert_new (GTK_WINDOW (win)));
    t_csv_alert_set_message (alert, "Contents aren't saved yet.\nAre you sure to close?");
    t_csv_alert_set_button_label (alert, "Close");
    g_signal_connect (GTK_DIALOG (alert), "response", G_CALLBACK (alert_response_cb), win);
    gtk_widget_show (GTK_WIDGET (alert));
  }
}

/* gsettings changed::font signal handler */
static void
changed_font_cb (GSettings *settings, char *key, gpointer user_data) {
  GtkWindow *win = GTK_WINDOW (user_data); 
  const char *font;
  PangoFontDescription *pango_font_desc;

  font = g_settings_get_string (settings, "font");
  pango_font_desc = pango_font_description_from_string (font);
  set_font_for_display_with_pango_font_desc (win, pango_font_desc);
}

/* --- TCsvWindow object construction/destruction --- */ 
static void
t_csv_window_init (TCsvWindow *win) {
  GtkBuilder *build;
  GMenuModel *menu;

  win->header = NULL;
  win->n_row = win->n_column = 0;
  win->file = NULL;
  win->saved = TRUE;

  gtk_widget_init_template (GTK_WIDGET (win));

  build = gtk_builder_new_from_resource ("/com/github/ToshioCP/tcsv/menu.ui");
  menu = G_MENU_MODEL (gtk_builder_get_object (build, "menu"));
  gtk_menu_button_set_menu_model (win->btnm, menu);
  g_object_unref(build);

  win->settings = g_settings_new ("com.github.ToshioCP.tcsv");
  g_signal_connect (win->settings, "changed::font", G_CALLBACK (changed_font_cb), win);
  changed_font_cb(win->settings, "font", win);

/* ----- action ----- */
  const GActionEntry win_entries[] = {
    { "addrec", add_record_activated, NULL, NULL, NULL },
    { "rmrec", remove_record_activated, NULL, NULL, NULL },
    { "insrec", insert_record_activated, NULL, NULL, NULL },
    { "modify-field", modify_field_activated, NULL, NULL, NULL },
    { "new", new_activated, NULL, NULL, NULL },
    { "open", open_activated, NULL, NULL, NULL },
    { "save", save_activated, NULL, NULL, NULL },
    { "close", close_activated, NULL, NULL, NULL },
    { "saveas", saveas_activated, NULL, NULL, NULL },
    { "pref", pref_activated, NULL, NULL, NULL },
    { "quit", quit_activated, NULL, NULL, NULL },
  };
  g_action_map_add_action_entries (G_ACTION_MAP (win), win_entries, G_N_ELEMENTS (win_entries), win);

  g_signal_connect (win, "close-request", G_CALLBACK (before_close), NULL);
  g_signal_connect (win->columnview, "activate", G_CALLBACK (columnview_activate_cb), win);
}

static void
t_csv_window_class_init (TCsvWindowClass *class) {
/*  GObjectClass *object_class = G_OBJECT_CLASS (class);*/

  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvwindow.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, singleselection);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, sortlist);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, liststore);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, btnm);
}

GListStore *
t_csv_window_get_header (TCsvWindow *win) {
  return win->header;
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

