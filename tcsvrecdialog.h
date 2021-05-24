#ifndef __T_CSV_REC_DIALOG_H__
#define __T_CSV_REC_DIALOG_H__

#include <gtk/gtk.h>
#include "tcsvwindow.h"

#define T_TYPE_CSV_REC_DIALOG t_csv_rec_dialog_get_type ()
G_DECLARE_FINAL_TYPE (TCsvRecDialog, t_csv_rec_dialog, T, CSV_REC_DIALOG, GtkDialog)

GListStore *
t_csv_rec_dialog_get_record (TCsvRecDialog *rec_dialog);

GtkWindow *
t_csv_rec_dialog_get_window (TCsvRecDialog *rec_dialog);

int
t_csv_rec_dialog_get_position (TCsvRecDialog *rec_dialog);

GtkWidget *
t_csv_rec_dialog_new (GtkWindow *win, GListStore *header, GListStore *record, int position);

#define T_TYPE_COUPLE (t_couple_get_type ())
G_DECLARE_FINAL_TYPE (TCouple, t_couple, T, COUPLE, GObject)

#endif /* __T_CSV_REC_DIALOG_H__ */

