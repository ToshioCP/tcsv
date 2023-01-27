#include <gtk/gtk.h>
#include "../tmodify.h"

void
test_real_test (TModify *modify, int old_position, int new_position, const char *old_string, const char *new_string) {
  int p1;
  const char *t1;
  char *t2;

  if ((p1 = t_modify_get_old_position (modify)) != old_position) {
    g_print ("T_modify_get_old_position returned different integer.\n");
    g_print ("Expected integer is %ds\n", old_position);
    g_print ("Returned integer is %d\n", p1);
  }
  if ((p1 = t_modify_get_new_position (modify)) != new_position) {
    g_print ("T_modify_get_new_position returned different integer.\n");
    g_print ("Expected integer is %ds\n", new_position);
    g_print ("Returned integer is %d\n", p1);
  }

  if (g_strcmp0 ((t1 = t_modify_look_old_string (modify)), old_string) != 0) {
    g_print ("T_modify_look_old_string returned different string.\n");
    g_print ("Expected string is %s\n", old_string);
    g_print ("Returned string is %s\n", t1);
  }
  if (g_strcmp0 ((t2 = t_modify_get_old_string (modify)), old_string) != 0) {
    g_print ("T_modify_get_old_string returned different string.\n");
    g_print ("Expected string is %s\n", old_string);
    g_print ("Returned string is %s\n", t2);
  }
  if (t1 == t2) {
    g_print ("T_modify_look_old_string and t_modify_get_old_string returned the same pointer.\n");
    g_print ("T_modify_get_old_string possibly doesn't use g_strdup.\n");
  }
  g_free (t2);

  if (g_strcmp0 ((t1 = t_modify_look_new_string (modify)), new_string) != 0) {
    g_print ("T_modify_look_new_string returned different string.\n");
    g_print ("Expected string is %s\n", new_string);
    g_print ("Returned string is %s\n", t1);
  }
  if (g_strcmp0 ((t2 = t_modify_get_new_string (modify)), new_string) != 0) {
    g_print ("T_modify_get_new_string returned different string.\n");
    g_print ("Expected string is %s\n", new_string);
    g_print ("Returned string is %s\n", t2);
  }
  if (t1 == t2) {
    g_print ("T_modify_look_new_string and t_modify_get_new_string returned the same pointer.\n");
    g_print ("T_modify_get_new_string possibly doesn't use g_strdup.\n");
  }
  g_free (t2);
}

void
test_t_modify (void) {
  int p1, p2;
  char *s1;
  char *s2;
  TModify *modify;

  s1 = "Hello";
  s2 = "Bye";
  modify = t_modify_new_with_data (1, 2, s1, s2);
  test_real_test (modify, 1, 2, s1, s2);

  s1 = "foo";
  s2 = "bar";
  t_modify_set_old_position (modify, 3);
  t_modify_set_new_position (modify, 4);
  t_modify_set_old_string (modify, s1);
  t_modify_set_new_string (modify, s2);  
  test_real_test (modify, 3, 4, s1, s2);

  s1 = "first";
  s2 = "second";
  g_object_set (modify, "old-position", 5, "new-position", 6, "old-string", s1, "new-string", s2, NULL);
  test_real_test (modify, 5, 6, s1, s2);
  
  g_object_get (modify, "old-position", &p1, "new-position", &p2, "old-string", &s1, "new-string", &s2, NULL);
  if (p1 != 5) {
    g_print ("g_object_get didn't work (old-position property).\n");
    g_print ("expected is %d\n", 5);
    g_print ("actual is %d\n", p1);
  }
  if (p2 != 6) {
    g_print ("g_object_get didn't work (new-position property).\n");
    g_print ("expected is %d\n", 6);
    g_print ("actual is %d\n", p2);
  }
  if (g_strcmp0 ("first", s1) != 0) {
    g_print ("g_object_get didn't work (old-string property).\n");
    g_print ("expected is %s\n", "first");
    g_print ("actual is %s\n", s1);
  }
  if (g_strcmp0 ("second", s2) != 0) {
    g_print ("g_object_get didn't work (new-string property).\n");
    g_print ("expected is %s\n", "second");
    g_print ("actual is %s\n", s2);
  }
}

int
main (int argc, char **argv) {
  g_print ("Test TModify\n");
  test_t_modify ();
}
