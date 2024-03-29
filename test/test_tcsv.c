#include <gtk/gtk.h>
#include "../tcsv.c"

static char *sample1 = 
"a,\"bc\",,\"ghi\"\n"
"0,\"1, 2\",3,\"4, 5\nand 6\"\n"
"\"one\",\"two\",\"three\",\"and \"\"four\"\".\"";

static char *sample1_array[3][4] = {
  {"a", "bc", "", "ghi" },
  {"0", "1, 2", "3", "4, 5\nand 6"},
  {"one", "two", "three", "and \"four\"."}
};

static char *sample2 = 
"a,bc,def,あいう\n"
"0,1,3,6\n"
"one,二,three,and \"four\".\n"
"10,20,30\n";

static char *sample3 = 
"10,20,30\n"
"1,\"2,\"3\",4\",5";

static void
test_csv_real_validate (void) {
  char *contents;
  size_t length;
  int n_row, n_column;
  GError *err;;

  contents = sample1;
  length = (size_t) (strlen (contents) + 1);
  err = NULL;
  if (! csv_real_validate (contents, length, &n_row, &n_column, &err)) {
    g_print ("Sample1 is correct, but csv_real_validate returns invalid.\n");
    g_print ("The error message is:\n");
    g_print ("    %s\n", err->message);
  }
  contents = sample2;
  length = (size_t) (strlen (contents) + 1);
  err = NULL;
  if (csv_real_validate (contents, length, &n_row, &n_column, &err))
    g_print ("Sample2 is incorrect, but csv_real_validate returns valid.\n");
  else if (err->code != T_CSV_ERROR_DIFF_NFIELDS) {
    g_print ("The error of sample2 is 'different number of fields', but csv_validate returns the other error.\n");
    g_print ("The error message is:\n");
    g_print ("    %s\n", err->message);
  }
  contents = sample3;
  length = (size_t) (strlen (contents) + 1);
  err = NULL;
  if (csv_real_validate (contents, length, &n_row, &n_column, &err))
    g_print ("Sample3 is incorrect, but csv_real_validate returns valid.\n");
  else if (err->code != T_CSV_ERROR_SYNTAX_ERROR) {
    g_print ("The error of sample3 is 'syntax error', but csv_validate returns the other error.\n");
    g_print ("The error message is:\n");
    g_print ("    %s\n", err->message);
  }
}

static void
test_csv_real_read (void) {
  GListStore *liststore;
  GListStore *record;
  char *contents;
  size_t length;
  int n_row, n_column;
  GError *err;
  int i, j;
  const char *s;

  liststore = g_list_store_new (G_TYPE_LIST_STORE);
  contents = sample1;
  length = (size_t) (strlen (contents) + 1);
  n_row = 3;
  n_column = 4;
  err = NULL;
  if (! csv_real_read (liststore, contents, length, n_row, n_column, &err)) {
    g_print ("Sample1 is correct, but csv_real_validate returns invalid.\n");
    g_print ("The error message is:\n");
    g_print ("    %s\n", err->message);
  } else {
    for (i=0; i<n_row; ++i) {
      if (! sl_is_tcsv_stringlist (record = G_LIST_STORE (g_list_model_get_item (G_LIST_MODEL (liststore), i)))) {
        g_print ("liststore[%d] isn't tcsv_stringlist.\n", i);
        continue;
      }
      for (j=0; j<n_column; ++j) {
        s = sl_look_string (record, j);
        if (s == NULL)
          g_print ("(%d, %d) string is NULL.\n", i, j);
        else if (strcmp (s, sample1_array[i][j]) != 0) {
          g_print ("(%d, %d) string is different. '%s' is expected, but '%s' is returned.\n", i, j, sample1_array[i][j], s);
        }
      }
      g_object_unref (record);
    }
  }
  g_object_unref (liststore);
}

static void
test_csv_real_write (void) {
  GListStore *liststore;
  GListStore *record;
  char *contents;
  size_t length;
  int n_row, n_column;
  int i, j;
  GError *err = NULL;

  char *a[3][4]= {
    {"a", "bc", "def", "gh\nij"},
    {"1", "23", "456", "78\"90"},
    {"\"one\"", ", \"two\"", ", \"three\"", "and \"four\"\n"}
  };
  char *csv ="a,bc,def,\"gh\nij\"\n1,23,456,\"78\"\"90\"\n\"\"\"one\"\"\",\", \"\"two\"\"\",\", \"\"three\"\"\",\"and \"\"four\"\"\n\"\n";


  liststore = g_list_store_new (G_TYPE_LIST_STORE);
  n_row = 3;
  n_column = 4;
  for (i=0; i<n_row; ++i) {
    record = g_list_store_new (T_TYPE_CSV_STR);
    g_list_store_append (liststore, record);
    for (j=0; j<n_column; ++j)
      sl_append_string (record, a[i][j]);
  }
  if ((contents = csv_real_write (liststore, n_row, n_column, &length, &err)) == NULL)
    g_print ("%s\n", err->message);
  else {
    if (strcmp (csv, contents) != 0) {
      g_print ("Csv write error. (The contents of the csv file is different from what is expected.)\n");
      g_print ("Expected contents are '%s'.\n", csv);
      g_print ("The returned contents are '%s'.\n", contents);
    }
    g_free (contents);
  }
  g_object_unref (liststore);

  char *b[3][4]= {
    {"a", "bc", "def", "gh\nij"},
    {"1", "23", "456", NULL},
    {"\"one\"", ", \"two\"", ", \"three\"", "and \"four\"\n"}
  };

  liststore = g_list_store_new (G_TYPE_LIST_STORE);
  n_row = 3;
  n_column = 4;
  for (i=0; i<n_row; ++i) {
    record = g_list_store_new (T_TYPE_CSV_STR);
    g_list_store_append (liststore, record);
    for (j=0; j<n_column; ++j)
      if (b[i][j])
        sl_append_string (record, b[i][j]);
  }
  if ((contents = csv_real_write (liststore, n_row, n_column, &length, &err)) == NULL) {
    if (err->code != T_CSV_ERROR_NO_STRING) {
      g_print ("Unexpected error code (!= T_CSV_ERROR_NO_STRING\n");
      g_print ("Returned error was:  %s\n", err->message);
    }
  } else {
    g_print ("Original string array has different number of fields, but csv_real_write returns no error.\n");
    g_free (contents);
  }
  g_object_unref (liststore);
}

void
test_csv_validate (void) {
  GFile *file;
  GError *err;
  gboolean state;

  file = g_file_new_for_path ("sample1.csv");
  err = NULL;
  if (! csv_validate (file, &err)) {
    g_print ("Sample1.csv is correct, but csv_validate returns error.");
    g_print ("  Error message was %s\n", err->message);
  }
  g_object_unref (file);

  file = g_file_new_for_path ("sample2.csv");
  err = NULL;
  if (! csv_validate (file, &err)) {
    g_print ("Sample2.csv is correct, but csv_validate returns error.");
    g_print ("  Error message was %s\n", err->message);
  }
  g_object_unref (file);

  file = g_file_new_for_path ("sample3.csv");
  err = NULL;
  if ((state = csv_validate (file, &err)) == TRUE)
    g_print ("Sample3.csv is incorrect, but csv_validate returns error.\n");
  else if (err->code != T_CSV_ERROR_SYNTAX_ERROR) {
    g_print ("Sample3.csv has syntax error, but csv_validate returns different error.\n");
    g_print ("  Error message was %s\n", err->message);
  }
  g_object_unref (file);
}

static void
test_csv_read (void) {
  GListStore *liststore;
  GListStore *record;
  GFile *file;
  int n_row, n_column;
  GError *err = NULL;
  int i, j;
  char *a[3][4] = {
    {"a", "bc", "def", "あいう"},
    {"0", "", "3", "6"},
    {"one", "二", "three", "and \"four\"."}
  };
  const char *s;

  liststore = g_list_store_new (G_TYPE_LIST_STORE);
  file = g_file_new_for_path ("sample1.csv");
  if (! csv_read (liststore, file, &n_row, &n_column, &err)) {
    g_print ("'sample1.csv' is a correct csv file, but csv_read returns FALSE.\n");
    g_print ("%s\n", err->message);
    g_clear_error (&err);
  } else if (n_row != 3 || n_column != 4) {
    g_print ("'sample1.csv' is a correct csv file, and csv_read returns TRUE.\n");
    g_print ("But it returns wrong (row, column).\n");
    g_print ("Correct size = (3, 4), but csv_read returned (%d, %d).\n", n_row, n_column);
  } else {
    for (i=0; i<n_row; ++i) {
      record = G_LIST_STORE (g_list_model_get_item (G_LIST_MODEL (liststore), i));
      for (j=0; j<n_column; ++j) {
        s = sl_look_string (record, j);
        if (s == NULL)
          g_print ("(%d, %d) string is NULL.\n", i, j);
        else if (strcmp (s, a[i][j]) != 0)
          g_print ("(%d, %d) string is different. '%s' is expected, but '%s' is returned.\n", i, j, a[i][j], s);
      }
      g_object_unref (record);
    }
  }
  g_object_unref (file);
  g_object_unref (liststore);

  liststore = g_list_store_new (G_TYPE_LIST_STORE);
  file = g_file_new_for_path ("sample3.csv");
  if (csv_read (liststore, file, &n_row, &n_column, &err)) {
    g_print ("'sample3.csv' is an incorrect csv file, but csv_read returns TRUE.\n");
  } else if (err->code != T_CSV_ERROR_SYNTAX_ERROR) {
    g_print ("'sample3.csv' is an incorrect csv file, but csv_read returns FALSE.\n");
    g_print ("The error must be 'syntax error', but it returns different error.\n");
    g_print ("%s\n", err->message);
    g_clear_error (&err);
  }
  g_object_unref (file);
  g_object_unref (liststore);
}

static int
t_strcmp_len (char *s, char *t, int n) {
/* s and t can end without '\0', but their length must be greater than n */
  int i;

  if (n == 0)
    return 0;
  else if (n < 0)
    return -2;
  for (i=0; i<n; ++i) {
    if (s[i] > t[i])
      return 1;
    else if (s[i] <t[i])
      return -1;
  }
  return 0;
}

static void
test_t_strcmp_len (void) {
  char s[] = "abcde";
  char t[] = "abcfg";
  int result[] = {-2, 0, 0, 0, 0, -1, -1};
  int i;

  for (i=-1; i<6; ++i)
    if (t_strcmp_len(s, t, i) != result[i+1])
      g_print ("%d is expected, but t_strcmp_len returned %d.\n", result[i+1], t_strcmp_len(s, t, i));
  if (t_strcmp_len(t, s, 4) != 1)
    g_print ("1 is expected, but t_strcmp_len returned %d.\n", t_strcmp_len(t, s, i));
}

static void
test_csv_write (void) {
  GListStore *liststore;
  GListStore *record;
  GFile *file;
  int n_row, n_column;
  GError *err = NULL;
  int i, j;
  char *a[3][4]= {
    {"a", "bc", "def", "gh\nij"},
    {"1", "23", "456", "78\"90"},
    {"\"one\"", ", \"two\"", ", \"three\"", "and \"four\"\n"}
  };
  char *csv ="a,bc,def,\"gh\nij\"\n1,23,456,\"78\"\"90\"\n\"\"\"one\"\"\",\", \"\"two\"\"\",\", \"\"three\"\"\",\"and \"\"four\"\"\n\"\n";
  int csv_len = strlen (csv);
  char *contents;
  size_t length;

  liststore = g_list_store_new (G_TYPE_LIST_STORE);
  n_row = 3;
  n_column = 4;
  for (i=0; i<n_row; ++i) {
    record = g_list_store_new (T_TYPE_CSV_STR);
    g_list_store_append (liststore, record);
    for (j=0; j<n_column; ++j)
      sl_append_string (record, a[i][j]);
    g_object_unref (record);
  }
  file = g_file_new_for_path ("sample4.csv");
  if (! csv_write (liststore, file, n_row, n_column, &err)) {
    g_print ("csv_write returns FALSE.\n");
    g_print ("%s\n", err->message);
    g_clear_error (&err);
  } else {
    if (! g_file_load_contents (file, NULL, &contents, &length, NULL, &err)) {
      g_print ("g_file_load_contents returns FALSE.\n");
      g_print ("%s\n", err->message);
      g_clear_error (&err);
    } else if (length != csv_len) {
      g_print ("Size of contents isn't strlen(csv).\n");
      g_print ("csv_write might write EOS ('\\0') to the file.\n");
      g_print ("Size of contents is %ld\n", length);
      g_print ("strlen(csv) = %d\n", csv_len);
    } else if (t_strcmp_len (csv, contents, length) != 0) {
      g_print ("Csv write error. (The contents of the csv file is different from what is expected.)\n");
      g_print ("Expected contents are '%s'.\n", csv);
      g_print ("The returned contents are '%s'.\n", contents);
    }
    g_free (contents);
    err = NULL;
    if (! g_file_delete (file, NULL, &err)) {
      g_print ("%s\n", err->message);
      g_clear_error (&err);
    }
  }
  g_object_unref (file);
  g_object_unref (liststore);
}

int
main (int argc, char **argv) {
  g_print ("Self test -- t_strcmp_len\n");
  test_t_strcmp_len ();
  g_print ("Test csv_real_validate.\n");
  test_csv_real_validate ();
  g_print ("Test csv_real_read.\n");
  test_csv_real_read ();
  g_print ("Test csv_real_write.\n");
  test_csv_real_write ();
  g_print ("Test csv_validate.\n");
  test_csv_validate ();
  g_print ("Test csv_read.\n");
  test_csv_read ();
  g_print ("Test csv_write.\n");
  test_csv_write ();
}
