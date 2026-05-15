# NoteP

A simple, lightweight text editor for Windows. Designed to be a replacement for Notepad, which has become bloated lately.

## Features

- **Basic Text Editing**: Full-featured text editing with undo/redo support
- **Find & Replace**: Search and replace functionality with forward/backward direction options
- **Line Numbers**: Toggle line number display on/off (hidden by default)
- **Word Wrap**: Toggle word wrapping to fit text within the window
- **Font Selection**: Choose custom fonts and sizes
- **Print Support**: Print documents with custom page setup
- **Window State Persistence**: Remembers window position and font settings between sessions
- **Date/Time Insertion**: Press F5 to insert the current date and time (uses Windows region settings)
- **Status Bar**: Real-time display of cursor position (line and column)
- **Command-line Support**: Open files directly: `NoteP.exe filename.txt`
- **Maximize/Restore**: Dragging a maximized window away from the top automatically restores it to normal size
- **Large File Support**: Optimized performance for files larger than 5MB

## Requirements

- Windows 10 or later
- wxWidgets 3.1.4 (for building)
- CMake 3.10 or later (for building)
- Visual Studio 2022 (for building)

## Building from Source

1. Install wxWidgets 3.1.4 and CMake
2. Clone or download this repository
3. Run `build.bat` in the project directory
4. The compiled executable will be at `build\Release\NoteP.exe`

## Usage

### Basic Operations

- **New File**: Ctrl+N
- **New Window**: Ctrl+Shift+N
- **Open**: Ctrl+O
- **Save**: Ctrl+S
- **Save As**: Ctrl+Shift+S
- **Print**: Ctrl+P
- **Exit**: Alt+F4 or File > Exit

### Edit Operations

- **Undo**: Ctrl+Z
- **Redo**: Ctrl+Y
- **Cut**: Ctrl+X
- **Copy**: Ctrl+C
- **Paste**: Ctrl+V
- **Select All**: Ctrl+A
- **Find**: Ctrl+F
- **Replace**: Ctrl+H

### View Options

- **Line Numbers**: Toggle via View > Line Numbers
- **Word Wrap**: Toggle via Format > Word Wrap
- **Font**: Change via Format > Font...

### Other

- **Insert Date/Time**: Press F5 (uses your Windows locale settings)

## Command-Line Usage

Open a file directly from the command line:

```
NoteP.exe C:\path\to\file.txt
```

## Project Structure

```
Notep/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── app.h/cpp             # wxApp implementation
│   ├── mainframe.h/cpp       # Main window and UI logic
│   ├── findreplace.h/cpp     # Find/Replace dialog
│   ├── config.h/cpp          # Configuration management
│   └── resource.rc           # Windows resource file with icon
├── icon.ico                  # Application icon
├── CMakeLists.txt            # CMake build configuration
├── build.bat                 # Build script
├── create_shortcut.bat       # Creates desktop shortcut
└── README.md                 # This file
```

## Configuration

Settings are saved in your user data directory (typically `%APPDATA%\NoteP\config`):
- Window position and size
- Font selection and size
- Line number and word wrap preferences

## Tips

- **Large Files**: For files larger than 10MB, undo functionality is disabled for performance
- **Drag & Drop**: You can select and copy text, but cannot drag to move it
- **Maximize Shortcut**: Double-click the title bar to maximize/restore, or drag the title bar down to restore a maximized window
- **Keyboard Navigation**: Use keyboard shortcuts for faster editing

## License

Created as a modern text editor alternative for Windows users.
