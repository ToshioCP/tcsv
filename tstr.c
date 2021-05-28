#include <glib-object.h>
#include "tstr.h"

enum {
  PROP_0,
  PROP_STRING,
  N_PROPERTIES
};

static GParamSpec *couple_properties[N_PROPERTIES] = {NULL, };

typedef struct _TStr {
  GObject parent;
  char *string;
} TStr;

G_DEFINE_TYPE(TStr, t_str, G_TYPE_OBJECT)

static void
t_str_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  TStr *self = T_STR (object);

  if (property_id == PROP_STRING)
    self->string = g_strdup (g_value_get_string (value));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_str_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  TStr *self = T_STR (object);

  if (property_id == PROP_STRING)
    g_value_set_string (value, g_strdup (self->string));
  else
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
t_str_finalize (GObject *object) {
  TStr *self = T_STR (object);

  if (self->string)
    g_free (self->string);
  G_OBJECT_CLASS (t_str_parent_class)->finalize (object);
}

static void
t_str_init (TStr *self) {
  self->string = NULL;
}

static void
t_str_class_init (TStrClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = t_str_finalize;
  gobject_class->set_property = t_str_set_property;
  gobject_class->get_property = t_str_get_property;
  couple_properties[PROP_STRING] = g_param_spec_string ("string", "str", "string", "", G_PARAM_READWRITE);
  g_object_class_install_properties (gobject_class,N_PROPERTIES, couple_properties);
}

/* setter and getter */
void
t_str_set_string (TStr *self, const char *s) {
  g_object_set (self, "string", g_strdup (s), NULL);
}

char *
t_str_get_string (TStr *self) {
  return g_strdup (self->string);
}

char *
t_str_look_string (TStr *self) {
  return self->string;
}

/* create a new TStr instance */
TStr *
t_str_new_with_string (const char *s) {
  return T_STR (g_object_new (T_TYPE_STR, "string", s, NULL));
}

TStr *
t_str_new (void) {
  return T_STR (g_object_new (T_TYPE_STR, NULL));
}

