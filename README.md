# cbGcov
cbGcov is a Code::Blocks Plugin for Gcov integration.
Gcov is a source code coverage analysis and statement-by-statement profiling tool.
The GNU Compiler Collection (GCC) suite provides gcov.

## Configuration
![alt text](https://github.com/danselmi/cbGcov/raw/master/doc/ConfigScreen.png "Configuration dialog")


## Usage
### Prepare Project ###
First, you have to prepare your project for using gcov. With the help of cbGcov, this is simple:
![alt text](https://github.com/danselmi/cbGcov/raw/master/doc/PrepareProject.png "")

This adds the compiler options `-fprofile-arcs` and `-ftest-coverage` and
adds `gcov` for the linker to the linked libraries. Also the `-coverage` option is added for the linker.
After that, your project needs be rebuilt.

If the project already enables the needed options for the linker and compiler, there is no need to do this again.
This step needs to be done only once per project.

### Get the coverage Data ###
When the project has been built it is time to run it and let gcov collect the data. This is ideally done with a unit test project.

Now it is time to run gcov over the project:\
![alt text](https://github.com/danselmi/cbGcov/raw/master/doc/RunGcov.png "")

After gcov has finished, cbGcov will show a summary for the project:\
![alt text](https://github.com/danselmi/cbGcov/raw/master/doc/SummaryScreen.png "")

After opening a sourcefile (by doubleclicking it in project manager or cbGcov summary) the file is shown with annotations from gcov:\
![alt text](https://github.com/danselmi/cbGcov/raw/master/doc/AnnotatedSource.png "")

With the toolbar ![alt text](https://github.com/danselmi/cbGcov/raw/master/doc/Toolbar.png "") you can goto next/previous line with a coverage of 0 (red) or a coverage >0 (green).

The coverage data will be accumulated from multiple runs of your program. Simply run gcov again (after running your program) and see that the coverage has changed.

After changes in the sources and compiling, all coverage will be 0 until the program has been run at leat once.

