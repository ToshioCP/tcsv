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
The specification is included in this document.

## Csv libraries in this repository

There are three kinds of csv libraries.

- `tcsv.h` and `tcsv.c` in the top directory.
They are included in the file viewer program.
-  `csv.h` and `csv.c` in `csv_with_glib` directory.
They depend on `glib` and `gio` libraries.
They provide functions to manipulate csv files and a two dimensional array.
-  `csv.h` and `csv.c` in `csv_without_glib` directory.
They are independent from `glib` and `gio` libraries.
They only use the C standard library (`glibc`).
They provide functions to manipulate csv files and a two dimensional array.

Each library has a test program in the `test` directory under each directory.

1. Change your current directory to `test`.
2. Compile them. Type `meson _build` then `ninja -C _build`.
3. Run the test program. For example, type `_build/test_csv`.

The rest of this document describes csv file viewer which includes `tcsv.h` and `tcsv.c`.

## The definition of CSV

According to Wikipedia, the CSV file format is not fully standardized.
The library `tcsv.c` validates, reads and writes a csv file in terms of the following definition.

- The file consists of UTF-8 characters.
- Each line ends with NL (New Line '\n').
At the end of the file, the last line can end either with or without NL. 
- A line consists of fields.
Fields are separated by comma (`,`).
Lines have the same number of fields.
- There are two types of fields.
The one isn't surrounded by double quotes (`"`) and the other is surrounded by double quotes.
The former one is called a bare field (b-field).
It doesn't include any commas and NLs.
And it doesn't begin with double quote (it can end with double quote).
The latter one is called wrapped field (w-field).
The double quotes before or after the w-fields are not included by w-field.
This type of field can include commas and/or NLs.
A double quote in a w-field needs to be escaped by a pair of double quotes.

B-field and w-field are not common term.
They are specific to this document.

Some examples are shown below.

~~~
a, bc, def, ghi
0, 1, 3, 6
one, two, three, and "four".
~~~

Fields are separated with comma.
Any fields are not surrounded by double quote.
If a double quote is in the field, but not at the beginning of the field, it is recognized literally.

~~~
a, "bc", def, "ghi"
0, "1, 2", 3, "4, 5
and 6"
"one", "two", "three", "and ""four""."
~~~

Fields can be surrounded by double quotes.
This type of field can include commas, NLs and pairs of double quotes.

## Manual of tcsv

### Compile

Prerequisite.

- Linux operationg system.
- gcc
- Gtk4
- Glib-2.0 version 2.68.0 or higher

Compiling.

1. Download and decompress the files from this repository.
2. Change your current directory to the top directory of the files above.
3. Type `meson _build`
4. Type `ninja -C _build`

The executable file `tcsv` is created in `_build` directory.

~~~
$ _build/tcsv
~~~

or

~~~
$ _build/tcsv <csv file>
~~~

### Read/write csv

- Click on `Open` button, then you can read a csv file.
- Click on `Save` button, then you can overwrite the csv file which you've read.
- Click on hamberger menu and select `Save as`, then you can write your csv data with a file name.

### Edit the file

- Click on a record, then you can select the record.
Type enter key, then a dialog appears.
You can double click the record as well to show the dialog.
- The dialog has two columns.
The first column contains the header of the csv.
The second column contains the selected record.
- The second column can be editied.
You need to press enter key after the modification of the cell.
If you don't press enter, the data will be lost.
- Press `Save` button, then the data are copied to the selected record.
If you press `Cancel`, then the data will be lost.

### Preference

There's a preference menu.
You can change the font.

### TODO

`Add field` and `Remove field` menu doesn't work now.

## Validation -- the state matrix for CSV

### UTF-8

UTF-8 is defined like this:

- `U+0000` - `U+007F`: These codes are the same as ASCII 7 bit characters.
For example, comma (`,`) is 0x2c (44 in decimal) in ASCII and also in UTF-8.
They, and only they, are one byte UTF-8 codes
- Other codes are 2 to 4 byte characters.
Each byte of them is 0x80 to 0xF4.

This definition enables byte character search program to work with UTF-8 string.
Byte by byte search method can be used for UTF-8 string if the character is from 0x00 to 0x7F.

`tcsv.c` uses this method to search a string for commas, NLs and double quotes.

### The state matrix for CSV

An input function reads one byte at a time.
It changes the state.
For example, the state is "start" at first.
If the first byte isn't a comma, NL or double quote, then the state changes to "in the b-field".
If the next character is a comma, then it makes the state "out of fields".

There are eight states.

- 0 Start. This is the initial state.
- 1 In a b-field.
- 2 A quote before a w-field.
- 3 In a w-field.
- 4 A quote in or after a w-field.
If the quote is in the w-field, it must be a pair of double quotes.
- 5 A comma.
- 6 A NL.
- 7 End. This is the last state.

The example below shows input line and states.

~~~
line:   abcd,"ghi",NL
state: 0111152333456 7
~~~

The strings of fields above are 'abcd', 'ghi' and '' (empty string).

States are determined with two things, the previous state and the input.
The transformation can be represented with a matrix.

|state\input|NL|, |" |other|EOF|
|:----------|:-|:-|:-|:----|:--|
|0          |6 |5 |2 |1    |7  |
|1          |6 |5 |1 |1    |7  |
|2          |3 |3 |4 |3    |-1 |
|3          |3 |3 |4 |3    |-1 |
|4          |6 |5 |3*|-1   |7  |
|5          |6 |5 |2 |1    |7  |
|6          |6 |5 |2 |1    |7  |
|7          |- |- |- |-    |-  |

`-1` in the table means error.

A function `csv_validate` analyzes a csv file with this matrix.
Each line needs to have the same number of fields.
The library has a field counter.
It increases the counter by one when the state is 5.
And it checks if the number of the fields is the same as before when the state is 6.

## Two dimensional data and list

Csv is a two dimensional data set.
When the program reads a csv file, it store the data into two list model.
The one is GListStore.
It is a child of GObject and implements GListModel interface.
Its items are descendants of GObject.
The other is GtkStringList.
It is also a child of GObject and implements GListModel interface.
Its item is a GtkStringObject which is a child of GObject.
GtkStringObject has a "string" property.

GListStore represents a list of records (or rows) of the csv data.
Each item of the GListStore object is GtkStringList.
GtkStringList represents a list of fields in a record of the csv data.

## Reading csv

A function `csv_read` reads csv file.

1. Reads the whole contents of the file into a memory buffer.
2. validates the csv data and gets the size.
3. converts the csv data into an array.

It uses the state matrix in the conversion.

~~~
gboolean
csv_read (GListStore *liststore, GFile *file, int *n_row, int *n_column, GError **err);

Parameters:
  liststore: a list to put records of the csv data. The records include the header (the first line).
  file: GFile object from which the csv data are read.
  n_row: a pointer to `int`. It is set the number of rows of the csv data. It includes the header.
  n_column: a pointer to `int` It is set the number of columns (fields) of the csv data. Each record must have the same number of fields.
  err: If an error occurs, `*err` is set with the error code and message.
Return value:
  TRUE if it succeeds, FALSE if it fails.
~~~

## Writing csv

A function `csv_write` writes csv data into a file.

1. Counts commas, NLs and double quotes in each item (string) of the list.
And decides whether b-field or w- field to convert the string into.
Calculates the csv size and allocate memory.
2. convert the array into csv data and put them into the memory.
3. save the memory into a file.

~~~
gboolean
csv_write (GListStore *liststore, GFile *file, int n_row, int n_column, GError **err);

Parameters:
  liststore: a list of the csv data. It includes the header.
  file: GFile object to which the csv data will be written.
  n_row: The number of rows of the csv data. It includes the header.
  n_column: The number of columns (fields) of the csv data
  err: If an error occurs, `*err` is set with the error code and message.
Return value:
  TRUE if it succeeds, FALSE if it fails.
~~~

## Error reporting

There are two types of errors.
One is a program error.
The other is a runtime error.

Program errors happen because of bugs in the program.
It is reported with error messages to the standard error.
If it is a fatal error, the program quits immediately.
Otherwise, it tries to continue the program as long as possible.

Runtime errors happen because of inputs or some runtime environment.
For example, They are illegal format of csv file or I/O errors.
GError objects are used to report the runtime error.
The caller displays an error message or does some alternative work to cover the error.
Runtime error is usually not a fatal error.

`tcsv.c` makes `TCsvError` domain for the runtime error.
This is based on `G_DEFINE_EXTENDED_ERROR` macro.
It is included to glib in version 2.68.
It is the latest version at present (12/May/2021).
If your glib version is lower than 2.68, it is impossible to compile this program.

## TODO

This program is in development.
It is expected to have the following features in the near future.

- Add/delete fields.

The followings are ideas and probably won't be implemented soon.

- Database like PostgreSQL is available instead of csv.
- Another edit page for one record is available and a user can switch the two type of edit pages.

