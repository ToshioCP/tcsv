# Tcsv -- Csv file editor

Tcsv is a csv file editor.
It runs on Linux operating system and Gtk4 library.
`tcsv` has its own csv read/write functions.

##  license

Tcsv is free; you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the [GNU General Public License](https://www.gnu.org/licenses/gpl-3.0.html) for more details.

## The definition of CSV

According to Wikipedia, the CSV file format is not fully standardized.
TCsv has a library `tcsv.c` which validates, reads and writes a csv file in terms of the following definition.

- The file consists of UTF-8 characters.
- Each line ends with NL (New Line '\n').
At the end of the file, the last line can end either with or without NL.
- A line consists of fields.
Fields are separated by commas (`,`).
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

Fields are separated with commas.
Any fields are not surrounded by double quotes.
(The last field is `and "four"`. It begins with 'a', not a double quote. So it is a B-field.)
If a double quote is in the field, but not at the beginning of the field, it is recognized literally.

~~~
a, "bc", def, "ghi"
0, "1, 2", 3, "4, 5
and 6"
"one", "two", "three", "and ""four""."
~~~

Fields can be surrounded by double quotes.
This type of field can include commas, NLs and pairs of double quotes.

## Tcsv manual

### Compile

Prerequisites.

- Linux operating system.
- gcc
- Gtk4 version 4.8 or higher
- Glib-2.0 version 2.68.0 or higher

Installation.

1. Download and decompress the files from this repository.
2. Change your current directory to the top directory of the files above.
3. Type `meson _build`
4. Type `ninja -C _build`
5. Type "sudo ninja -C _build install"

The executable file `tcsv` is created in `/usr/local/bin` directory.
Type `tcsv`, then `tcsv will be executed.
~~~
$ tcsv
~~~

If you want to install it in your user space, add `--prefix` option to meson.
The following shows that the executable program is installed to `$HOME/.local/bin`.
You don't need `sudo` because the installation target directory is in your user space.
You may need to re-login.

~~~
$ meson --prefix=$HOME/.local _build
$ ninja -C _build
$ ninja -C _build install
~~~

### Main window

Execute `tcsv`.
A main window appears.
There's a title bar at the top of the window.
It has title, minimize button, maximize/restore button and close button.
(It depends on your windowing sysytem, though.)
There's a tool bar below the title bar.
It has nine buttons.
Five buttons `Append Record`, `Remove Record`, `Edit Record`, `↑` and `↓` are on the left.
And four buttons `Open`, `Save`, `Close` and a menu button (hamburger menu) are on the right.
These nine buttons are often used.

If you click on the menu button, a popup menu appears.
There are seven menus.
They are `Insert Records`, `Modify Fields`, `Reset Sorting`, `New`, `Save As…`, `Preference` and `Quit`.
They are less used than the buttons on the tool bar.

### Read/Write a csv file

- Click on `Open` button, then you can read a csv file.
- Click on `Save` button, then you can overwrite the csv file which you've read.
- Click on the menu button and select `Save as` menu, then you can write your csv data with a file name.

There're sample csv files in the top directory of the source files.
One of them is "tcsv_object.csv".
Click on the `Open` button, then a file chooser dialog appears.
Select "tcsv_object.csv", you might need to select the directory first, and click on `Open` button.
Then, the contents of the file appears in the main window.

Click on the `Close` button, then the contents disappear.

### Csv, field, header and record

Csv data has rows and columns.
The first row is a header.
The second row and after are called records.
Each element is called a field.
The columns are also called fields.

If you read a csv file, the headers appear at the top of the table.
Records are shown in the body of the table.
If the number of records are more than the number of rows in the screen, you can scroll it to see hidden records.

### Create a new csv

Click on the menu button and select `New` menu.
Then "Modify fields" dialog appears.

The first column is "current".
The cells in the first column is buttons.
If you click on a button, the row becomes "current row" and the bottun color changes to red.
Only one row can be a "current row".

There are five buttons on the tool bar.

- `Append new field`
- `Insert new field`.
- `Remove field`
- `↑`
- `↓`
  
Click on `Append new field`, then a new row is added to the next of the current row or the end of the rows if no current row exists.
Click on `Insert new field`, then a new row appears before the current row or the first row if no current row exists.
The last column of the rows are entries (GtkText).
You can enter characters from your keyboard.
But only visible characters can be inputted.
For example, you can't put NL (new line) into the buffer.

Type "field 1", for example.
You need to put non-empty string to the entry.
If the entry is empty, the field won't be added to the table in the main window.
Then click on `Append Field` button twice.
Two new rows appear.next to the row "field 1".
Type "field 2" and "field 3" on the new two entries.
Now you mad three fields.
If you want to remove "field 3", click on the bottun on the row and click on "Remove field" button.

Click on the `Save` button at the bottom of the dialog, then the dialog disappears.
And the main window appears with fields you made.
It has two fields, `field 1` and `field 2`.
(The field `field 3` are deleted by you so it didn't appear on the main window.)

### Create/edit a record

There is a button at the left of each row.
If you click on the button, the button color changes to red.
The red button indicates the row is current.
If you click on another bottun, the new button is colored red and the previous red button changes its color to white.
So, there is only one red color button at most.

When a current row exists (that means the row has a red button),
you can click on "Append Record", "Remove Record", "Edit Record", "↑" and "↓" buttons.

- Append Record: appends a new row after the current row.
- Remove Record:  removes the current row. After that, no current row exists.
- Edit Record: A new dialog appears and you can edit the current record.
- ↑: Move up the current row by one.
- ↓: Move down the current row by one.

You can click on the "Insert Record" menu and insert a new row before the current row.

When you clicked on the "Edit Record" button, a dialog appears.
There are two columns.
"Header" and "Record".
You can't edit "Header" column but you can edit "Record" column.
After editing, click on the `Save` button.
Then the dialog disappears and the change is seen in the main window.

You can edit records in the main window.
However, the entries in it are not so big.
If you enter a long string, a part of it will be hidden.
"Edit Record" button and the edit dialog is helpful for such long strings.

### Save csv data

Click on the "Save" button.
If you make a new csv data, a file chooser dialog appears.
Type "example.csv" and click on `Save` button.
Then a file "example.csv" is created and the contents of the table are written to the file.

Click on the "Close" button, then the contents disappear.

Click on the "Open" button, select "example.csv" and click on the "Open" button, then the contents of "example.csv" appears again.

### Restructure fields

Csv is similar to database.
Fields should be defined carefully before data are inputted.
However, sometimes you need to change the fields later.
You can restructure the fields by clicking `Modify Fields` menu.

If you click the menu, "Modify Fields" dialog appears.
It has six columns.

- Current: shows whether the row (field) is current or not. If you click on the button on the "current" column, the row will be "current".
- Old position: The position of the old field. This never changes even if rows are added or deleted.
- Old Field: The names of the old fields. They can't be modified.
- State: There are three states, "Changed", "Added" and "Removed".
- New position: The position of the new fields. This is the same as the positions of the rows.
- New Field: The names of the new fields. They can be modified.
If the name of a new field is an empty string, the field will be removed.

There are five buttons.
These are the same as the buttons on the dialog when you click on the "New" menu.

Now, let's try to restructure fields of "example.csv".
If your table in the main window is empty, click on the "Open" button and read "example.csv".
Then click on the `Modify Fields` menu.

First, the "Modify Fields" dialog is as follows.

|Current|Old position|Old field|State|New position|New field|
|:-:|:----------:|:-------:|:---:|:----------:|:-------:|
|   |    0       |field 1  |     |     0      | field 1 |
|   |    1       |field 2  |     |     1      | field 2 |

- Change "field 1" into "City".
- Change "field 2" into "Country".
- Click the `Append new field` button.
Then, a new field is added at the bottom of the list.
- Fill in the new field with "Population".
- Click the `Append new field` button and append a new field.
- Fill in the new field with "Area".

|Current|Old position|Old field| State |New position|New field |
|:-:|:----------:|:-------:|:-----:|:----------:|:--------:|
|   |    0       |field 1  |changed|     0      |   City   |
|   |    1       |field 2  |changed|     1      | Country  |
|   |   -1       |         |Added  |     2      |Population|
|   |   -1       |         |Added  |     3      |   Area   |

Click on the `Save` button.
The dialog disappears and the new fields appear on the header of the table in the main window.

Edit the records.

|      City       |Country|Population|Area|
|:---------------:|:-----:|:--------:|:--:|
| Shinjuku (Tokyo)| Japan |  346235  | 18 |
|      Osaka      | Japan | 2668586  |225 |
|     London      |England| 8961989  |1572|
|  New York City  |  USA  | 8175133  |486 |

Click on the "Save As…" menu and save it with the name "city.csv".

### Sorting

Sorting is supported.
Click on a header so that the table will be sorted according to the string sorter.
If you want to reset the table, click on the `Reset Sorting` menu.

Sorting doesn't affect the order of the original data.
So, when you save the data to a file, the order of the data in the file is the same as the original one.

### Preference

There's a preference menu.
You can change the font (font-family, font-style, weight and size).

## Csv libraries in this repository

There are three kinds of csv libraries.

- `tcsv.h` and `tcsv.c` in the top directory.
They are included in the file editor program.
They read a csv file and set the header and body of the list.
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

### Validation -- the state matrix for CSV

#### UTF-8

UTF-8 is defined like this:

- `U+0000` - `U+007F`: These codes are the same as ASCII 7 bit characters.
For example, comma (`,`) is 0x2c (44 in decimal) in ASCII and also in UTF-8.
They, and only they, are one byte UTF-8 codes
- Other codes are 2 to 4 byte characters.
Each byte of them is 0x80 to 0xF4.

This definition enables byte character search program to work with UTF-8 string.
Byte by byte search method can be used for UTF-8 string if the character is from 0x00 to 0x7F.

`tcsv.c` uses this method to search a string for commas, NLs and double quotes.

#### The state matrix for CSV

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

### Two dimensional data and list

Csv is a two dimensional data set.
When the program reads a csv file, it stores the data into two list model.
The one is GListStore for rows.
GListStore is a child of GObject and implements GListModel interface.
Its items are descendants of GObject.
The other is GtkStringList for columns.

The GListStore represents a list of records (or rows) of the csv data.
Each item of the GListStore object is a GtkStringList instance.
The GtkStringList represents a record which is a list of strings.

### Reading csv

A function `csv_read` reads csv file.

1. Reads the whole contents of the file into a memory buffer.
2. Validates the csv data and gets the size.
3. Converts a line of the csv data into an GtkStringList object, which is called a record.
4. The record is appended to the GListStore.

It uses the state matrix in the conversion.

~~~
gboolean
csv_read (GListStore *liststore, GFile *file, int *n_row, int *n_column, GError **err);

Parameters:
  liststore: a list to put records of the csv data. The records include the header (the first line).
  file: GFile object from which the csv data are read.
  n_row: a pointer to `int`. It is set to the number of rows of the csv data. It includes the header.
  n_column: a pointer to `int` It is set to the number of columns (fields) of the csv data. Each record must have the same number of elements.
  err: If an error occurs, `*err` is set with the error code and message.
Return value:
  TRUE if it succeeds, FALSE if it fails.
~~~

### Writing csv

A function `csv_write` writes csv data into a file.

1. Counts commas, NLs and double quotes in each item (string) of the list.
And decides whether b-field or w- field to convert the string into.
Calculates the csv size and allocate memory.
2. convert the data into csv data and put them into the memory.
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
If your glib version is lower than 2.68, it is impossible to compile `tcsv` source files.

## TODO

This program is in development.
It is expected to have the following features in the future.

- Database like PostgreSQL is available instead of csv.
- Support more preference.
For example, the default width of fields.
- Support report format and output it as pdf formats.
