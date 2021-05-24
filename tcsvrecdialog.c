#include "tcsvrecdialog.h"
#include "css.h"
#include "tstr.h"

/* ----- Start of the definition of TCouple object -----*/
enum {
  PROP_0,
  PROP_FIRST,
  PROP_SECOND,
  N_PROPERTIES
};

static GParamSpec *couple_properties[N_PROPERTIES] = {NULL, };

typedef struct _TCouple {
  GObject parent;
  char *s1;
  char *s2;
} TCouple;

G_DEFINE_TYPE(TCouple, t_couple, G_TYPE_OBJECT)

static void
t_couple_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  TCouple *self = T_COUPLE (object);

  if (property_id == PROP_FIRST)
    self->s1 = g_strdup (g_value_get_string (value));
  else if (property_id == PROP_SECOND)
    self->s2 = g_strdup (g_value_get_string (value));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_couple_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  TCouple *self = T_COUPLE (object);

  if (property_id == PROP_FIRST)
    g_value_set_string (value, g_strdup (self->s1));
  else if (property_id == PROP_SECOND)
    g_value_set_string (value, g_strdup (self->s2));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_couple_init (TCouple *self) {
  self->s1 = self->s2 = NULL;
}

static void
t_couple_finalize (GObject *object) {
  TCouple *self = T_COUPLE (object);

  if (self->s1)
    g_clear_object (&self->s1);
  if (self->s2)
    g_clear_object (&self->s2);
  G_OBJECT_CLASS (t_couple_parent_class)->finalize (object);
}

static void
t_couple_class_init (TCoupleClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_couple_finalize;
  gobject_class->set_property = t_couple_set_property;
  gobject_class->get_property = t_couple_get_property;
  couple_properties[PROP_FIRST] = g_param_spec_string ("first-string", "str1", "first string", "", G_PARAM_READWRITE);
  couple_properties[PROP_SECOND] = g_param_spec_string ("second-string", "str2", "second string", "", G_PARAM_READWRITE);
  g_object_class_install_properties (gobject_class,N_PROPERTIES, couple_properties);
}

/* setter and getter */
void
t_couple_set_strings (TCouple *self, const char *s1, const char *s2) {
  g_object_set (self, "first-string", g_strdup (s1), NULL);
  g_object_set (self, "second-string", g_strdup (s2), NULL);
}

void
t_couple_get_strings (TCouple *self, char **s1, char **s2) {
  *s1 = g_strdup (self->s1);
  *s2 = g_strdup (self->s2);
  return;
}

/* create a new TCouple instance */
TCouple *
t_couple_new_with_strings (char *s1, char *s2) {
  return g_object_new (T_TYPE_COUPLE, "first-string", s1, "second-string", s2, NULL);
}
/* ----- End of the definition of TCouple object -----*/


struct _TCsvRecDialog
{
  GtkDialog parent;
  GtkColumnView *columnview;
  GtkColumnViewColumn *column2;
  GtkNoSelection *noselection;
  GListStore *liststore;
  GtkButton *btn_accept;
  GtkButton *btn_cancel;
  GSList *slist;
  GtkWindow *win;
  int position;
};

G_DEFINE_TYPE (TCsvRecDialog, t_csv_rec_dialog, GTK_TYPE_DIALOG);

GListStore *
t_csv_rec_dialog_get_record (TCsvRecDialog *rec_dialog) {
  GListStore *record;
  int i, n_items;
  TCouple *couple;
  char *s1, *s2;
  TStr *str;

  record = g_list_store_new (T_TYPE_STR);
  n_items = g_list_model_get_n_items (G_LIST_MODEL (rec_dialog->liststore));
  for (i=0; i<n_items; ++i) {
    couple = T_COUPLE (g_list_model_get_item (G_LIST_MODEL (rec_dialog->liststore), i));
    t_couple_get_strings (couple, &s1, &s2);
    str = t_str_new_with_string (s2);
    g_free (s1);
    g_free (s2);
    g_list_store_append (record, str);
  }
  return record;
}

GtkWindow *
t_csv_rec_dialog_get_window (TCsvRecDialog *rec_dialog) {
  return rec_dialog->win;
}

int
t_csv_rec_dialog_get_position (TCsvRecDialog *rec_dialog) {
  return rec_dialog->position;
}

static void
text_activate_cb (GtkText *text, gpointer user_data) {
  TCouple *couple = T_COUPLE (user_data);
  GtkEntryBuffer *buffer = gtk_text_get_buffer (text);

  g_object_set (couple, "second-string", gtk_entry_buffer_get_text (buffer), NULL);
}

/* closure used in GtkBuilderListItemFactory */
GStrv
get_css_class (GtkListItem *listitem, char *css) {
  char *s;
  char **header_class;

  s = g_new (char, strlen(css)+1);
  strcpy (s, css);
  header_class = g_new (char *, 2);
  header_class[0] = s;
  header_class[1] = NULL;
  return header_class; 
}

static void
setup_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  GtkWidget *text = gtk_text_new ();
  gtk_list_item_set_child (listitem, text);
}

typedef struct _Handlertbl Handlertbl;
struct _Handlertbl {
  GtkListItem *listitem;
  gulong handler_id;
};

static void
bind_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvRecDialog *rec_dialog = T_CSV_REC_DIALOG (user_data);
  GtkWidget *text = gtk_list_item_get_child (listitem);
  TCouple *couple = T_COUPLE (gtk_list_item_get_item (listitem));
  GtkEntryBuffer *buffer;
  char *s1, *s2;
  Handlertbl *table;

  if (T_IS_COUPLE (couple)) {
    buffer = gtk_text_get_buffer (GTK_TEXT (text));
    t_couple_get_strings (couple, &s1, &s2);
    gtk_entry_buffer_set_text (buffer, s2, -1);
    g_free (s1);
    g_free (s2);
    table = g_new (Handlertbl, 1);
    table->listitem = listitem;
    table->handler_id = g_signal_connect (GTK_TEXT (text), "activate", G_CALLBACK (text_activate_cb), couple);
    rec_dialog->slist = g_slist_append (rec_dialog->slist, table);
  }
}

static void
unbind_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  TCsvRecDialog *rec_dialog = T_CSV_REC_DIALOG (user_data);
  GtkWidget *text = gtk_list_item_get_child (listitem);
  GSList *slist;
  Handlertbl *table;

  for (slist = rec_dialog->slist; slist != NULL; slist = slist->next) {
    table = (Handlertbl *) slist->data;
    if (table->listitem == listitem) {
      g_signal_handler_disconnect (GTK_TEXT (text), table->handler_id);
      break;
    }
  }
  if (slist != NULL)
    rec_dialog->slist = g_slist_remove (rec_dialog->slist, table);
}

static void
teardown_cb (GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
  gtk_list_item_set_child (listitem, NULL);
}

static void
t_csv_rec_dialog_finalize (GObject *object) {
  TCsvRecDialog *rec_dialog = T_CSV_REC_DIALOG (object);

  if (rec_dialog->slist)
    g_slist_free_full (rec_dialog->slist, g_free);
  G_OBJECT_CLASS (t_csv_rec_dialog_parent_class)->finalize (object);
}

static void
t_csv_rec_dialog_init (TCsvRecDialog *rec_dialog) {
  gtk_widget_init_template (GTK_WIDGET (rec_dialog));
  rec_dialog->slist = NULL;
  GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
  gtk_column_view_column_set_factory (rec_dialog->column2, factory);
  g_signal_connect (factory, "setup", G_CALLBACK (setup_cb), NULL);
  g_signal_connect (factory, "bind", G_CALLBACK (bind_cb), rec_dialog);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind_cb), rec_dialog);
  g_signal_connect (factory, "teardown", G_CALLBACK (teardown_cb), NULL);
}

static void
t_csv_rec_dialog_class_init (TCsvRecDialogClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_csv_rec_dialog_finalize;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/com/github/ToshioCP/tcsv/tcsvrecdialog.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, columnview);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, column2);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, noselection);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, liststore);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, btn_accept);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, btn_cancel);
}

GtkWidget *
t_csv_rec_dialog_new (GtkWindow *win, GListStore *header, GListStore *record, int position) {
  g_return_val_if_fail (GTK_IS_WINDOW (win), NULL);
  g_return_val_if_fail (G_IS_LIST_STORE (header), NULL);
  g_return_val_if_fail (G_IS_LIST_STORE (record), NULL);

  TCsvRecDialog *rec_dialog;
  int j, n_items;
  TCouple *couple;
  TStr *str1, *str2;
  char *s1, *s2;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (header));
  if (n_items != g_list_model_get_n_items (G_LIST_MODEL (record)))
    return NULL;

  rec_dialog = T_CSV_REC_DIALOG (g_object_new (T_TYPE_CSV_REC_DIALOG, "transient-for", win, NULL));
  for (j=0; j<n_items; ++j) {
    str1 = T_STR (g_list_model_get_item (G_LIST_MODEL (header), j));
    str2 = T_STR (g_list_model_get_item (G_LIST_MODEL (record), j));
    s1 = t_str_get_string (str1);
    s2 = t_str_get_string (str2);
    couple = t_couple_new_with_strings (s1, s2);
    g_list_store_append (rec_dialog->liststore, couple);
    g_free (s1);
    g_free (s2);
    g_object_unref (str1);
    g_object_unref (str2);
  }
  rec_dialog->win = win;
  rec_dialog->position = position;
  set_css_for_display (GTK_WINDOW (rec_dialog), "text:focus {border: 1px solid gray;} label.header {background: #e8e8e8;}");

  return GTK_WIDGET (rec_dialog);
}

