#include <gio/gio.h>
#include <string.h>
#include "csv.h"

/* Two dimensional array */
char ***
array_alloc (int n_i, int n_j) {
  char ***array;
  char i;

  array = g_new0 (char **, n_i);
  for (i = 0; i < n_i; ++i)
    *(array + i) = g_new0 (char *, n_j);
  return array;
}

void
array_free (char ***array, int n_i, int n_j) {
  int i, j;

  for (i=0; i<n_i; ++i) {
    for (j=0; j<n_j; ++j)
      if (*(*(array+i)+j) != NULL)
        g_free (*(*(array+i)+j));
    g_free (*(array + i));
  }
  g_free (array);
  array = NULL;
}

void
array_put (char ***array, int i, int j, char *s) {
  *(*(array+i)+j) = s;
}

char *
array_get (char ***array, int i, int j) {
  return *(*(array+i)+j);
}

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
t_csv_error_get_diff_nfields_detail (GError *error)
{
  g_return_val_if_fail (error != NULL, NULL);  
  TCsvErrorPrivate *priv = t_csv_error_get_private (error);
  g_return_val_if_fail (priv != NULL, NULL);
  g_return_val_if_fail (error->code != T_CSV_ERROR_DIFF_NFIELDS, NULL);
  return priv->detail;
}

const char *
t_csv_error_get_syntax_error_detail (GError *error)
{
  g_return_val_if_fail (error != NULL, NULL);  
  TCsvErrorPrivate *priv = t_csv_error_get_private (error);
  g_return_val_if_fail (priv != NULL, NULL);
  g_return_val_if_fail (error->code != T_CSV_ERROR_SYNTAX_ERROR, NULL);
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

    state = tbl[state][input]; /* Now, this is the state of current character */

    if (state == 6) { /* NL */
      *n_row += 1;
      ++line;
      column = 1;
    } else if (0 < state && state < 7)
      ++column;
    if (state == 6 || state == 7) { /* New line or EOF*/
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

static char ***
csv_real_read (const char *contents, size_t length, int n_row, int n_column, GError **err) {
  char *pt;
  char c;
  int state = 0, input;

#define MAXBUF 4096
  char buf[MAXBUF];
  int bi; /* buf index */
  char ***array;
  int i, j;
  int line, column; /* the location of the current character */

  array = array_alloc (n_row, n_column);
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

    if ((state == 1 || state == 4) && (input == 0 || input == 1 || input == 4)) {
      if (i >= n_row) {
        g_error ("%s\n", csv_prog_error_message (record_overflow));
        array_free (array, n_row, n_column);
        return NULL;
      }
      buf[bi] = '\0';
      array_put (array, i, j, g_strdup (buf));
      bi = 0;
      if (++j >= n_column) {
        j = 0;
        ++i;
      }
    }

    state = tbl[state][input]; /* Now, this is the state of current character */
    if (state == 6) { /* NL */
      ++line;
      column = 1;
    } else if (0 < state && state < 7)
      ++column;
    if (state == 1 || state == 3) { /* in the fieldL */
      if (MAXBUF - bi > 2) /* buf has enough space to insert a char. */
        buf[bi++] = c;
      else {
        g_error ("%s\n", csv_prog_error_message (temp_buf_overflow));
        array_free (array, n_row, n_column);
        return NULL;
      }
    }
    if (state < 0) {
      if (err != NULL)
        t_csv_error_set_syntax_error (err, line, column, NULL);
      array_free (array, n_row, n_column);
      return NULL;
    }
    if (state > 7) {
      g_error ("%s\n", csv_prog_error_message (state_error));
      array_free (array, n_row, n_column);
      return NULL;
    }
    if (state == 7) /* EOF */
      break;
  }
  return array;
}

static char *
csv_real_write (char ***array, int n_row, int n_column, size_t *length) {
  char c;
  int i, j, n_nl, n_comma, n_quote;
  char *s, *t, *csv, *pt;

  *length = 0;
  for (i=0; i<n_row; ++i)
    for (j=0; j<n_column; ++j) {
      s = array_get (array, i, j);
      if (s == NULL)
        ; /* do nothing */
      else {
        for (t=s, n_nl=n_comma=n_quote=0; *t; ++t) {
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
      }
    }
  ++(*length); /* EOS */
  csv = g_new0 (char, *length);
  for (i=0, pt=csv; i<n_row; ++i) {
    for (j=0; j<n_column; ++j) {
      s = array_get (array, i, j);
      if (s == NULL)
        ; /* do nothing */
      else {
        for (t=s, n_nl=n_comma=n_quote=0; *t; ++t) {
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
            for (t=s; (*pt = *t); ++pt, ++t)
              ;
            *pt++ = ',';
          }
        } else
          if (csv + *length <= pt + strlen (s) + n_quote + 2 + 1)
            g_error ("%s\n", csv_prog_error_message (csv_buf_overflow));
          else {
            *pt++ = '"';
            for (t=s; *t; ++pt, ++t) {
              if (*t == '"') {
                *pt++ = '"';
                *pt = '"';
              } else
                *pt = *t;
            }
            *pt++ = '"';
            *pt++ = ',';
          }
      }
    }
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
csv_validate (char *filename, GError **err) {
  g_return_val_if_fail (err == NULL || (err != NULL && *err == NULL), FALSE);  

  GFile *file;
  char *contents;
  size_t length;
  int state;
  int n_row, n_column;

  file = g_file_new_for_path (filename);
  if (! g_file_load_contents (file, NULL, &contents, &length, NULL, err)) { /* read error */
    g_object_unref (file);
    return FALSE;
  }
  g_object_unref (file);
  state = csv_real_validate (contents, length, &n_row, &n_column, err);
  g_free (contents);
  return state;
}

char ***
csv_read (char *filename, int *n_row, int *n_column, GError **err) {
  g_return_val_if_fail (err == NULL || (err != NULL && *err == NULL), NULL);  

  GFile *file;
  char *contents;
  size_t length;
  gboolean state;
  char ***array;

  file = g_file_new_for_path (filename);
  if (! g_file_load_contents (file, NULL, &contents, &length, NULL, err)) { /* read error */
    g_object_unref (file);
    return NULL;
  }
  g_object_unref (file);
  state = csv_real_validate (contents, length, n_row, n_column, err);
  if (! state) {
    free (contents);
    return NULL;
  }
  array = csv_real_read (contents, length, *n_row, *n_column, err);
  free (contents);
  return array;
}

gboolean
csv_write (char *filename, char ***array, int n_row, int n_column, GError **err) {
  g_return_val_if_fail (err == NULL || (err != NULL && *err == NULL), FALSE);  

  GFile *file;
  char *contents;
  size_t length;
  gboolean state;

  contents = csv_real_write (array, n_row, n_column, &length);
  if (contents == NULL)
    return FALSE;
  file = g_file_new_for_path (filename);
  state = g_file_replace_contents (file, contents, length, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, err);
  g_object_unref (file);
  g_free (contents);
  return state;
}

