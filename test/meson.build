project('test_csv', 'c')

gtkdep = dependency ('gtk4')

gnome=import('gnome')
resources = gnome.compile_resources('resources','../tcsv.gresource.xml',source_dir: '..')

run_command ('cp', '../com.github.ToshioCP.tcsv.gschema.xml', 'com.github.ToshioCP.tcsv.gschema.xml', check: true)
gnome.compile_schemas(build_by_default: true, depend_files: 'com.github.ToshioCP.tcsv.gschema.xml')

executable('test_csv', ['test_tcsv.c', '../tcsvstr.c', '../tcsvstringlist.c'], dependencies: gtkdep, export_dynamic: true, install: false)
executable('test_modify', ['test_tmodify.c', '../tmodify.c'], dependencies: gtkdep, export_dynamic: true, install: false)
executable('test_rec_dialog', ['test_tcsvrecdialog.c', '../tcsvrecdialog.c', '../tmodify.c', '../tcsvstringlist.c', '../tcsvstr.c'], resources, dependencies: gtkdep, export_dynamic: true, install: false)
executable('test_mod_dialog', ['test_tcsvmoddialog.c', '../tcsvmoddialog.c', '../tmodify.c', '../tcsvstringlist.c', '../tcsvstr.c'], resources, dependencies: gtkdep, export_dynamic: true, install: false)
win_sourcefiles = files('../tcsvalert.c', '../tcsvpref.c', '../tcsv.c', '../pfd2css.c', '../tcsvrecdialog.c', '../tcsvmoddialog.c',
 '../tmodify.c', '../tcsvstringlist.c', '../tcsvstr.c')
executable('test_win', ['test_tcsvwindow.c', win_sourcefiles], resources, dependencies: gtkdep, export_dynamic: true, install: false)
