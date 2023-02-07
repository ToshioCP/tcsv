#include <gtk/gtk.h>
#include "tcsvstr.h"

/* TCsvStr is a wrapper of a string */

enum {
  PROP_0,
  PROP_STRING,
  N_PROPERTIES
};

static GParamSpec *str_properties[N_PROPERTIES] = {NULL, };

struct _TCsvStr{
  GObject parent;
  char *string; // string in not NULL.
};

G_DEFINE_TYPE (TCsvStr, t_csv_str, G_TYPE_OBJECT)

static void
t_csv_str_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  TCsvStr *self = T_CSV_STR (object);

  if (property_id == PROP_STRING) {
    self->string = g_strdup (g_value_get_string (value));
  } else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_csv_str_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  TCsvStr *self = T_CSV_STR (object);

  if (property_id == PROP_STRING)
    g_value_set_string (value, g_strdup (self->string));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

/* setter and getter */

// s must not NULL
void
t_csv_str_set_string (TCsvStr *self, const char *s) {
  g_return_if_fail (T_IS_CSV_STR (self));
  g_return_if_fail (s != NULL);
  if (self->string)
    g_free (self->string);
  self->string = g_strdup (s);
}

// s must not NULL
void
t_csv_str_take_string (TCsvStr *self, char *s) {
  g_return_if_fail (T_IS_CSV_STR (self));
  g_return_if_fail (s != NULL);
  if (self->string)
    g_free (self->string);
  self->string = s;
}

char *
t_csv_str_get_string (TCsvStr *self) {
  g_return_val_if_fail (T_IS_CSV_STR (self), NULL);
  return g_strdup (self->string);
}

const char *
t_csv_str_look_string (TCsvStr *self) {
  g_return_val_if_fail (T_IS_CSV_STR (self), NULL);
  return self->string;
}

static void
t_csv_str_finalize (GObject *object) {
  TCsvStr *self = T_CSV_STR (object);
  if (self->string)
    g_free (self->string);
  G_OBJECT_CLASS (t_csv_str_parent_class)->finalize (object);
}

static void
t_csv_str_init (TCsvStr *self) {
  self->string = NULL;
}

static void
t_csv_str_class_init (TCsvStrClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_csv_str_finalize;
  gobject_class->set_property = t_csv_str_set_property;
  gobject_class->get_property = t_csv_str_get_property;
  str_properties[PROP_STRING] = g_param_spec_string ("string", "str", "string", "", G_PARAM_READWRITE);
  g_object_class_install_properties (gobject_class, N_PROPERTIES, str_properties);
}

/* create a new TCsvStr instance */
// s must not NULL
TCsvStr *
t_csv_str_new_with_string (const char *s) {
  g_return_val_if_fail (s != NULL, NULL);
  return T_CSV_STR (g_object_new (T_TYPE_CSV_STR, "string", s, NULL));
}

TCsvStr *
t_csv_str_new (void) {
  return T_CSV_STR (g_object_new (T_TYPE_CSV_STR, "string", "", NULL));
}
