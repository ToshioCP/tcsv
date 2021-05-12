#ifndef __CSV_H__
#define __CSV_H__

#include <gio/gio.h>

char ***
array_alloc (int n_i, int n_j);

void
array_free (char ***array, int n_i, int n_j);

void
array_put (char ***array, int i, int j, char *s);

char *
array_get (char ***array, int i, int j);

typedef enum {
  T_CSV_ERROR_DIFF_NFIELDS,
  T_CSV_ERROR_SYNTAX_ERROR
} TCsvError;
#define T_CSV_ERROR (t_csv_error_quark ())
GQuark t_csv_error_quark (void);

gboolean
csv_validate (char *filename, GError **err);

char ***
csv_read (char *filename, int *n_row, int *n_column, GError **err);

gboolean
csv_write (char *filename, char ***array, int n_row, int n_column, GError **err);

#endif /* __CSV_H__ */

