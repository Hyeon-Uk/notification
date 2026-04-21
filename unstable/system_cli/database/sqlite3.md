# sqlite3 - Command-line interface for SQLite database

## NAME

**sqlite3** - A command-line interface to the SQLite database library.

## SYNOPSIS

```bash
sqlite3 [OPTIONS] [FILENAME [SQL...]]
```

## DESCRIPTION

**sqlite3** is a command-line interface to the SQLite database library. SQLite is a C library that implements an embeddable SQL database engine. Programs that link with the SQLite library can have SQL database access without running a separate RDBMS process.

SQLite is not a client library used to connect to a big database server. SQLite is the server and the SQLite library reads and writes directly to and from the database files on disk.

If FILENAME is provided and the file does not exist, a new database is created. If FILENAME is not provided, an in-memory database (`:memory:`) is used.

## OPTIONS

### General Options

| Option | Description |
|--------|-------------|
| `-help` | Show help message. |
| `-version` | Show SQLite version. |
| `-` | Treat no subsequent arguments as options. |
| `-cmd COMMAND` | Run "COMMAND" before reading stdin. |
| `-init FILENAME` | Read/process named file. |

### Database Options

| Option | Description |
|--------|-------------|
| `-append` | Append the database to the end of the file. |
| `-deserialize` | Open the database using sqlite3_deserialize(). |
| `-maxsize N` | Maximum size for a --deserialize database. |
| `-mmap N` | Default mmap size set to N. |
| `-nofollow` | Refuse to open symbolic links to database files. |
| `-readonly` | Open the database read-only. |
| `-safe` | Enable safe-mode. |
| `-vfs NAME` | Use NAME as the default VFS. |

### Output Mode Options

| Option | Description |
|--------|-------------|
| `-ascii` | Set output mode to 'ascii'. |
| `-box` | Set output mode to 'box'. |
| `-column` | Set output mode to 'column'. |
| `-csv` | Set output mode to 'csv'. |
| `-html` | Set output mode to HTML. |
| `-json` | Set output mode to 'json'. |
| `-line` | Set output mode to 'line'. |
| `-list` | Set output mode to 'list'. |
| `-markdown` | Set output mode to 'markdown'. |
| `-quote` | Set output mode to 'quote'. |
| `-table` | Set output mode to 'table'. |
| `-tabs` | Set output mode to 'tabs'. |

### Output Formatting Options

| Option | Description |
|--------|-------------|
| `-[no]header` | Turn headers on or off. |
| `-separator SEP` | Set output column separator. Default: '\|'. |
| `-newline SEP` | Set output row separator. Default: '\n'. |
| `-nullvalue TEXT` | Set text string for NULL values. Default ''. |
| `-escape T` | Control-char escape; T is one of: symbol, ascii, off. |

### I/O Options

| Option | Description |
|--------|-------------|
| `-batch` | Force batch I/O. |
| `-interactive` | Force interactive I/O. |
| `-bail` | Stop after hitting an error. |
| `-echo` | Print inputs before execution. |

### Debug/Trace Options

| Option | Description |
|--------|-------------|
| `-memtrace` | Trace all memory allocations and deallocations. |
| `-pcachetrace` | Trace all page cache operations. |
| `-stats` | Print memory stats before each finalize. |
| `-vfstrace` | Enable tracing of all VFS calls. |
| `-lookaside SIZE N` | Use N entries of SZ bytes for lookaside memory. |
| `-pagecache SIZE N` | Use N slots of SZ bytes each for page cache memory. |

### Advanced Options

| Option | Description |
|--------|-------------|
| `-nonce STRING` | Set the safe-mode escape nonce. |
| `-no-rowid-in-view` | Disable rowid-in-view using sqlite3_config(). |
| `-unsafe-testing` | Allow unsafe commands and modes for testing. |

## EXIT STATUS

| Exit Code | Description |
|-----------|-------------|
| `0` | Success. Command completed without errors. |
| `1` | Error. SQL error or invalid arguments. |

## EXAMPLES

### Open an in-memory database

```bash
$ sqlite3
SQLite version 3.50.2
Enter ".help" for usage hints.
Connected to a transient in-memory database.
sqlite>
```

### Open a database file

```bash
$ sqlite3 mydb.db
```

### Execute SQL directly

```bash
$ sqlite3 mydb.db "SELECT * FROM users;"
```

### Create a new database with a table

```bash
$ sqlite3 mydb.db "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT);"
```

### Insert data

```bash
$ sqlite3 mydb.db "INSERT INTO users (name) VALUES ('Alice');"
```

### Query with CSV output

```bash
$ sqlite3 -csv mydb.db "SELECT * FROM users;"
id,name
1,Alice
```

### Query with JSON output

```bash
$ sqlite3 -json mydb.db "SELECT * FROM users;"
[{"id":1,"name":"Alice"}]
```

### Query with table output

```bash
$ sqlite3 -table -header mydb.db "SELECT * FROM users;"
+----+-------+
| id | name  |
+----+-------+
| 1  | Alice |
+----+-------+
```

### Run multiple SQL commands from file

```bash
$ sqlite3 mydb.db < commands.sql
```

### Use with echo to show commands

```bash
$ sqlite3 -echo mydb.db "SELECT * FROM users;"
SELECT * FROM users;
1|Alice
```

### Open database read-only

```bash
$ sqlite3 -readonly mydb.db "SELECT * FROM users;"
```

## BUILT-IN COMMANDS

Inside the SQLite interactive shell, these special commands (starting with `.`) are available:

| Command | Description |
|---------|-------------|
| `.help` | Show help for all commands. |
| `.quit` | Exit the program. |
| `.tables` | List all tables. |
| `.schema` | Show the CREATE statements. |
| `.databases` | List all databases. |
| `.mode MODE` | Set output mode (csv, column, line, list, etc.). |
| `.headers ON\|OFF` | Turn headers on or off. |
| `.separator SEP` | Set column separator. |
| `.import FILE TABLE` | Import data from FILE into TABLE. |
| `.dump` | Dump database as SQL text. |
| `.output FILE` | Send output to FILE. |

## NOTES

- SQLite databases are stored as single files on disk.
- No server process is required; the library accesses files directly.
- Supports most SQL92 features including transactions, triggers, and views.
- The default database is `:memory:` if no filename is provided.
- Use `.help` inside the interactive shell to see all built-in commands.

## SEE ALSO

**sqlite**(3), **sqlite3_api**(3)

## PACKAGE

`sqlite`