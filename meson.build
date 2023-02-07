project('csv', 'c')

glibdep = dependency('glib-2.0', version: '>=2.68.0')
gtkdep = dependency('gtk4')

gnome=import('gnome')
resources = gnome.compile_resources('resources','tcsv.gresource.xml')
gnome.compile_schemas(build_by_default: true, depend_files: 'com.github.ToshioCP.tcsv.gschema.xml')

sourcefiles=files('main.c', 'tcsvapplication.c', 'tcsvwindow.c', 'tcsvpref.c', 'tcsvalert.c', 'tcsv.c', 'pfd2css.c',
                  'tcsvrecdialog.c', 'tcsvmoddialog.c', 'tmodify.c', 'tcsvstringlist.c', 'tcsvstr.c')

executable('tcsv', sourcefiles, resources, dependencies: gtkdep, export_dynamic: true, install: true)

schema_dir = get_option('prefix') / get_option('datadir') / 'glib-2.0/schemas/'
install_data('com.github.ToshioCP.tcsv.gschema.xml', install_dir: schema_dir)
meson.add_install_script('glib-compile-schemas', schema_dir)

