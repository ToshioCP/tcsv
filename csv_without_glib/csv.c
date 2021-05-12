#include "csv.h" 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0

char ***
array_alloc (int n_i, int n_j) {
  void **array;
  char i;

  array = calloc ((size_t) n_i + n_i*n_j, sizeof (void *));
  if (array == NULL)
    return NULL;
  for (i = 0; i < n_i; ++i)
    *(array + i) = array + n_i + i*n_j;
  return (char ***) array;
}

void
array_free (char ***array, int n_i, int n_j) {
  int i, j;

  for (i=0; i<n_i; ++i) {
    for (j=0; j<n_j; ++j)
      if (*(*(array+i)+j) != NULL)
        free (*(*(array+i)+j));
  }
  free (array);
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

int
str_dup (char **t, char *s) {
  if (s == NULL) {
    *t = NULL;
    return TRUE;
  }
  *t = (char *) malloc (strlen (s) + 1);
  if (*t == NULL)
    return FALSE;
  strcpy (*t, s);
  return TRUE;
}

static char *error_message[] = {
  "Normal",
  "Csv is illegal (different number of fields)",
  "Csv is illegal (Syntax error)",
  "System error",
  "", /* end of runtime error */
  "Program error: state is out of the range (1-7)",
  "Program error: record overflow",
  "Program error: temporary buffer overflow",
  "Program error: csv buffer overflow"
};

char *
csv_runtime_error_message (struct CsvError *err) {
  int csv_err = err->csv_err;

  if (csv_err > 0 && csv_err < end_of_runtime_error)
    return error_message[csv_err];
  else
    return NULL;
}

static char *
csv_program_error_message (int csv_err) {
  if (csv_err > end_of_runtime_error && csv_err < end_of_program_error)
    return error_message[csv_err];
  else
    return NULL;
}

static void
print_prog_error (int csv_err) {
  char *s;

  if ((s = csv_program_error_message (csv_err)) != NULL)
    fprintf (stderr, "%s\n", s);
}

static off_t
file_size (char *filename, struct CsvError *err) {
  struct stat stbuf;

  if (stat (filename, &stbuf) == -1) {
    if (err != NULL) {
      err->csv_err = system_error;    
      err->sys_err = errno;
    }
    return (off_t) 0;
  } else
    return stbuf.st_size;
}

char *
file_read (char *filename, size_t *size, struct CsvError *err) {
  FILE *stream;
  char *buf;
  size_t n_read;
  int f_error;

  if ((stream = fopen (filename, "r")) == NULL) {
    if (err != NULL) {
      err->csv_err = system_error;
      err->sys_err = errno;
    }
    return NULL;
  }
  n_read = (size_t) file_size (filename, err);
  if (n_read == 0) {
    fclose (stream);
    return NULL;
  }
  buf = (char *) malloc (n_read);
  if (buf == NULL) {
    if (err != NULL) {
      err->csv_err = system_error;
      err->sys_err = errno;
    }
    fclose (stream);
    return NULL;
  }
  *size = fread (buf, sizeof (char), n_read, stream);
  f_error = errno;
  if (ferror (stream) != 0) {
    if (err != NULL) {
      err->csv_err = system_error;
      err->sys_err = f_error;
    }
    fclose (stream);
    free (buf);
    return NULL;
  }
  return buf;
}

size_t
file_write (char *filename, char *buf, size_t size, struct CsvError *err) {
  FILE *stream;
  size_t n_write;

  if ((stream = fopen (filename, "w")) == NULL) {
    if (err != NULL) {
      err->csv_err = system_error;
      err->sys_err = errno;
    }
    return 0;
  }
  n_write = fwrite (buf, sizeof (char), size, stream);
  if (n_write < size && err != NULL) {
    err->csv_err = system_error;
    err->sys_err = errno;
  }
  fclose (stream);
  return n_write;
}

static int tbl[7][5] = {
  {6, 5, 2, 1, 7},
  {6, 5, 1, 1, 7},
  {3, 3, 4, 3, -1},
  {3, 3, 4, 3, -1},
  {6, 5, 3, -1, 7},
  {6, 5, 2, 1, 7},
  {6, 5, 2, 1, 7}
};

static int
csv_real_validate (char *contents, size_t length, int *n_row, int *n_column, struct CsvError *err) {
  char *pt;
  char c;
  int state = 0, input;
  int nfield0 = 0;
  int nfield = 0;

  *n_row = 0; /* row counter */

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
    
    if (state == 6) /* NL */
      *n_row += 1;
    if (state == 6 || state == 7) { /* New line or EOF*/
      ++nfield;
      nfield0 = (nfield0 == 0) ? nfield : nfield0;
      if (nfield0 != nfield) {
        if (err != NULL)
          err->csv_err = csv_diff_n_fields;
        return FALSE;
      }
      nfield = 0;
    }
    if (state == 5) /* Comma */
      ++nfield;
    if (state < 0) {
      if (err != NULL)
        err->csv_err = csv_syntax_error;
      return FALSE;
    }
    if (state > 7) {
      print_prog_error (state_error);
      return FALSE;
    }
    if (state == 7) /* EOF */
      break;
  }
  *n_column = nfield0;
  return TRUE;
}

static char ***
csv_real_read (const char *contents, size_t length, int n_row, int n_column, struct CsvError *err) {
  char *pt, *s;
  char c;
  int state = 0, input;

#define MAXBUF 4096
  char buf[MAXBUF];
  int bi; /* buf index */
  char ***array;
  int i, j;
  int s_error;

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
        print_prog_error (record_overflow);
        array_free (array, n_row, n_column);
        return NULL;
      }
      buf[bi] = '\0';
      if (! str_dup (&s, buf)) {
        s_error = errno;
        print_prog_error (system_error);
        printf ("%s\n", strerror (s_error));
        array_free (array, n_row, n_column);
        return NULL;
      }
      array_put (array, i, j, s);
      bi = 0;
      if (++j >= n_column) {
        j = 0;
        ++i;
      }
    }

    state = tbl[state][input]; /* Now, this is the state of current character */
    if (state == 1 || state == 3) { /* in the fieldL */
      if (MAXBUF - bi > 2) /* buf has enough space to insert a char. */
        buf[bi++] = c;
      else {
        print_prog_error (temp_buf_overflow);
        array_free (array, n_row, n_column);
        return NULL;
      }
    }
    if (state < 0) {
      if (err != NULL)
        err->csv_err = csv_syntax_error;
      array_free (array, n_row, n_column);
      return NULL;
    }
    if (state > 7) {
      print_prog_error (state_error);
      array_free (array, n_row, n_column);
      return NULL;
    }
    if (state == 7) /* EOF */
      break;
  }
  return array;
}

static char *
csv_real_write (char ***array, int n_row, int n_column, size_t *length, struct CsvError *err) {
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
  csv = (char *) calloc (*length, sizeof (char));
  if (csv == NULL) {
    if (err != NULL) {
      err->csv_err = system_error;
      err->sys_err = errno;
    }
    return NULL;
  }
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
            print_prog_error (csv_buf_overflow);
          else {
            for (t=s; (*pt = *t); ++pt, ++t)
              ;
            *pt++ = ',';
          }
        } else
          if (csv + *length <= pt + strlen (s) + n_quote + 2 + 1)
            print_prog_error (csv_buf_overflow);
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
    print_prog_error (csv_buf_overflow);
    free (csv);
    csv = NULL;
  } else
    *pt = '\0';
  return csv;
}

int
csv_validate (char *filename, struct CsvError *err) {
  char *contents;
  size_t length;
  int state;
  int n_row, n_column;

  if ((contents = file_read (filename, &length, err)) == NULL) /* read error */
    return FALSE;
  state = csv_real_validate (contents, length, &n_row, &n_column, err);
  free (contents);
  return state;
}

char ***
csv_read (char *filename, int *n_row, int *n_column, struct CsvError *err) {

  char *contents;
  size_t length;
  int state;
  char ***array;

  if ((contents = file_read (filename, &length, err)) == NULL) /* read error */
    return FALSE;
  state = csv_real_validate (contents, length, n_row, n_column, err);
  if (! state) {
    free (contents);
    return NULL;
  }
  array = csv_real_read (contents, length, *n_row, *n_column, err);
  free (contents);
  return array;
}

void
csv_write (char *filename, char ***array, int n_row, int n_column, struct CsvError *err) {
  char *contents;
  size_t length;

  contents = csv_real_write (array, n_row, n_column, &length, err);
  if (contents == NULL)
    return;
  file_write (filename, contents, length, err);
}

