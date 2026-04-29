# SimpleNotebook - Specification

## 1. Project Overview
- **Project name**: SimpleNotebook
- **Type**: Desktop application (Qt6)
- **Core functionality**: A dark-mode note-taking app for students with folder-based organization
- **Target users**: Students needing organized notes for school subjects

## 2. UI/UX Specification

### Visual Design - Dark Mode

#### Color Palette
| Element | Color | Hex |
|---------|-------|-----|
| Background | Dark charcoal | #1E1E1E |
| Sidebar | Slightly lighter | #252526 |
| Text (primary) | Light gray | #D4D4D4 |
| Text (secondary) | Medium gray | #808080 |
| Accent/Primary | Purple | #7C3AED |
| Accent Hover | Lighter purple | #8B5CF6 |
| Success (New) | Green | #10B981 |
| Warning | Orange | #F59E0B |
| Error | Red | #EF4444 |
| Selected item | Purple tint | #3730A3 |
| Border | Subtle dark | #3E3E42 |
| Input background | Darker | #1E1E1E |
| Scrollbar | Medium gray | #424242 |
| Current line highlight | Subtle | #2D2D30 |
| Search highlight | Yellow tint | #4B4B00 |

#### Typography
- **Font**: Consolas / JetBrains Mono / system monospace
- **Editor font size**: 14px
- **UI font size**: 12-13px

### Layout Structure

#### Start Page (Dark Mode)
- Centered welcome with dark background (#1E1E1E)
- Subject buttons in grid (3 columns)
- Hover effects with white/purple accent
- Smooth button animations
- Custom folder option at bottom

#### Editor View (Dark Mode)
- **Sidebar (left, 220px)**:
  - Back button
  - Current folder path
  - File list with hover highlighting
  - "New Note" button at bottom
  
- **Main area (right)**:
  - Toolbar with New, Save, Search buttons
  - Collapsible search bar
  - Text editor with current line highlight
  - Status bar at bottom

### Components

#### Start Page Components
1. Welcome label (large, centered)
2. Subtitle (muted color)
3. Subject button grid (3x4)
4. Custom folder button

#### Editor Components
1. **Toolbar**:
   - New | Save | Search toggle
   
2. **Search Bar** (collapsible):
   - Search icon
   - Text input
   - Match count display
   - Previous/Next buttons
   - Close button

3. **Text Editor**:
   - Current line highlight
   - Scrollable
   - Word wrap enabled
   - Custom scrollbars

4. **Status Bar**:
   - Left: Filename (with ● unsaved indicator)
   - Center: Line and column number
   - Right: Word count | Character count

### Editor Features

#### Core Features
1. **✅ Dark Mode UI** - Full dark theme throughout
2. **✅ Current Line Highlight** - Subtle highlight on active line
3. **✅ Search/Replace** - Ctrl+F to search, highlight all matches, navigate with arrows
4. **✅ Word Count** - Real-time word and character count
5. **✅ Line/Column Indicator** - Shows cursor position
6. **✅ Auto-save** - Auto-save every 30 seconds when changes detected
7. **✅ Keyboard Shortcuts**:
   - Ctrl+N: New note
   - Ctrl+S: Save
   - Ctrl+F: Find/Search

#### School Subjects
| Subject | Icon | Color |
|---------|------|-------|
| German | 🇩🇪 | #7C3AED |
| Mathematics | 📐 | #3B82F6 |
| English | 🇬🇧 | #10B981 |
| Biology | 🧬 | #22C55E |
| Chemistry | ⚗️ | #A855F7 |
| Physics | ⚡ | #6366F1 |
| History | 📜 | #F59E0B |
| Geography | 🌍 | #14B8A6 |
| Music | 🎵 | #EC4899 |
| Art | 🎨 | #F97316 |
| Physical Education | ⚽ | #84CC16 |
| Computer Science | 💻 | #64748B |

## 3. Functional Specification

### Classes
- `MainWindow`: Container with stacked widget
- `StartPage`: Dark-themed subject selection
- `EditorView`: Dark-themed editor with sidebar, search, auto-save

### File Operations
- Save location: `~/Documents/SchoolNotes/[Subject]/[Note].txt`
- Encoding: UTF-8
- Auto-save interval: 30 seconds

## 4. Acceptance Criteria
- [x] Dark mode UI throughout
- [x] Current line highlighted
- [x] Search functionality (Ctrl+F) with match count
- [x] Word/character/line count in status bar
- [x] Auto-save every 30 seconds
- [x] Keyboard shortcuts (Ctrl+N, Ctrl+S, Ctrl+F)
- [x] German subject included
- [x] Push to GitHub
