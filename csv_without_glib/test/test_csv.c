#include "../csv.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static void
test_str_dup (void) {
  printf ("Test str_dup\n");
  char *s;
  int bool;
  bool = str_dup (&s, "Hello.");
  if (bool && strcmp (s, "Hello.") != 0) {
    printf ("str_dup didn't work.\n");
    printf ("Original string is 'Hello.'\n");
    printf ("New string is '%s'\n", s);
  } else if (! bool)
    printf ("str_dup: %s\n", strerror (errno));
  else {
    printf ("str_dup suceeds.\n");
    printf ("Original string is 'Hello.'\n");
    printf ("New string is '%s'\n", s);
  }
}

static void
test_array (void) {
  int n_row = 3, n_column = 4;
  char ***array;
  int i, j;
  char *a[3][4]= {
    {"a", "bc", "def", "ghij"},
    {"1", "23", "456", "7890"},
    {"one", "two", "three", "four"}
  };
  char *s;

  if ((array = array_alloc (n_row, n_column)) == NULL) {
    printf ("test_array: Array allocation error. %s\n", strerror (errno));
    return;
  }
  printf ("array = %p\n", array);
  for (i=0; i<n_row; ++i) {
    printf ("i = %d, array+i = %p\n", i, array+i);
    for (j=0; j<n_column; ++j) {
      printf ("  j = %d, *(array+i)+j = %p\n", j, *(array+i)+j);
      if (! str_dup (&s, a[i][j])) {
        printf ("test_array: %s\n", strerror (errno));
        array_free (array, n_row, n_column);
        return;
      }
      array_put (array, i, j, s);
    }
  }
  for (i=0; i<n_row; ++i)
    for (j=0; j<n_column; ++j)
      printf ("(%d, %d) => %s\n", i, j, array_get (array, i, j));

  array_free (array, n_row, n_column);
}

static char *sample1 = 
"a,\"bc\",def,\"ghi\"\n"
"0,\"1, 2\",3,\"4, 5\nand 6\"\n"
"\"one\",\"two\",\"three\",\"and \"\"four\"\".\"";

static char *sample2 = 
"a,bc,def,あいう\n"
"0,1,3,6\n"
"one,二,three,and \"four\".\n"
"10,20,30\n";

static char *sample3 = 
"10,20,30\n"
"1,\"2,\"3\",4\",5";

static void
test_file_read_write (void) {
  size_t length, size;
  char *filename = "sample1.csv";
  struct CsvError err;
  char *s;

  size = (size_t) (strlen (sample1) + 1);
  length = file_write (filename, sample1, size, &err);

  if (length != size) {
    printf ("test_file_read_write: file write error.\n");
    printf ("%s\n", csv_runtime_error_message (&err));
    if (err.csv_err == system_error)
      printf ("%s\n", strerror (err.sys_err));
    return;
  }
  s = file_read (filename, &length, &err);
  if (s == NULL || length != size) {
    printf ("test_file_read_write: file read error.\n");
    printf ("%s\n", csv_runtime_error_message (&err));
    if (err.csv_err == system_error)
      printf ("%s\n", strerror (err.sys_err));
    system ("rm sample1.csv");
    return;
  }
  if (strcmp (sample1, s)) {
    printf ("test_file_read_write: string not equal.\n");
    printf ("original string = '%s'\n", sample1);
    printf ("returned string = '%s'\n", s);
  } else
    printf ("test_file_read_write: test succeeds.\n");
  system ("rm sample1.csv");
  free (s);
}

static void
val_read (char *filename, char *contents) {
  char ***array;
  int n_row, n_column;
  int i, j;
  struct CsvError err;
  size_t size, length;
  char *s;

  size = (size_t) (strlen (contents) + 1);
  length = file_write (filename, contents, size, &err);
  if (length != size) {
    printf ("test_file_read_write: file write error.\n");
    printf ("%s\n", csv_runtime_error_message (&err));
    if (err.csv_err == system_error)
      printf ("%s\n", strerror (err.sys_err));
    return;
  }
  if (! csv_validate (filename, &err)) {
    printf ("csv_validate returns FALSE.\n");
    printf ("%s\n", csv_runtime_error_message (&err));
    if (err.csv_err == system_error)
      printf ("%s\n", strerror (err.sys_err));
    printf ("No test will be done for csv_read.\n");
  } else if ((array = csv_read (filename, &n_row, &n_column, &err)) == NULL) {
    printf ("Csv_read returns NULL.\n");
    printf ("%s\n", csv_runtime_error_message (&err));
    if (err.csv_err == system_error)
      printf ("%s\n", strerror (err.sys_err));
  } else {
    printf ("csv_validate returns TRUE, and csv_read succeeds.\n");
    for (i=0; i<n_row; ++i) {
      printf ("\n");
      for (j=0; j<n_column; ++j)
        printf ("%s\n", array_get (array, i, j));
    }
    array_free (array, n_row, n_column);
  }
  s = (char *) malloc (strlen (filename) + 3 + 1);
  sprintf (s, "rm %s", filename);
  system (s);
  free (s);
}

static void
test_val_read (void) {
  printf ("-- sample1 --\n");
  val_read ("sample1.csv", sample1);
  printf ("-- sample2 --\n");
  val_read ("sample2.csv", sample2);
  printf ("-- sample3 --\n");
  val_read ("sample3.csv", sample3);
}

static void
test_val_write (void) {
  char *a[3][4]= {
    {"a", "bc", "def", "gh\nij"},
    {"1", "23", "456", "78\"90"},
    {"\"one\"", ", \"two\"", ", \"three\"", "and \"four\"\n"}
  };
  char *csv ="a,bc,def,\"gh\nij\"\n1,23,456,\"78\"\"90\"\n\"\"\"one\"\"\",\", \"\"two\"\"\",\", \"\"three\"\"\",\"and \"\"four\"\"\n\"\n";
  char *contents;
  size_t length;
  char ***array;
  int i, j;
  struct CsvError err;
  char *s;
  int s_error;

  array = array_alloc (3, 4);
  for (i=0; i<3; ++i)
    for (j=0; j<4; ++j) {
      if (! str_dup (&s, a[i][j])) {
        printf ("test_val_write: %s\n", strerror (errno));
        array_free (array, 3, 4);
        return;
      }        
      array_put (array, i, j, s);
    }
  csv_write ("sample4.csv", array, 3, 4, &err);
  if ((contents = file_read ("sample4.csv", &length, &err)) == NULL) { /* read error */
    s_error = errno;
    printf ("%s\n", csv_runtime_error_message (&err));
    printf ("%s\n", strerror (s_error));
  } else if (strcmp (csv, contents) != 0) {
    printf ("Csv write error. (The contents of the csv file is different from what is expected.)\n");
    printf ("The contents are \'%s\'.\n", contents);
    free (contents);
    system ("rm sample4.csv");
  } else {
    printf ("Csv is successfully written.\n");
    free (contents);
    system ("rm sample4.csv");
  }
  array_free (array, 3, 4);
}

int
main (int argc, char **argv) {
  test_str_dup ();
  printf ("\n");
  test_array ();
  printf ("\n");
  test_file_read_write ();
  printf ("\n");
  test_val_read ();
  printf ("\n");
  test_val_write ();
}
