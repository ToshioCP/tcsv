#ifndef __T_CSV_H__
#define __T_CSV_H__

#include <gtk/gtk.h>

typedef enum {
  T_CSV_ERROR_DIFF_NFIELDS,
  T_CSV_ERROR_SYNTAX_ERROR,
  T_CSV_ERROR_NO_STRING
} TCsvError;
#define T_CSV_ERROR (t_csv_error_quark ())
GQuark t_csv_error_quark (void);

const char *
t_csv_error_get_detail (GError *error);

gboolean
csv_validate (GFile *file, GError **err);

gboolean
csv_read (GListStore *liststore, GFile *file, int *n_row, int *n_column, GError **err);

gboolean
csv_write (GListStore *liststore, GFile *file, int n_row, int n_column, GError **err);

#endif /* __T_CSV_H__ */

