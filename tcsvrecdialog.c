#include "tcsvrecdialog.h"
#include "css.h"

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
t_couple_class_init (TCoupleClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

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
  TCsvWindow *win;
  int position;
};

G_DEFINE_TYPE (TCsvRecDialog, t_csv_rec_dialog, GTK_TYPE_DIALOG);

GtkStringList *
t_csv_rec_dialog_get_string_list (TCsvRecDialog *rec_dialog) {
  GtkStringList *stringlist;
  int j, n_items;
  TCouple *couple;
  char *s1, *s2;

  stringlist = gtk_string_list_new (NULL);
  n_items = g_list_model_get_n_items (G_LIST_MODEL (rec_dialog->liststore));
  for (j=0; j<n_items; ++j) {
    couple = T_COUPLE (g_list_model_get_item (G_LIST_MODEL (rec_dialog->liststore), j));
    t_couple_get_strings (couple, &s1, &s2);
    gtk_string_list_take (stringlist, s2);
    g_free (s1);
  }
  return stringlist;
}

TCsvWindow *
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

  if (GTK_IS_TEXT (text)) {
    buffer = gtk_text_get_buffer (GTK_TEXT (text));
    t_couple_get_strings (couple, &s1, &s2);
    gtk_entry_buffer_set_text (buffer, s2, -1);
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
  int j, n_items;
  TCouple *couple;
  char *s1, *s2;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (rec_dialog->liststore));
  for (j=0; j<n_items; ++j) {
    couple = T_COUPLE (g_list_model_get_item (G_LIST_MODEL (rec_dialog->liststore), j));
    t_couple_get_strings (couple, &s1, &s2);
    g_clear_pointer (&s1, g_free);
    g_clear_pointer (&s2, g_free);
  }
  g_clear_object (&rec_dialog->liststore);
  G_OBJECT_CLASS (t_csv_rec_dialog_parent_class)->finalize (object);
}

static void
t_csv_rec_dialog_init (TCsvRecDialog *rec_dialog) {
  gtk_widget_init_template (GTK_WIDGET (rec_dialog));
  rec_dialog->liststore = g_list_store_new (T_TYPE_COUPLE);
  gtk_no_selection_set_model (rec_dialog->noselection, G_LIST_MODEL (rec_dialog->liststore));
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
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, btn_accept);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), TCsvRecDialog, btn_cancel);
}

GtkWidget *
t_csv_rec_dialog_new (GtkWindow *win, GtkStringList *header, GtkStringList *stringlist, int position) {
  g_return_val_if_fail (GTK_IS_WINDOW (win), NULL);
  g_return_val_if_fail (GTK_IS_STRING_LIST (header), NULL);
  g_return_val_if_fail (GTK_IS_STRING_LIST (stringlist), NULL);

  TCsvRecDialog *rec_dialog;
  int j, n_items;
  TCouple *couple;

  n_items = g_list_model_get_n_items (G_LIST_MODEL (header));
  if (n_items != g_list_model_get_n_items (G_LIST_MODEL (stringlist)))
    return NULL;

  rec_dialog = T_CSV_REC_DIALOG (g_object_new (T_TYPE_CSV_REC_DIALOG, "transient-for", win, NULL));
  for (j=0; j<n_items; ++j) {
    couple = t_couple_new_with_strings (g_strdup (gtk_string_list_get_string (header, j)),
                                         g_strdup (gtk_string_list_get_string (stringlist, j)));
    g_list_store_append (rec_dialog->liststore, couple);
  }
  rec_dialog->win = T_CSV_WINDOW (win);
  rec_dialog->position = position;
  set_css_for_display (GTK_WINDOW (rec_dialog), "text:focus-within {border: 1px solid gray;}");

  return GTK_WIDGET (rec_dialog);
}

