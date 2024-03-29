#include <gtk/gtk.h>
#include "tmodify.h"

enum {
  PROP_0,
  PROP_OLD_POSITION,
  PROP_NEW_POSITION,
  PROP_OLD_STRING,
  PROP_NEW_STRING,
  N_PROPERTIES
};

static GParamSpec *modify_properties[N_PROPERTIES] = {NULL, };

typedef struct _TModify {
  GObject parent;
  GtkListItem *listitem;
  int old_position;
  int new_position;
  char *old_string;
  char *new_string;
} TModify;

G_DEFINE_TYPE(TModify, t_modify, G_TYPE_OBJECT)

static void
t_modify_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  TModify *self = T_MODIFY (object);

  if (property_id == PROP_OLD_POSITION)
    self->old_position = g_value_get_int (value);
  else if (property_id == PROP_NEW_POSITION)
    self->new_position = g_value_get_int (value);
  else if (property_id == PROP_OLD_STRING)
    self->old_string = g_strdup (g_value_get_string (value));
  else if (property_id == PROP_NEW_STRING)
    self->new_string = g_strdup (g_value_get_string (value));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_modify_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  TModify *self = T_MODIFY (object);

  if (property_id == PROP_OLD_POSITION)
    g_value_set_int (value, self->old_position);
  else if (property_id == PROP_NEW_POSITION)
    g_value_set_int (value, self->new_position);
  else if (property_id == PROP_OLD_STRING)
    g_value_set_string (value, g_strdup (self->old_string));
  else if (property_id == PROP_NEW_STRING)
    g_value_set_string (value, g_strdup (self->new_string));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_modify_init (TModify *self) {
  self->listitem = NULL;
  self->old_position = self->new_position = -1;
  self->old_string = self->new_string = NULL;
}

static void
t_modify_dispose (GObject *object) {
  TModify *self = T_MODIFY (object);

  if (self->listitem)
    g_clear_object (&self->listitem);
  G_OBJECT_CLASS (t_modify_parent_class)->dispose (object);
}

static void
t_modify_finalize (GObject *object) {
  TModify *self = T_MODIFY (object);

  if (self->old_string)
    g_free (self->old_string);
  if (self->new_string)
    g_free (self->new_string);
  G_OBJECT_CLASS (t_modify_parent_class)->finalize (object);
}

static void
t_modify_class_init (TModifyClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_modify_dispose;
  gobject_class->finalize = t_modify_finalize;
  gobject_class->set_property = t_modify_set_property;
  gobject_class->get_property = t_modify_get_property;
  modify_properties[PROP_OLD_POSITION] = g_param_spec_int ("old-position", "old-pos", "old field position", -1, G_MAXINT, -1, G_PARAM_READWRITE);
  modify_properties[PROP_NEW_POSITION] = g_param_spec_int ("new-position", "new-pos", "new field position", -1, G_MAXINT, -1, G_PARAM_READWRITE);
  modify_properties[PROP_OLD_STRING] = g_param_spec_string ("old-string", "old-str", "old string", "", G_PARAM_READWRITE);
  modify_properties[PROP_NEW_STRING] = g_param_spec_string ("new-string", "new-str", "new string", "", G_PARAM_READWRITE);
  g_object_class_install_properties (gobject_class,N_PROPERTIES, modify_properties);
}

/* setter and getter */
void
t_modify_set_list_item (TModify *self, GtkListItem *listitem) {
  g_return_if_fail (T_IS_MODIFY (self));
  g_return_if_fail (GTK_IS_LIST_ITEM (listitem) || listitem == NULL);
  self->listitem = listitem ? g_object_ref (listitem) : NULL;
}

void
t_modify_set_old_position (TModify *self, int old_position) {
  g_return_if_fail (T_IS_MODIFY (self));
  self->old_position = old_position;
}

void
t_modify_set_new_position (TModify *self, int new_position) {
  g_return_if_fail (T_IS_MODIFY (self));
  self->new_position = new_position;
}

void
t_modify_set_old_string (TModify *self, const char *old_string) {
  g_return_if_fail (T_IS_MODIFY (self));
  if (self->old_string)
    g_free (self->old_string);
  self->old_string = g_strdup (old_string);
}

void
t_modify_set_new_string (TModify *self, const char *new_string) {
  g_return_if_fail (T_IS_MODIFY (self));
  if (self->new_string)
    g_free (self->new_string);
  self->new_string = g_strdup (new_string);
}

GtkListItem *
t_modify_get_list_item (TModify *self) {
  g_return_val_if_fail (T_IS_MODIFY (self), NULL);
  return self->listitem ? g_object_ref (self->listitem) : NULL;
}

int
t_modify_get_old_position (TModify *self) {
  g_return_val_if_fail (T_IS_MODIFY (self), -2);
  return self->old_position;
}

int
t_modify_get_new_position (TModify *self) {
  g_return_val_if_fail (T_IS_MODIFY (self), -2);
  return self->new_position;
}

char *
t_modify_get_old_string (TModify *self) {
  g_return_val_if_fail (T_IS_MODIFY (self), NULL);
  return g_strdup (self->old_string);
}

char *
t_modify_get_new_string (TModify *self) {
  g_return_val_if_fail (T_IS_MODIFY (self), NULL);
  return g_strdup (self->new_string);
}

const char *
t_modify_look_old_string (TModify *self) {
  g_return_val_if_fail (T_IS_MODIFY (self), NULL);
  return self->old_string;
}

const char *
t_modify_look_new_string (TModify *self) {
  g_return_val_if_fail (T_IS_MODIFY (self), NULL);
  return self->new_string;
}

/* exchanges data without listitem */
void
t_modify_swap_data (TModify *self, TModify *other) {
  g_return_if_fail (T_IS_MODIFY (self));
  g_return_if_fail (T_IS_MODIFY (other));

  int i;
  char *s;

  i = self->old_position;
  g_object_set (self, "old-position", t_modify_get_old_position (other), NULL);
  g_object_set (other, "old-position", i, NULL);

  i = self->new_position;
  g_object_set (self, "new-position", t_modify_get_new_position (other), NULL);
  g_object_set (other, "new-position", i, NULL);

  s = g_strdup (self->old_string);
  g_object_set (self, "old-string", t_modify_look_old_string (other), NULL);
  g_object_set (other, "old-string", s, NULL);
  g_free (s);

  s = g_strdup (self->new_string);
  g_object_set (self, "new-string", t_modify_look_new_string (other), NULL);
  g_object_set (other, "new-string", s, NULL);
  g_free (s);
}

/* create a new TModify instance */
TModify *
t_modify_new_with_data (int old_position, int new_position, const char *old_string, const char *new_string) {
  return T_MODIFY (g_object_new (T_TYPE_MODIFY, "old-position", old_position,
                                                "new-position", new_position,
                                                "old-string", old_string,
                                                "new-string", new_string, NULL));
}

TModify *
t_modify_new (void) {
  return T_MODIFY (g_object_new (T_TYPE_MODIFY, NULL));
}
