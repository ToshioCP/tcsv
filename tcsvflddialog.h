#ifndef __T_CSV_FLD_DIALOG_H__
#define __T_CSV_FLD_DIALOG_H__

#include <gtk/gtk.h>
#include "tcsvwindow.h"

#define T_TYPE_CSV_FLD_DIALOG t_csv_fld_dialog_get_type ()
G_DECLARE_FINAL_TYPE (TCsvFldDialog, t_csv_fld_dialog, T, CSV_FLD_DIALOG, GtkDialog)

GListStore *
t_csv_fld_dialog_get_record (TCsvFldDialog *fld_dialog);

GtkWindow *
t_csv_fld_dialog_get_window (TCsvFldDialog *fld_dialog);

GtkWidget *
t_csv_fld_dialog_new (GtkWindow *win);

#endif /* __T_CSV_FLD_DIALOG_H__ */

