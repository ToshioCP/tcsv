#include "../csv.h"
#include <gio/gio.h>

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
    g_print ("test_array: Array allocation error. %s\n", strerror (errno));
    return;
  }
  g_print ("array = %p\n", array);
  for (i=0; i<n_row; ++i) {
    g_print ("i = %d, array+i = %p\n", i, array+i);
    for (j=0; j<n_column; ++j) {
      g_print ("  j = %d, *(array+i)+j = %p\n", j, *(array+i)+j);
      s = g_strdup (a[i][j]);
      array_put (array, i, j, s);
    }
  }
  for (i=0; i<n_row; ++i)
    for (j=0; j<n_column; ++j)
      g_print ("(%d, %d) => %s\n", i, j, array_get (array, i, j));

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
val_read (char *filename, const char *contents) {
  char ***array = NULL;
  int n_row, n_column;
  int i, j;
  GFile *file;
  GError *err = NULL;
  size_t length;

  file = g_file_new_for_path (filename);
  length = (size_t) (strlen (contents) + 1);
  if (! g_file_replace_contents (file, contents, length, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, &err)) {
    g_print ("%s\n", err->message);
    g_clear_error (&err);
    g_object_unref (file);
    return;
  }
  if (! csv_validate (filename, &err)) {
    g_print ("%s\n", err->message);
    g_print ("No test will be done for csv_read.\n");
    g_clear_error (&err);
  } else if ((array = csv_read (filename, &n_row, &n_column, &err)) == NULL) {
    g_print ("%s\n", err->message);
    g_clear_error (&err);
  } else {
    g_print ("csv_validate returns TRUE, and csv_read succeeds.\n");
    for (i=0; i<n_row; ++i) {
      g_print ("\n");
      for (j=0; j<n_column; ++j)
        g_print ("%s\n", array_get (array, i, j));
    }
    array_free (array, n_row, n_column);
  }
  if (! g_file_delete (file, NULL, &err)) {
    g_print ("%s\n", err->message);
    g_clear_error (&err);
  }
  g_object_unref (file);
}

static void
test_val_read (void) {
  g_print ("-- sample1 --\n");
  val_read ("sample1.csv", sample1);
  g_print ("-- sample2 --\n");
  val_read ("sample2.csv", sample2);
  g_print ("-- sample3 --\n");
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
  char *filename;
  GFile *file;
  GError *err = NULL;

  array = array_alloc (3, 4);
  for (i=0; i<3; ++i)
    for (j=0; j<4; ++j)
      array_put (array, i, j, g_strdup (a[i][j]));
  filename = "sample4.csv";
  if (! csv_write (filename, array, 3, 4, &err)) {
    g_print ("%s\n", err->message);
    g_clear_error (&err);
    return;
  }
  file = g_file_new_for_path (filename);
  if (! g_file_load_contents (file, NULL, &contents, &length, NULL, &err)) {
    g_print ("%s\n", err->message);
    g_clear_error (&err);
  } else if (strcmp (csv, contents) != 0) {
    g_print ("Csv write error. (The contents of the csv file is different from what is expected.)\n");
    g_print ("Expected contents are \'%s\'.\n", csv);
    g_print ("The returned contents are \'%s\'.\n", contents);
    g_free (contents);
  } else {
    g_print ("Csv is successfully written.\n");
    g_free (contents);
  }
  if (! g_file_delete (file, NULL, &err)) {
    g_print ("%s\n", err->message);
    g_clear_error (&err);
  }
  g_object_unref (file);
  array_free (array, 3, 4);
}

int
main (int argc, char **argv) {
  test_array ();
  g_print ("\n");
  test_val_read ();
  g_print ("\n");
  test_val_write ();
}
