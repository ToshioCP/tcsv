project('test_csv', 'c')

gtkdep = dependency ('gtk4')

gnome=import('gnome')
resources = gnome.compile_resources('resources','../tcsv.gresource.xml',source_dir: '..')

run_command ('cp', '../com.github.ToshioCP.tcsv.gschema.xml', 'com.github.ToshioCP.tcsv.gschema.xml')
gnome.compile_schemas(build_by_default: true, depend_files: 'com.github.ToshioCP.tcsv.gschema.xml')

executable('test_csv', ['test_tcsv.c', '../tstr.c'], dependencies: gtkdep, export_dynamic: true, install: false)
win_sourcefiles = files('../tcsvalert.c', '../tcsvpref.c', '../tcsv.c', '../css.c', '../tcsvrecdialog.c', '../tcsvflddialog.c', '../tstr.c')
executable('test_win', ['test_tcsvwindow.c', win_sourcefiles], resources, dependencies: gtkdep, export_dynamic: true, install: false)
rec_dialog_sourcefiles = files('../tcsvwindow.c', '../tcsvalert.c', '../tcsvpref.c', '../tcsv.c', '../css.c', '../tcsvflddialog.c', '../tstr.c')
executable('test_dialog', ['test_tcsvrecdialog.c', rec_dialog_sourcefiles], resources, dependencies: gtkdep, export_dynamic: true, install: false)
executable('test_tstr', 'test_tstr.c', dependencies: gtkdep, export_dynamic: true, install: false)

