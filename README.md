# 🐚 Shellux - A Custom Unix Shell

<p align="center">
  <img src="https://img.shields.io/badge/Language-C-blue.svg" alt="Language">
  <img src="https://img.shields.io/badge/Platform-Linux-green.svg" alt="Platform">
  <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License">
  <img src="https://img.shields.io/badge/Status-Active-brightgreen.svg" alt="Status">
</p>

## 📖 Overview

**Shellux** (Shell + Linux) is a fully-featured custom Unix shell written in C. It provides an interactive command-line interface with support for built-in commands, process management, I/O redirection, pipes, background processes, signal handling, command history, aliases, and user-defined functions.

This shell was developed as a systems programming project to demonstrate understanding of:
- Process creation and management (`fork`, `exec`, `wait`)
- Signal handling (SIGINT, SIGTSTP, SIGCHLD)
- File descriptors and I/O redirection
- Inter-process communication via pipes
- Terminal control and raw mode
- Network programming for man page fetching

---

## ✨ Features at a Glance

| Feature | Description |
|---------|-------------|
| 🏠 **Custom Prompt** | Dynamic prompt showing `<username@hostname:path>` |
| 📂 **hop** | Built-in `cd` command with path shortcuts |
| 📋 **reveal** | Built-in `ls` command with color coding |
| 🔍 **seek** | Built-in file/directory search command |
| 📝 **log** | Command history with persistence |
| 🔬 **proclore** | Process information inspector |
| 📡 **iMan** | Fetch man pages from the internet |
| 👶 **neonate** | Monitor latest PIDs in real-time |
| ⏪ **Background Processes** | Run commands with `&` |
| 🔀 **Pipes** | Chain commands with `|` |
| 📥 **I/O Redirection** | Support for `<`, `>`, `>>` |
| 🎯 **Signal Handling** | Ctrl+C, Ctrl+Z, Ctrl+D |
| 📌 **fg/bg** | Foreground/background process control |
| 🏷️ **Aliases** | Custom command shortcuts |
| 🔧 **Functions** | User-defined shell functions |

---

## 🚀 Getting Started

### Prerequisites

- **Operating System**: Linux (tested on Ubuntu 20.04+)
- **Compiler**: GCC with C11 support
- **Build Tool**: Make

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/iDheer/MiniProject-1.git
   cd MiniProject-1
   ```

2. **Build the shell**
   ```bash
   make
   ```

3. **Run the shell**
   ```bash
   ./bin/shellux
   ```

### Alternative Build Commands

```bash
# Debug build (with symbols)
make debug

# Release build (optimized)
make release

# Clean build artifacts
make clean

# Full rebuild
make rebuild

# Install system-wide (requires sudo)
make install

# Uninstall
make uninstall
```

---

## 🎮 Usage

Once you start the shell, you'll see a colorful prompt:

```
<username@hostname:~>
```

The prompt shows:
- **Username**: Current user
- **Hostname**: System name
- **Path**: Current directory (with `~` representing the shell's home directory)

### Exiting the Shell

- Type `exit` and press Enter
- Press `Ctrl+D` (EOF signal)

---

## 📚 Built-in Commands

### 1. 🏠 `hop` - Change Directory

Navigate between directories with enhanced path handling.

**Syntax:**
```bash
hop [path] [path2] ...
```

**Examples:**
```bash
hop                    # Go to shell's home directory
hop ~                  # Go to shell's home directory
hop ..                 # Go to parent directory
hop .                  # Stay in current directory (prints path)
hop -                  # Go to previous directory
hop /absolute/path     # Go to absolute path
hop ~/Documents        # Go to Documents in user's home
hop dir1 dir2 dir3     # Navigate through multiple directories
```

**Features:**
- Supports `~` for home directory
- Supports `-` for previous directory (like cd -)
- Supports `.` and `..` for relative paths
- Prints the new working directory after each hop
- Can handle multiple paths in sequence

---

### 2. 📋 `reveal` - List Directory Contents

List files and directories with color coding and detailed information.

**Syntax:**
```bash
reveal [flags] [path]
```

**Flags:**
| Flag | Description |
|------|-------------|
| `-a` | Show all files including hidden ones (starting with `.`) |
| `-l` | Long format with permissions, size, dates, etc. |

**Examples:**
```bash
reveal                 # List current directory
reveal -a              # Show all files including hidden
reveal -l              # Long format listing
reveal -al             # Combined: all files in long format
reveal -la             # Same as above (order doesn't matter)
reveal ~/Documents     # List specific directory
reveal -al ~           # All files in home, long format
```

**Color Coding:**
- 🔵 **Blue**: Directories
- 🟢 **Green**: Executable files
- ⚪ **White**: Regular files

**Long Format Output:**
```
drwxr-xr-x 2 user group  4096 Dec 18 14:30 Documents
-rwxr-xr-x 1 user group 12288 Dec 18 14:25 script.sh
-rw-r--r-- 1 user group   256 Dec 18 14:20 file.txt
```

---

### 3. 📝 `log` - Command History

Manage and execute commands from history. The log persists between sessions.

**Syntax:**
```bash
log                     # Display command history
log purge               # Clear all history
log execute <index>     # Execute command at index
```

**Examples:**
```bash
log                     # Show last 15 commands (numbered)
log purge               # Clear the history
log execute 3           # Re-execute the 3rd most recent command
```

**Features:**
- Stores up to 15 commands
- Commands containing "log" are not stored (prevents recursive logging)
- Consecutive duplicate commands are not stored
- History is saved to `command_log.txt` and persists across sessions

---

### 4. 🔍 `seek` - Search for Files and Directories

Recursively search for files and directories matching a pattern.

**Syntax:**
```bash
seek [flags] <target> [directory]
```

**Flags:**
| Flag | Description |
|------|-------------|
| `-d` | Search for directories only |
| `-f` | Search for files only |
| `-e` | Execute: open file/cd into directory if exactly one match |

**Examples:**
```bash
seek file               # Search for files/dirs starting with "file"
seek -f config          # Search for files starting with "config"
seek -d src             # Search for directories starting with "src"
seek -e main.c          # Find and open main.c if single match
seek -de build          # Find and cd into 'build' dir if single match
seek config ~/project   # Search in specific directory
```

**Output Color Coding:**
- 🔵 **Blue**: Directories
- 🟢 **Green**: Files

**Special Behavior with `-e`:**
- If exactly one **file** matches: displays its contents
- If exactly one **directory** matches: changes into that directory
- If multiple matches: only prints results

---

### 5. 🔬 `proclore` - Process Information

Display detailed information about a process.

**Syntax:**
```bash
proclore [pid]
```

**Examples:**
```bash
proclore                # Show info about the shell process
proclore 1234           # Show info about process with PID 1234
```

**Output:**
```
PID: 12345
Process Group: 12345
State: S (sleeping)
VmSize: 4096 kB
Executable Path: /bin/bash
```

**State Indicators:**
- `R` - Running
- `S` - Sleeping (interruptible)
- `T` - Stopped
- `Z` - Zombie
- `+` suffix - Foreground process

---

### 6. 📡 `iMan` - Internet Man Pages

Fetch manual pages from the internet using `man.he.net`.

**Syntax:**
```bash
iMan <command_name>
```

**Examples:**
```bash
iMan ls                 # Fetch man page for 'ls'
iMan grep               # Fetch man page for 'grep'
iMan fork               # Fetch man page for 'fork' syscall
```

**Requirements:**
- Internet connection
- Port 80 accessible

---

### 7. 👶 `neonate` - PID Monitor

Continuously print the PID of the most recently created process.

**Syntax:**
```bash
neonate -n <time_interval>
```

**Examples:**
```bash
neonate -n 1            # Print latest PID every 1 second
neonate -n 5            # Print latest PID every 5 seconds
```

**Controls:**
- Press `x` to stop the monitor and return to shell

---

### 8. 🎯 `ping` - Send Signals to Processes

Send signals to processes by PID.

**Syntax:**
```bash
ping <pid> <signal_number>
```

**Examples:**
```bash
ping 1234 9             # Send SIGKILL to process 1234
ping 1234 15            # Send SIGTERM to process 1234
ping 1234 19            # Send SIGSTOP to process 1234
ping 1234 18            # Send SIGCONT to process 1234
```

**Note:** Signal numbers are taken modulo 32.

---

### 9. 📌 `fg` - Bring to Foreground

Bring a background process to the foreground.

**Syntax:**
```bash
fg <pid>
```

**Examples:**
```bash
fg 1234                 # Bring process 1234 to foreground
```

---

### 10. 📌 `bg` - Resume in Background

Resume a stopped background process.

**Syntax:**
```bash
bg <pid>
```

**Examples:**
```bash
bg 1234                 # Resume process 1234 in background
```

---

### 11. 📊 `activities` - List Background Processes

Display all background processes spawned by the shell.

**Syntax:**
```bash
activities
```

**Output:**
```
[1234] : sleep 100 - Running
[5678] : vim file.txt - Stopped
```

**States:**
- `Running` - Process is running
- `Stopped` - Process is stopped (Ctrl+Z)
- `Terminated` - Process has exited

---

## 🔀 Pipes and I/O Redirection

### Input Redirection (`<`)

Read input from a file instead of keyboard.

```bash
sort < unsorted.txt
wc -l < file.txt
```

### Output Redirection (`>`)

Write output to a file (overwrites existing content).

```bash
ls > files.txt
echo "Hello" > greeting.txt
```

### Append Redirection (`>>`)

Append output to a file.

```bash
echo "New line" >> log.txt
date >> timestamps.txt
```

### Pipes (`|`)

Connect the output of one command to the input of another.

```bash
ls | wc -l                      # Count files
cat file.txt | grep pattern     # Search in file
ps aux | grep firefox | head    # Complex pipeline
```

### Combined Example

```bash
cat input.txt | sort | uniq > output.txt
grep error < log.txt | wc -l >> count.txt
```

---

## ⏪ Background Processes

Run commands in the background by appending `&`.

```bash
sleep 100 &                     # Run sleep in background
gedit file.txt &                # Open editor in background
./long_script.sh &              # Run script in background
```

**Features:**
- Background processes print their PID when started
- Use `activities` to list all background processes
- Use `fg <pid>` to bring to foreground
- Use `bg <pid>` to resume stopped process
- Shell notifies when background processes exit

---

## 🎯 Signal Handling

### Keyboard Signals

| Signal | Key | Behavior |
|--------|-----|----------|
| SIGINT | `Ctrl+C` | Terminates foreground process (not the shell) |
| SIGTSTP | `Ctrl+Z` | Stops foreground process, moves to background |
| EOF | `Ctrl+D` | Exits shell (after killing all background processes) |

### Signal Behavior

- **Foreground process running**: Signal affects only the foreground process
- **No foreground process**: Signal is ignored with a message
- **Shell protection**: The shell itself ignores Ctrl+C and Ctrl+Z

---

## 🏷️ Aliases and Functions

### Configuration File

The shell reads `shellux.myshrc` on startup to load aliases and functions.

### Alias Format

```bash
alias shortname=command
```

**Example `shellux.myshrc`:**
```bash
alias dikhao=ls
alias cls=clear
alias ll=ls -la
```

**Usage:**
```bash
dikhao                  # Executes 'ls'
cls                     # Executes 'clear'
```

### Function Format

```bash
function_name()
{
    command1 "$1"
    command2
}
```

**Example Functions:**
```bash
mk_hop()
{
    mkdir "$1"
    hop "$1"
}

hop_seek()
{
    hop "$1"
    seek "$1"
}
```

**Usage:**
```bash
mk_hop new_project      # Creates 'new_project' and cd's into it
hop_seek Documents      # cd to Documents and search there
```

**Note:** `$1` is replaced with the first argument passed to the function.

---

## 🔄 Multiple Commands

Execute multiple commands in sequence using `;`.

```bash
hop ~ ; reveal ; proclore
mkdir test ; hop test ; reveal
```

Commands are executed left to right, regardless of previous command's exit status.

---

## 🎨 Prompt Customization

The prompt is dynamically generated with colors:

```
<username@hostname:path>
```

| Component | Color |
|-----------|-------|
| `<`, `>` | Teal (bold) |
| `username@hostname` | Teal (bold) |
| `:` | Default |
| `path` | Coral (bold) |

**Path Display:**
- `~` when in shell's home directory
- `~/subdir` when in subdirectory of home
- Full path for directories outside home

---

## 📁 Project Structure

```
MiniProject-1/
├── main.c              # Entry point, signal handlers, main loop
├── commands.c          # Built-in commands implementation
├── commands.h          # Command function declarations
├── utils.c             # Utility functions, command execution
├── utils.h             # Utility function declarations
├── globals.c           # Global variables, helper functions
├── globals.h           # Global declarations, structs, macros
├── prompt.c            # Prompt display logic
├── prompt.h            # Prompt function declarations
├── alias.c             # Alias and function handling
├── alias.h             # Alias declarations
├── shellux.myshrc      # Configuration file (aliases/functions)
├── Makefile            # Build configuration
├── test_shell.sh       # Testing script
├── README.md           # This documentation
├── bin/                # Compiled binary
│   └── shellux
└── obj/                # Object files
    └── *.o
```

---

## 🔧 Technical Details

### Memory Management
- Dynamic allocation for background process tracking
- Proper cleanup on shell exit
- Memory is freed for command history and process lists

### Process Management
- Foreground processes are waited on with `waitpid`
- Background processes use non-blocking `waitpid` with WNOHANG
- SIGCHLD handler cleans up terminated background processes

### Signal Safety
- Critical sections use signal blocking
- Foreground PID updates are protected
- Async-signal-safe functions used in handlers

### File Descriptors
- Proper dup2 usage for redirection
- File descriptors closed after duplication
- Multiple pipe support with dynamic fd allocation

---

## 🧪 Testing

Run the comprehensive test suite:

```bash
# Make test script executable
chmod +x test_shell.sh

# Run all tests
./test_shell.sh

# Run quick tests only
./test_shell.sh --quick

# Show help
./test_shell.sh --help
```

### Manual Testing

```bash
# Start the shell
./bin/shellux

# Try these commands:
hop ~
reveal -al
echo "Hello World" > test.txt
cat test.txt
ls | grep test | wc -l
sleep 10 &
activities
proclore
log
exit
```

---

## 🐛 Known Limitations

1. **Platform**: Linux only (uses `/proc` filesystem)
2. **Tab Completion**: Not implemented
3. **Arrow Keys**: History navigation via arrows not supported
4. **Job Control**: Limited to basic fg/bg operations
5. **Quoted Strings**: Limited support for complex quoting
6. **Environment Variables**: `$VAR` expansion not implemented
7. **Wildcards**: Glob patterns (`*`, `?`) not expanded

---

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 👤 Author

**Inesh Dheer**
- GitHub: [@iDheer](https://github.com/iDheer)

---

## 🙏 Acknowledgments

- Operating Systems course concepts
- Linux man pages
- GNU Bash for inspiration
- [man.he.net](https://man.he.net) for online man pages

---

## 📊 Command Reference Quick Card

| Command | Description | Example |
|---------|-------------|---------|
| `hop` | Change directory | `hop ~/Documents` |
| `reveal` | List files | `reveal -al` |
| `seek` | Search files | `seek -f config` |
| `log` | Command history | `log execute 3` |
| `proclore` | Process info | `proclore 1234` |
| `iMan` | Online man | `iMan grep` |
| `neonate` | PID monitor | `neonate -n 2` |
| `ping` | Send signal | `ping 1234 9` |
| `fg` | Foreground | `fg 1234` |
| `bg` | Background | `bg 1234` |
| `activities` | List bg procs | `activities` |
| `exit` | Exit shell | `exit` |

---

<p align="center">
  Made with ❤️ by Inesh Dheer
</p>
