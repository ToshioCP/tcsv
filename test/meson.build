project('test_csv', 'c')

gtkdep = dependency ('gtk4')

gnome=import('gnome')
resources = gnome.compile_resources('resources','../tcsv.gresource.xml',source_dir: '..')

run_command ('cp', '../com.github.ToshioCP.tcsv.gschema.xml', 'com.github.ToshioCP.tcsv.gschema.xml')
gnome.compile_schemas(build_by_default: true, depend_files: 'com.github.ToshioCP.tcsv.gschema.xml')

executable('test_csv', 'test_tcsv.c', dependencies: gtkdep, export_dynamic: true, install: false)
win_sourcefiles = files('test_tcsvwindow.c', '../tcsvalert.c', '../tcsvpref.c', '../tcsv.c', '../css.c')
executable('test_win', win_sourcefiles, resources, dependencies: gtkdep, export_dynamic: true, install: false)

