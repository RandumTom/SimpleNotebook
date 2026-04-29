# SimpleNotebook - Specification

## 1. Project Overview
- **Project name**: SimpleNotebook
- **Type**: Desktop application (Qt6)
- **Core functionality**: A dark-mode note-taking app with math symbol support for students
- **Target users**: Students needing organized notes with math/scientific notation

## 2. UI/UX Specification

### Visual Design - Dark Mode

#### Color Palette
| Element | Color | Hex |
|---------|-------|-----|
| Background | Dark charcoal | #1E1E1E |
| Sidebar | Slightly lighter | #252526 |
| Text (primary) | Light gray | #D4D4D4 |
| Accent/Primary | Purple | #7C3AED |
| Success (New) | Green | #10B981 |

### Layout Structure
- **Start Page**: Subject selection grid with custom folder option
- **Editor View**: Sidebar with file list + main editor area

## 3. Features

### Editor Features
| Feature | Shortcut | Description |
|---------|----------|-------------|
| New note | Ctrl+N | Create new file |
| Save | Ctrl+S | Save current file |
| Search | Ctrl+F | Find in document |
| Delete line | Ctrl+Backspace, Ctrl+Delete, Alt+Backspace, Alt+Delete | Delete entire current line |
| Auto-save | - | Saves every 30 seconds |

### Math Symbol Converter (Space-triggered)
Type LaTeX-style input and press **space** to convert:

#### Greek Letters
| Input | Output |
|-------|--------|
| `alpha` | α |
| `beta` | β |
| `gamma` | γ |
| `delta` | δ |
| `theta` | θ |
| `pi` | π |
| `sigma` | σ |
| `phi` | φ |
| `omega` | ω |
| `Omega` | Ω |

#### Superscripts
| Input | Output |
|-------|--------|
| `^2` | ² |
| `^3` | ³ |
| `^n` | ⁿ |
| `^+` | ⁺ |
| `^-` | ⁻ |

#### Subscripts
| Input | Output |
|-------|--------|
| `_1` | ₁ |
| `_x` | ₓ |
| `_n` | ₙ |
| `_-` | ₋ |

#### Roots
| Input | Output |
|-------|--------|
| `root(16)` | √16 (square root) |
| `root(3,64)` | ³√64 (cube root) |
| `root(9,256)` | ⁹√256 (9th root) |

#### Math Operators
| Input | Output |
|-------|--------|
| `sqrt` | √ |
| `sum` | Σ |
| `prod` | Π |
| `integral` | ∫ |
| `nabla` | ∇ |

#### Relations
| Input | Output |
|-------|--------|
| `<=` | ≤ |
| `>=` | ≥ |
| `!=` | ≠ |
| `approx` | ≈ |
| `pm` | ± |
| `times` | × |
| `div` | ÷ |
| `cdot` | · |

#### Sets & Logic
| Input | Output |
|-------|--------|
| `in` | ∈ |
| `!in` | ∉ |
| `subset` | ⊂ |
| `subseteq` | ⊆ |
| `union` | ∪ |
| `intersect` | ∩ |
| `empty` | ∅ |
| `N` | ℕ |
| `Z` | ℤ |
| `R` | ℝ |
| `and` | ∧ |
| `or` | ∨ |
| `not` | ¬ |
| `implies` | ⇒ |
| `iff` | ⇔ |
| `forall` | ∀ |
| `exists` | ∃ |

#### Arrows
| Input | Output |
|-------|--------|
| `->` | → |
| `<-` | ← |
| `to` | → |
| `gets` | ← |

#### Fractions
| Input | Output |
|-------|--------|
| `1/2` | ½ |
| `1/3` | ⅓ |
| `2/3` | ⅔ |
| `1/4` | ¼ |
| `3/4` | ¾ |

#### Other
| Input | Output |
|-------|--------|
| `infty` | ∞ |
| `degree` | ° |
| `partial` | ∂ |
| `perp` | ⊥ |

### School Subjects
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

## 4. Status Bar
- Filename (with ● unsaved indicator)
- Word count
- Character count
- Line and column number

## 5. File Location
Notes saved to: `~/Documents/SchoolNotes/[Subject]/[Note].txt`
