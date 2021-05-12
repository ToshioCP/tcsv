# Csv file viewer

## Development mode, license

The program in this repository is in a development mode and unstable.

It is free; you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the [GNU General Public License](https://www.gnu.org/licenses/gpl-3.0.html) for more details.

## About the program in this repository

`tcsv` is a csv file viewer.
It uses GTK4.
The main widget is GtkColumnView.
It is a widget newly added to GTK4.

`tcsv` has its own csv read/write functions.

## Documentation

There's a [document (`doc.md`)](doc.md) in the top directory of this repository.

## Compile

Prerequisites.

- Gtk4
- Glib 2.68.0 or later

Compiling.

~~~
$ meson _build
$ ninja -C _build
~~~

Running.

~~~
$ _build/tcsv
~~~

No installation is supported now because of the development version.
