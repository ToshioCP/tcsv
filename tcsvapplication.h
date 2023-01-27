#pragma once

#include <gtk/gtk.h>

#define T_TYPE_CSV_APPLICATION t_csv_application_get_type ()
G_DECLARE_FINAL_TYPE (TCsvApplication, t_csv_application, T, CSV_APPLICATION, GtkApplication)

TCsvApplication *
t_csv_application_new (const char* application_id, GApplicationFlags flags);
