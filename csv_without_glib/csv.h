#ifndef __CSV_H__
#define __CSV_H__

#include <stdio.h>

char ***
array_alloc (int n_i, int n_j);

void
array_free (char ***array, int n_i, int n_j);

void
array_put (char ***array, int i, int j, char *s);

char *
array_get (char ***array, int i, int j);

int
str_dup (char **t, char *s);

/* Runtime error */
enum {
  normal,
  csv_diff_n_fields,
  csv_syntax_error,
  system_error,
  end_of_runtime_error,
  state_error,
  record_overflow,
  temp_buf_overflow,
  csv_buf_overflow,
  end_of_program_error
};

struct CsvError {
  int csv_err;
  int sys_err;
};

char *
csv_runtime_error_message (struct CsvError *err);

char *
file_read (char *filename, size_t *size, struct CsvError *err);

size_t
file_write (char *filename, char *buf, size_t size, struct CsvError *err);

int
csv_validate (char *filename, struct CsvError *err);

char ***
csv_read (char *filename, int *n_row, int *n_column, struct CsvError *err);

void
csv_write (char *filename, char ***array, int n_row, int n_column, struct CsvError *err);

#endif /* __CSV_H__ */

