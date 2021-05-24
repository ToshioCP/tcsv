#include <gtk/gtk.h>
#include "../tstr.c"

void
test_t_str (void) {
  char *s1 = "Hello";
  char *s2 = "Goodby";
  TStr *str1 = t_str_new_with_string (s1);
  TStr *str2 = t_str_new ();
  char *t1, *t2;

  t1 = t_str_get_string (str1);
  if (strcmp (s1, t1) != 0) {
    g_print ("TStr returned different string.\n");
    g_print ("Expected string is %s\n", s1);
    g_print ("Returned string is %s\n", t1);
  }
  g_free (t1);
  t_str_set_string (str2, s2);
  t2 = t_str_get_string (str2);
  if (strcmp (s2, t2) != 0) {
    g_print ("TStr returned different string.\n");
    g_print ("Expected string is %s\n", s2);
    g_print ("Returned string is %s\n", t2);
  }
  g_free (t2);
  /* Does freeing t1 affects str2? */
  g_object_get (str2, "string", &t2, NULL);
  if (strcmp (s2, t2) != 0) {
    g_print ("TStr returned different string.\n");
    g_print ("Expected string is %s\n", s2);
    g_print ("Returned string is %s\n", t2);
  }
  g_free (t2);

  g_object_unref (str1);
  g_object_unref (str2);
}

int
main (int argc, char **argv) {
  g_print ("Test t_str\n");
  test_t_str ();
}
