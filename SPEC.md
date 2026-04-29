# Simple Notebook App - Specification

## 1. Project Overview
- **Project name**: SimpleNotebook
- **Type**: Desktop application (Qt6)
- **Core functionality**: A minimal text editor for writing and saving notes with folder-based organization
- **Target users**: Anyone needing a simple text editor organized by folders

## 2. UI/UX Specification

### Layout Structure
- **Start Page** (initial view):
  - Centered welcome message
  - Grid of school subject buttons (German, Mathematics, English, Biology, Chemistry, Physics, History, Geography, Music, Art, Physical Education, Computer Science)
  - "Choose Custom Folder" button for custom locations
  - Notes are saved to ~/Documents/SchoolNotes/[Subject]/
  
- **Editor View** (after selecting folder):
  - Sidebar (left): File list from selected folder
  - Main area (right): Text editor
  - Toolbar at top
  - Status bar at bottom

### Visual Design
- **Color palette**:
  - Background: System default (native look)
  - Sidebar background: #F5F5F5 (light gray)
  - Toolbar background: System default toolbar color
  - Status bar: System default status bar color
  - Selected item: System highlight color
- **Typography**: System default monospace or sans-serif, 12pt for text area
- **Window**: 900x650 default size, resizable

### Components

#### Start Page
1. **Welcome label**: "Welcome to SimpleNotebook"
2. **Select Folder button**: Large button to open folder dialog
3. **Recent Folders list**: Shows last 5 used folders (optional for MVP)

#### Sidebar (Editor View)
1. **Back button**: Return to start page
2. **Current folder label**: Shows folder path
3. **File list**: Scrollable list of .txt files in folder
4. **Add button**: Create new note in folder

#### Toolbar
- New button ("New")
- Save button ("Save")

#### Text Area
- Multi-line text edit (QPlainTextEdit)
- Scrollable
- Monospace font

#### Status Bar
- Left: Current file name
- Right: Character count

### Behaviors
- **Start Page → Select Folder**: Opens native folder dialog, transitions to Editor View
- **Back**: Returns to Start Page
- **New**: Creates new file in current folder
- **Save**: Saves current file
- **Click file in sidebar**: Loads that file into editor

## 3. Functional Specification

### Core Features
1. Start page with folder selection
2. List .txt files from selected folder in sidebar
3. Create new notes in selected folder
4. Open existing .txt files from folder
5. Save notes to folder
6. Return to start page to select different folder

### Classes
- `MainWindow` (mainwindow.h/cpp): Main application window
  - Manages stacked widget (start page / editor)
  - Handles navigation between views
  
- `StartPage` (startpage.h/cpp): Start page widget
  - Folder selection button
  - Recent folders list (optional)
  
- `EditorView` (editorview.h/cpp): Editor widget
  - Sidebar with file list
  - Text editor
  - Toolbar and status bar

### Data Flow
- Folder selection: QFileDialog::getExistingDirectory
- File operations: QFile, QTextStream
- File list: QDir, QDirIterator for .txt files

### Edge Cases
- Empty folder: Show "No notes yet" message
- File deleted externally: Refresh file list
- Multiple instances editing same file: Simple approach - last save wins

## 4. Acceptance Criteria
- [ ] App launches with start page
- [ ] "Select Folder" opens native folder dialog
- [ ] After selecting folder, shows file list in sidebar
- [ ] Clicking file loads it in editor
- [ ] "New" creates new file in current folder
- [ ] "Save" saves to current folder
- [ ] "Back" returns to start page
- [ ] Status bar shows filename and character count
- [ ] Window is resizable
