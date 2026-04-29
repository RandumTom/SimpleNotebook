# 📚 SimpleNotebook

A simple and clean desktop note-taking application for students, built with C++ and Qt6.

![SimpleNotebook](screenshot.png)

## Features

- 🎓 **Pre-made Subject Folders**: Quick access to common school subjects
  - German (Deutsch)
  - Mathematics (Mathe)
  - English (Englisch)
  - Biology (Biologie)
  - Chemistry (Chemie)
  - Physics (Physik)
  - History (Geschichte)
  - Geography (Geografie)
  - Music (Musik)
  - Art (Kunst)
  - Physical Education (Sport)
  - Computer Science (Informatik)

- 📁 **Custom Folder Support**: Choose your own folder location
- ✏️ **Simple Text Editor**: Clean, distraction-free writing
- 📊 **Status Bar**: Shows filename and character count
- 💾 **Auto-save Support**: Never lose your notes

## Screenshots

### Start Page
Click on any subject to instantly create/open notes for that class.

### Editor View
Clean interface with sidebar showing all your notes in the selected folder.

## Installation

### Prerequisites
- Qt6 or Qt6 Widgets
- CMake 3.16+
- C++17 compatible compiler

### Build from Source

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/SimpleNotebook.git
cd SimpleNotebook

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j4

# Run
./SimpleNotebook
```

## Usage

1. **Launch the app** - You'll see the start page with subject buttons
2. **Select a Subject** - Click on any subject (e.g., "German", "Mathematics") to open that folder
3. **Create/Edit Notes** - Use "+ New Note" to create a new file, or double-click existing files
4. **Go Back** - Click "← Back" to return to the subject selection

Notes are automatically saved to `~/Documents/SchoolNotes/[Subject]/`

## Project Structure

```
SimpleNotebook/
├── main.cpp           # Entry point
├── mainwindow.h/cpp  # Main window with stacked widget
├── startpage.h/cpp   # Subject selection start page
├── editorview.h/cpp  # Note editor with sidebar
├── CMakeLists.txt    # Build configuration
└── SPEC.md           # Detailed specification
```

## License

MIT License - Feel free to use and modify!
