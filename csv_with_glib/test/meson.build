project('test_csv', 'c')

giodep = dependency ('gio-2.0')

sourcefiles = files('test_csv.c', '../csv.c')
executable('test_csv', sourcefiles, dependencies: giodep, export_dynamic: true, install: false)

