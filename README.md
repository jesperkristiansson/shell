# shell
A shell that is actively being developed.

Compile the executable by running `make` in the build-directory

This project is being developed for, and has only been tested on Ubuntu 20.04, other platforms might experience problems.

## Current functionality
- Run programs with `programname [argumentlist]`
  - Use raw text by surrounding it with `"`, e.g. `programname "string1 string2"`
- Input/output-redirection
  - Redirect stdin from file with `program < file`
  - Redirect stdout to file with `program > file`
  - Pipe input and output between programs with `program | program`
    - This executes the programs sequentially, in the future this should be extended to properly run all programs in parallell 
- Run programs as background processes using `program &`
- Run multiple programs with one line of input by seperating them with `;`, e.g. `program; program`
- Move cursor using left and right arrow keys
  - Insertion and deletion(currently only works with backspace, support for the delete-button is coming) works wherever the cursor is located
- Traverse directories using `cd directory`
- Exit the shell by running `exit` or press ctrl+D
- Set environment variables with `export $name=value`