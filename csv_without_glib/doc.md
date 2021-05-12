`csv.h` and `csv.c` under the directory `csv_without_glib` don't depend on glib and gio.
Glibc is the only necessary library.
`test_csv.c`, which is in `test` directory, is a test program for `csv.c`.

1. Change your current directory to `test`.
2. Type `meson _build`.
3. Type `ninja -C _build` for compilation.
4. Run `_build/test_csv` to test `csv.c`.

The rest of this document describes:

- how to use `csv.c`.
- Algorithm that is used in `csv.c`

According to Wikipedia, the CSV file format is not fully standardized.
The library `csv.c`validates, reads and writes a csv file in terms of the following definition.

## The definition of CSV

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

`csv.c` uses this method to search a string for commas, NLs and double quotes.

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

## Two dimensional array

Csv is two dimensional data set.
And each data is a string.
This type of data set is represented by an array (`char *array[][]`) in C program.
But the size of the array is determined when the csv file is given.
So, it's impossible to define the array when it is programmed.
Instead, a pointer (`char ***array`) is used.
Manipulating the pointer is complicated, so four functions are provided.

- `char ***array_alloc (int n_i, int n_j);`
Allocates memory for an `n_i`x`n_j` sized array and returns the pointer to the array.
- `void array_free (char ***array, int n_i, int n_j);`
Frees the memory of the array and strings pointed by the array.
- `void array_put (char ***array, int i, int j, char *s)`;
Assigns a string `s` to `array[i][j]`.
- `char *array_get (char ***array, int i, int j);`
Gets a string `array[i][j]`.

~~~C
/*
Prefix 'n_': 'number of'. For example, n_i is the number of i, n_row is the number of rows.

static char ***
array_alloc (int n_i, int n_j);
  parameter
    n_i: size of row
    n_j: size of column
  return: a pointer to the allocated memory.

static void
array_free (char ***array, int n_i, int n_j);
  Frees each string pointed by **array.
  And frees the memories allocated by array_alloc.
  parameter
    array: a pointer that has returned by array_alloc.
    n_i: size of row
    n_j: size of column

void
array_put (char ***array, int i, int j, char *s);
  Put the string s to the array[i][j].
  Array take the ownership of the string s.

const char *
array_get (char ***array, int i, int j)
  Get a string from the array[i][j].
  The returned string is owned by array.
  You need to duplicate it if necessary.
*/
~~~

`csv.c` transfer data between an array and csv.
It reads csv and puts them into an array.
It writes an array to csv.

## Reading csv

A function `csv_read` reads csv file.

1. Reads the whole contents of the file into a memory buffer.
2. validates the csv data and gets the size.
3. converts the csv data into an array.

It uses the state matrix in the conversion.

## Writing csv

A function `csv_write` writes array data into a csv file.

1. Counts commas, NLs and double quotes in each string in the array.
And decides whether b-field or w- field to convert the string into.
Calculates the csv size and allocate memory.
2. convert the array into csv data and put them into the memory.
3. save the memory into a csv file.

## Error reporting

There are two types of errors.
One is a program error.
The other is a runtime error.

Program errors happen because of bugs in the program.
It is reported with error messages to the standard error.
If it is a fatal error, the program quits immediately.
Otherwise, it tries to continue the program as long as possible.

Runtime errors happen because of inputs or some runtime environment.
For example, it is illegal format of csv file or I/O errors.
Functions return a pointer to an error structure (`struct Csverror`) to report the runtime error.
The caller displays an error message or does some alternative work to cover the error.
Runtime error is usually not a fatal error.

## Test program

There is a test program `test_csv.c` in the `test` directory.
It is useful to know how to use `csv.c` and manipulate error system.

