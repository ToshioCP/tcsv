#include <string.h>
#include "tcsv.h"
#include "tstr.h"

/* program error */
enum {
  state_error,
  record_overflow,
  temp_buf_overflow,
  csv_buf_overflow,
  arg_error,
  end_of_prog_error
};

static char *prog_error_message[] = {
  "Program error: state is out of the range (1-7)\n",
  "Program error: record overflow\n",
  "Program error: temporary buffer overflow\n",
  "Program error: csv buffer overflow\n",
  "Program error: argument error\n"
};

static char *
csv_prog_error_message (int csv_err) {
  if (csv_err >= 0 && csv_err < end_of_prog_error)
    return prog_error_message[csv_err];
  else
    return NULL;
}

/* Runtime error */

typedef struct {
  char *detail;
} TCsvErrorPrivate;

static void
t_csv_error_private_init (TCsvErrorPrivate *priv) {
  priv->detail = NULL;
}

static void
t_csv_error_private_copy (const TCsvErrorPrivate *src_priv, TCsvErrorPrivate *dst_priv) {
  dst_priv->detail = g_strdup (src_priv->detail);
}

static void
t_csv_error_private_clear (TCsvErrorPrivate *priv) {
  if (priv->detail) {
    g_free (priv->detail);
    priv->detail = NULL;
  }
}

G_DEFINE_EXTENDED_ERROR (TCsvError, t_csv_error)

const char *
t_csv_error_get_detail (GError *error) {
  g_return_val_if_fail (error != NULL, NULL);  
  TCsvErrorPrivate *priv = t_csv_error_get_private (error);
  g_return_val_if_fail (priv != NULL, NULL);
  g_return_val_if_fail (error->code != T_CSV_ERROR_DIFF_NFIELDS
                     || error->code != T_CSV_ERROR_SYNTAX_ERROR
                     || error->code != T_CSV_ERROR_NO_STRING, NULL);
  return priv->detail;
}

static void
t_csv_error_set_diff_nfields (GError **error, int line, int column, const char *detail) {
  TCsvErrorPrivate *priv;

  g_set_error (error, T_CSV_ERROR, T_CSV_ERROR_DIFF_NFIELDS, "Csv error: different number of fields at line %d, column %d", line, column);
  priv = t_csv_error_get_private (*error);
  priv->detail = g_strdup (detail);
}

static void
t_csv_error_set_syntax_error (GError **error, int line, int column, const char *detail) {
  TCsvErrorPrivate *priv;

  g_set_error (error, T_CSV_ERROR, T_CSV_ERROR_SYNTAX_ERROR, "Csv error: syntax error at line %d, column %d", line, column);
  priv = t_csv_error_get_private (*error);
  priv->detail = g_strdup (detail);
}

static void
t_csv_error_set_no_string (GError **error, const char *detail) {
  TCsvErrorPrivate *priv;

  g_set_error (error, T_CSV_ERROR, T_CSV_ERROR_NO_STRING, "Csv error: No string to build csv");
  priv = t_csv_error_get_private (*error);
  priv->detail = g_strdup (detail);
}
/* end of runtime error */

static int tbl[7][5] = {
  {6, 5, 2, 1, 7},
  {6, 5, 1, 1, 7},
  {3, 3, 4, 3, -1},
  {3, 3, 4, 3, -1},
  {6, 5, 3, -1, 7},
  {6, 5, 2, 1, 7},
  {6, 5, 2, 1, 7}
};

static gboolean
csv_real_validate (char *contents, size_t length, int *n_row, int *n_column, GError **err) {
  char *pt;
  char c;
  int state = 0, input;
  int nfield0 = 0;
  int nfield = 0;
  int line, column; /* the location of the current character */

  *n_row = 0; /* row counter */
  line = column = 1;

  for (pt = contents; ; ++pt) {
    if (pt >= contents + length)
      input = 4; /* EOF */
    else if ((c = *pt) == '\0')
      input = 4;
    else if (c == '\n')
      input = 0;
    else if (c == ',')
      input = 1;
    else if (c == '"')
      input = 2;
    else
      input = 3;

    if (state != 6 && input == 4) /* EOF comes after non NL character. */
      *n_row += 1;
    if (state == 6 && input == 4)
      break;

    state = tbl[state][input]; /* Now, this is the state of current character */

    if (state == 6) { /* NL */
      *n_row += 1;
      ++line;
      column = 1;
    } else if (0 < state && state < 7)
      ++column;
    if (state == 6 || state == 7) { /* New line or (EOF after non NL) */
      ++nfield;
      nfield0 = (nfield0 == 0) ? nfield : nfield0;
      if (nfield0 != nfield) {
        if (err != NULL)
          t_csv_error_set_diff_nfields (err, line - 1, 1, NULL); /* the previous line has different number of fields */
        return FALSE;
      }
      nfield = 0;
    }
    if (state == 5) /* Comma */
      ++nfield;
    if (state < 0) {
      if (err != NULL)
        t_csv_error_set_syntax_error (err, line, column, NULL);
      return FALSE;
    }
    if (state > 7) {
      g_error ("%s\n", csv_prog_error_message (state_error));
      return FALSE;
    }
    if (state == 7) /* EOF */
      break;
  }
  *n_column = nfield0;
  return TRUE;
}

/* This version stores data into TStr which is the item of GListStore (record). */
gboolean
csv_real_read (GListStore *liststore, const char *contents, size_t length, int n_row, int n_column, GError **err) {
  char *pt;
  char c;
  int state = 0, input;

#define MAXBUF 4096
  char buf[MAXBUF];
  int bi; /* buf index */
  int i, j;
  int line, column; /* the location of the current character */
  GListStore *record = NULL;
  TStr *str;

  i = j = 0;
  bi = 0;

  for (pt = (char *) contents; ; ++pt) {
    if (pt >= contents + length)
      input = 4; /* EOF */
    else if ((c = *pt) == '\0')
      input = 4;
    else if (c == '\n')
      input = 0;
    else if (c == ',')
      input = 1;
    else if (c == '"')
      input = 2;
    else
      input = 3;

    if (state == 6 && input == 4)
      break;

    state = tbl[state][input]; /* Now, this is the state of current character */
    if (state == 5 || state == 6 || state == 7) {
      if (i >= n_row) {
        g_error ("%s\n", csv_prog_error_message (record_overflow));
        if (record)
          g_object_unref (record);
        g_list_store_remove_all (liststore);
        return FALSE;
      }
      buf[bi] = '\0';
      if (record == NULL) {
        record = g_list_store_new (T_TYPE_STR);
        g_list_store_append (liststore, record);
      }
      str = t_str_new_with_string (buf);
      g_list_store_append (record, str);
      bi = 0;
      if (++j >= n_column) {
        j = 0;
        ++i;
      }
    }
    if (state == 6) { /* NL */
      ++line;
      column = 1;
      record = NULL;
    } else if (0 < state && state < 7)
      ++column;
    if (state == 1 || state == 3) { /* in the fieldL */
      if (MAXBUF - bi > 2) /* buf has enough space to insert a char. */
        buf[bi++] = c;
      else {
        g_error ("%s\n", csv_prog_error_message (temp_buf_overflow));
        if (record)
          g_object_unref (record);
        g_list_store_remove_all (liststore);
        return FALSE;
      }
    }
    if (state < 0) {
      if (err != NULL)
        t_csv_error_set_syntax_error (err, line, column, NULL);
      if (record)
        g_object_unref (record);
      g_list_store_remove_all (liststore);
      return FALSE;
    }
    if (state > 7) {
      g_error ("%s\n", csv_prog_error_message (state_error));
      if (record)
        g_object_unref (record);
      g_list_store_remove_all (liststore);
      return FALSE;
    }
    if (state == 7) { /* EOF */
      record = NULL;
      break;
    }
  }
  return TRUE;
}

static char *
csv_real_write (GListStore *liststore, int n_row, int n_column, size_t *length, GError **err) {
  char c;
  int i, j, n_nl, n_comma, n_quote;
  char *s;
  char *t, *csv, *pt;
  GListStore *record;
  TStr *str;

  *length = 0;
  for (i=0; i<n_row; ++i)
    for (j=0; j<n_column; ++j) {
      record = G_LIST_STORE (g_list_model_get_item (G_LIST_MODEL (liststore), i));
      if (G_IS_LIST_STORE (record)
         && T_IS_STR (str = T_STR (g_list_model_get_item (G_LIST_MODEL (record), j)))
         && (s = t_str_get_string (str)) != NULL) {
        for (t = s, n_nl = n_comma = n_quote = 0; *t; ++t) {
          if ((c = *t) == '\n')
            ++n_nl;
          else if (c == ',')
            ++n_comma;
          else if (c == '"')
            ++n_quote;
        }
        if (n_nl == 0 && n_comma == 0 && n_quote == 0)
          *length += strlen (s) + 1;
        else
          *length += strlen (s) + n_quote + 2 + 1;
      } else {
        t_csv_error_set_no_string (err, NULL);
        return NULL;
      }
      if (s) g_free (s);
      if (str) g_clear_object (&str);
    }
  ++(*length); /* EOS */
  csv = g_new0 (char, *length);
  for (i=0, pt=csv; i<n_row; ++i) {
    for (j=0; j<n_column; ++j) {
      record = G_LIST_STORE (g_list_model_get_item (G_LIST_MODEL (liststore), i));
      if (G_IS_LIST_STORE (record)
         && T_IS_STR (str = T_STR (g_list_model_get_item (G_LIST_MODEL (record), j)))
         && (s = t_str_get_string (str)) != NULL) {
        for (t = s, n_nl = n_comma = n_quote = 0; *t; ++t) {
          if ((c = *t) == '\n')
            ++n_nl;
          else if (c == ',')
            ++n_comma;
          else if (c == '"')
            ++n_quote;
        }
        if (n_nl == 0 && n_comma == 0 && n_quote == 0) {
          if (csv + *length <= pt + strlen (s) + 1)
            g_error ("%s\n", csv_prog_error_message (csv_buf_overflow));
          else {
            for (t = (char *) s; (*pt = *t); ++pt, ++t)
              ;
            *pt++ = ',';
          }
        } else
          if (csv + *length <= pt + strlen (s) + n_quote + 2 + 1)
            g_error ("%s\n", csv_prog_error_message (csv_buf_overflow));
          else {
            *pt++ = '"';
            for (t = s; *t; ++pt, ++t) {
              if (*t == '"') {
                *pt++ = '"';
                *pt = '"';
              } else
                *pt = *t;
            }
            *pt++ = '"';
            *pt++ = ',';
          }
      } else {
        t_csv_error_set_no_string (err, NULL);
        g_free (csv);
        return NULL;
      }
      if (s) g_free (s);
      if (str) g_clear_object (&str);
      g_object_unref (record);
    }
    if (pt > csv)
      *(pt-1) = '\n';
  }
  if (csv + *length <= pt) {
    g_error ("%s\n", csv_prog_error_message (csv_buf_overflow));
    g_free (csv);
    csv = NULL;
  } else
    *pt = '\0';
  return csv;
}

gboolean
csv_validate (GFile *file, GError **err) {
  g_return_val_if_fail (err == NULL || (err != NULL && *err == NULL), FALSE);

  char *contents;
  size_t length;
  int state;
  int n_row, n_column;

  if (! g_file_load_contents (file, NULL, &contents, &length, NULL, err))
    return FALSE;
  state = csv_real_validate (contents, length, &n_row, &n_column, err);
  g_free (contents);
  return state;
}

gboolean
csv_read (GListStore *liststore, GFile *file, int *n_row, int *n_column, GError **err) {
  g_return_val_if_fail (G_IS_LIST_STORE (liststore), FALSE);
  g_return_val_if_fail (G_IS_FILE (file), FALSE);
  g_return_val_if_fail (err == NULL || (err != NULL && *err == NULL), FALSE);

  char *contents;
  size_t length;
  gboolean state;

  if (! g_file_load_contents (file, NULL, &contents, &length, NULL, err))
    return FALSE;
  state = csv_real_validate (contents, length, n_row, n_column, err);
  if (! state) {
    free (contents);
    return FALSE;
  }
  state = csv_real_read (liststore, contents, length, *n_row, *n_column, err);
  free (contents);
  return state;
}

gboolean
csv_write (GListStore *liststore, GFile *file, int n_row, int n_column, GError **err) {
  g_return_val_if_fail (G_IS_LIST_STORE (liststore), FALSE);
  g_return_val_if_fail (G_IS_FILE (file), FALSE);
  g_return_val_if_fail (err == NULL || (err != NULL && *err == NULL), FALSE);  

  char *contents;
  size_t length;
  gboolean state;

  contents = csv_real_write (liststore, n_row, n_column, &length, err);
  if (contents == NULL)
    return FALSE;
  state = g_file_replace_contents (file, contents, length - 1, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, err);
  g_free (contents);
  return state;
}

