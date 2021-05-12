#include "tcsvwindow.h"
#include "tcsvalert.h"
#include "tcsvpref.h"
#include "tcsv.h"
#include "css.h"

struct _TCsvWindow {
  GtkApplicationWindow parent;
  GtkColumnView *columnview;
  GtkSortListModel *sortlist;
  GListStore *liststore;
  GtkStringList *header;
  GtkMenuButton *btnm;
  GFile *file;
  int n_row;
  int n_column;
  gboolean saved;
  GSettings *settings;
};

G_DEFINE_TYPE (TCsvWindow, t_csv_window, GTK_TYPE_APPLICATION_WINDOW);

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

char *
get_string (GtkListItem *item, GtkStringList *stringlist, int i) {
  if (GTK_IS_STRING_LIST (stringlist))
    return g_strdup (gtk_string_list_get_string (stringlist, i));
  else
    return NULL;
}

GBytes *
get_bytes (int position) {
  char *data;
  gsize size;

  char *format = 
"<?xml version='1.0' encoding='UTF-8'?>"
"<interface>"
  "<template class='GtkListItem'>"
    "<property name='child'>"
      "<object class='GtkLabel'>"
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

static void
t_csv_column_view_init (GtkColumnView *columnview, GListStore *liststore, GtkStringList *header, int n_row, int n_column) {
  g_return_if_fail (GTK_IS_COLUMN_VIEW (columnview));
  g_return_if_fail (G_IS_LIST_STORE (liststore));
  g_return_if_fail (GTK_IS_STRING_LIST (header));

  GtkColumnViewColumn *column;
  GtkListItemFactory *factory;
  int j;

  for (j=0; j<n_column; ++j) {
    factory = gtk_builder_list_item_factory_new_from_bytes (NULL, get_bytes (j));
    column = gtk_column_view_column_new (gtk_string_list_get_string (header, j), factory);
    gtk_column_view_append_column (columnview, column);
  }
}

static void
t_csv_window_read (TCsvWindow *win, GFile *file) {
  GtkWidget *message_dialog;
  GError *err = NULL;

  if (! csv_read (win->liststore, file, &win->n_row, &win->n_column, &err)) {
    message_dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                            "%s.\n", err->message);
    g_signal_connect (message_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_widget_show (message_dialog);
    g_error_free (err);
  } else {
    win->file = file;
    win->header = GTK_STRING_LIST (g_list_model_get_item (G_LIST_MODEL (win->liststore), 0));
    g_list_store_remove (win->liststore, 0);
    --win->n_row;
    t_csv_column_view_init (win->columnview, win->liststore, win->header, win->n_row, win->n_column);
  }
}

/* ----- read/write functions ----- */

static void
open_dialog_response(GtkWidget *dialog, gint response, TCsvWindow *win) {
  GFile *file;

  if (response == GTK_RESPONSE_ACCEPT
      && G_IS_FILE (file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog))))
    t_csv_window_read (win, file);
  g_object_unref (file);
  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
t_csv_window_write (TCsvWindow *win, GFile *file) {
  GError *err = NULL;
  GtkWidget *message_dialog;

  g_list_store_insert (win->liststore, 0, win->header);
  if (! csv_write (win->liststore, file, win->n_row + 1, win->n_column, &err)) {
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
  g_object_unref (file);
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
add_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) { }
static void
remove_record_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) { }
static void
add_field_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) { }
static void
remove_field_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) { }

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
close_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
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

  win->file = NULL;
  win->n_row = win->n_column = 0;
  win->saved = TRUE;

  gtk_widget_init_template (GTK_WIDGET (win));
  win->liststore = g_list_store_new (GTK_TYPE_STRING_LIST);
  gtk_sort_list_model_set_model (win->sortlist, G_LIST_MODEL (win->liststore));

  build = gtk_builder_new_from_resource ("/com/github/ToshioCP/tcsv/menu.ui");
  menu = G_MENU_MODEL (gtk_builder_get_object (build, "menu"));
  gtk_menu_button_set_menu_model (win->btnm, menu);
  g_object_unref(build);

  win->settings = g_settings_new ("com.github.ToshioCP.tcsv");
  g_signal_connect (win->settings, "changed::font", G_CALLBACK (changed_font_cb), win);

/* ----- action ----- */
  const GActionEntry win_entries[] = {
    { "addrec", add_record_activated, NULL, NULL, NULL },
    { "rmrec", remove_record_activated, NULL, NULL, NULL },
    { "addfld", add_field_activated, NULL, NULL, NULL },
    { "rmfld", remove_field_activated, NULL, NULL, NULL },
    { "open", open_activated, NULL, NULL, NULL },
    { "save", save_activated, NULL, NULL, NULL },
    { "close", close_activated, NULL, NULL, NULL },
    { "saveas", saveas_activated, NULL, NULL, NULL },
    { "pref", pref_activated, NULL, NULL, NULL },
  };
  g_action_map_add_action_entries (G_ACTION_MAP (win), win_entries, G_N_ELEMENTS (win_entries), win);

  g_signal_connect (win, "close-request", G_CALLBACK (before_close), NULL);

  changed_font_cb(win->settings, "font", win);
}

static void
t_csv_window_class_init (TCsvWindowClass *class) {
/*  GObjectClass *object_class = G_OBJECT_CLASS (class);*/

  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvwindow.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, btnm);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvWindow, sortlist);
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

