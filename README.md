# SQLite-CPP

### A C++ implementation of a simple database, created attempting to imitate the internal design of SQLite. Made with help of [this guide](https://github.com/cstack/db_tutorial) and [this book](https://books.google.com/books?id=OEJ1CQAAQBAJ).

### Requiremets
Made for Windows. You also need to have **CMake** installed in order to build the project.

### Installation
1. Clone the repository:
```
git clone https://github.com/lizardwizardd/SQLite-CPP
```
2. Set up the project using CMake:
```
cd SQLite-CPP
cmake -S . -B ./build
```
3. Compile the project:
```
cmake --build ./build
```
4. After compiling you can run database using *db.exe*, or execute tests using *tests.exe*.

### Supported commands
- ```create table [table-name]``` - create a new *[table-name].db* file and open it.
- ```open table [table-name]``` - open an existing *[table-name].db* file.
- ```drop table [table-name]``` - delete an existing *[table-name].db* file.
- ```insert [id] [string1] [string2]``` - insert a new row into the opened database. Length of [string1] <= 32, [string2] <= 255.
- ```update [id] [string1] [string2]``` - update an existing row with new [string1] and [string2] values.
- ```delete [id]``` - soft delete an existing row from the opened database.
- ```select``` - print all rows from the opened database, sorted by primary key in ascending order.
- ```.save``` - save database.
- ```.exit``` - save database and exit the program.
- ```.btree``` - debug command. Prints all inserted row keys in a B-Tree structure.
- ```.constants``` - debug command. Print sizes of constants.
