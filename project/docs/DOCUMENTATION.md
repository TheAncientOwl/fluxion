# Fluxion Project Documentation

## Overview

This documentation provides comprehensive architectural documentation for the Fluxion project, including PlantUML diagrams and detailed component design information.

### Documentation Components

1. **Component Detailed Design (CDD)** - [COMPONENT_DESIGN.md](COMPONENT_DESIGN.md)
   - High-level architecture overview
   - Component responsibilities and functionalities
   - Dependency relationships between components
   - Interaction patterns and workflows
   - Detailed specifications for each component

2. **Component Dependency Diagram** - [COMPONENT_DIAGRAM.puml](COMPONENT_DIAGRAM.puml)
   - Visual representation of component dependencies
   - Shows how all components relate to each other

3. **Module Diagrams** - Per-module PlantUML documentation
   - State diagrams showing data structures
   - Interface diagrams showing classes and methods
   - Code flow diagrams showing execution patterns

### Diagram Types

1. **`state.uml`** - Data Structures & State
   - Shows all major data structures, enums, and their relationships
   - Visualizes the state management patterns
   - Identifies dependencies between components

2. **`interface.uml`** - Class Methods & Interfaces
   - Displays public interfaces, classes, and their methods
   - Shows inheritance relationships
   - Highlights key API points

3. **`code.uml`** - Workflow & Code Flow
   - Illustrates code execution flow and sequence diagrams
   - Shows how components interact at runtime
   - Describes the action and data flow between modules

---

## Module Documentation

### 1. API Layer

📁 `src/API/`

**Purpose**: Plugin system interface and common data structures for the logging plugin.

**Diagrams**:

- `state.uml` - Highlight, Filter, and plugin data structures
- `interface.uml` - IFluxionLogsPlugin interface definition
- `code.uml` - Plugin lifecycle and log processing workflow

**Key Components**:

- `IFluxionLogsPlugin` - Main plugin interface for log handling
- `Highlight` - Color configuration for log highlighting
- Plugin bridge data structures (OnEnableData, OnDisableData)

---

### 2. Application Data Layer

📁 `src/Application/Data/`

**Purpose**: Application state management and data structures for filters, logs, and UI state.

**Diagrams**:

- `state.uml` - AppState, Filter hierarchy, and double-buffer patterns
- `interface.uml` - AppState methods and flag-based operations
- `code.uml` - State mutation and filter application workflow

**Key Components**:

- `AppState` - Central application state container
- `Filters::Filter` / `Condition` / `Tab` - Filter management hierarchy
- `Logs::VisibleLogsChunk` - Visible log data representation
- Flag-based enums (EFilterFlag, EConditionFlag, ETabFlag)

---

### 3. Application Layers

📁 `src/Application/Layers/`

**Purpose**: UI layer system for rendering different application views (menu, filters, logs, debug).

Each layer has its own documentation directory with state, interface, and code flow diagrams.

#### BaseLayer

📁 `src/Application/Layers/docs/BaseLayer/`

**Purpose**: Core base layer for common UI elements.

**Key Methods**:

- `OnAdd()` - Initialize layer
- `OnIterate()` - Update state
- `OnRender()` - Render UI
- `OnRemove()` - Cleanup

---

#### MainMenuLayer

📁 `src/Application/Layers/docs/MainMenuLayer/`

**Purpose**: Main application menu bar rendering.

**Key Methods**:

- `RenderMenu()` - Renders menu items (File, Edit, View, etc.)
- `OnAdd()`, `OnIterate()`, `OnRender()`, `OnRemove()` - Layer lifecycle

---

#### DevLayer

📁 `src/Application/Layers/DevLayer/docs/`

**Purpose**: Debug and development utilities (toggleable).

**Key Features**:

- Extends `TSoftCloseableLayer` - Can be toggled visible/hidden
- `RenderLogger()` - Logger configuration UI (scopes, levels, output)
- `IsActive()` / `SetIsActive()` - Toggle visibility
- `GetDisplayName()` - Display name in UI

**Key Methods**:

- `OnAdd()`, `OnIterate()`, `OnRender()`, `OnRemove()` - Layer lifecycle

---

#### FiltersLayer

📁 `src/Application/Layers/FiltersLayer/docs/`

**Purpose**: Filter creation and management UI with dispatcher-based actions.

**Key Features**:

- Extends `TSoftCloseableLayer` - Can be toggled visible/hidden
- Implements `TDispatcher<FiltersLayer, FilterAction, FiltersLayerActionPayload>` - Action queue
- Renders tabs, filters, and conditions

**Action Types** (EFilterActionType):

- `ApplyFilters` - Apply current filters to logs
- `DisableFilters` - Clear all filters
- Tab Management: `AddTab`, `RemoveTab`, `DuplicateTab`
- Filter Management: `AddFilter`, `RemoveFilter`, `DuplicateFilter`
- Condition Management: `AddCondition`, `RemoveCondition`
- Navigation: `NextLog`, `PrevLog` - Search through filtered logs

**Key Methods**:

- `RenderToolbar()` - Toolbar buttons
- `RenderTabs()` - Tab UI and management
- `RenderFilter()` - Filter editing UI
- `RenderCondition()` - Condition editing UI
- `MarkFiltersMetadataDirty()` - Mark UI state as dirty

---

#### LogsViewLayer

📁 `src/Application/Layers/LogsViewLayer/docs/`

**Purpose**: Logs table display with dispatcher-based action handling.

**Key Features**:

- Extends `TSoftCloseableLayer` - Can be toggled visible/hidden
- Implements `TDispatcher<LogsViewLayer, LogsViewLayerAction, LogsViewLayerActionPayload>` - Action queue
- Lazy-loads log rows based on visible range
- Displays colors from active filters

**Action Types** (ELogsViewActionLayerType):

- `UpdateVisibleLogs` - Update visible log range (with Range indices)

**Key Methods**:

- `RenderLogsTable()` - ImGui table with columns and rows
  - Fetches logs from plugin for visible range
  - Applies filter colors and highlights
  - Handles pagination/scrolling

**Workflow**:

1. User scrolls table → Visible range changes
2. Dispatcher pushed `UpdateVisibleLogs` action
3. Request log rows from plugin for new range
4. Update `AppState::visible_chunk`
5. Re-render with updated data

---

### 4. Graphite Logger

📁 `src/Graphite/Logger/`

**Purpose**: Unified logging system with scope and level management.

**Diagrams**:

- `state.uml` - LogMessage, ELogLevel, Logger singleton structure
- `interface.uml` - Logger public API and configuration methods
- `code.uml` - Logging workflow, worker thread pattern, and scope timing

**Key Components**:

- `Logger` (Singleton) - Thread-safe logging with worker thread
- `LogMessage` - Log entry structure with time and level
- `ScopeLogger` - RAII-based scope entry/exit tracking
- `ELogLevel` - Seven-level logging hierarchy

---

### 5. Graphite Common

📁 `src/Graphite/Common/`

**Purpose**: Reusable utilities and data structures for thread-safe operations.

**Diagrams**:

- `state.uml` - Double buffers, thread-safe queues, and utility types
- `interface.uml` - Methods for synchronization and memory management
- `code.uml` - Producer-consumer patterns, reference counting, and library loading

**Key Components**:

- `TDoubleBuffer<T, Policy>` - Lock-free double buffering with swap, copy, or locking policies
- `TThreadSafeQueue<T>` - Thread-safe queue with condition variables
- `UniqueID` - UUID-based unique identification
- `TWithFlags<T, Enum>` - Bit-packing for flag storage
- `Ref<T>` / `RefCounted` - Reference counting smart pointer
- `DynamicLibrary` - Plugin dynamic loading wrapper

---

### 6. Graphite Application

📁 `src/Graphite/Application/`

**Purpose**: Main application framework and layer system.

**Diagrams**:

- `state.uml` - TGraphiteApplication, TLayer, and window configuration
- `interface.uml` - Application lifecycle methods and layer management
- `code.uml` - Application initialization, render loop, and layer iteration

**Key Components**:

- `TGraphiteApplication<AppState, ActionEnum>` - Template-based main application
- `TLayer<AppState, ActionEnum>` - Base layer class with lifecycle
- `TSoftCloseableLayer` - Layer that can be toggled on/off
- `WindowConfiguration` - Window setup parameters

---

### 7. Graphite Renderer

📁 `src/Graphite/Application/src/Renderer/`

**Purpose**: Graphics rendering abstraction with Vulkan and Metal backends.

**Diagrams**:

- `state.uml` - IRenderer, VulkanRenderer state, and MetalRenderer state
- `interface.uml` - Renderer interface and backend implementations
- `code.uml` - Initialization, render loop, and frame rendering workflow

**Key Components**:

- `IRenderer` - Abstract renderer interface
- `VulkanRenderer` - Vulkan API implementation
- `MetalRenderer` - Metal API implementation (macOS)
- `IRenderable` - Interface for objects that can be rendered

---

## Architecture Notes

### Threading Model

- **Logger**: Background worker thread for async logging
- **Application**: Worker thread for action processing
- **DoubleBuffer**: Lock-free read-write with optional locking variants

### Message Passing

- `TThreadSafeQueue` - Used for application actions and logger messages
- Producer-consumer pattern with condition variables

### Memory Management

- Smart pointers for automatic cleanup
- Reference counting via `Ref<T>` / `RefCounted`
- Shared pointers for plugin instances

### Plugin System

- Dynamic library loading via `DynamicLibrary`
- `IFluxionLogsPlugin` interface contract
- Factory pattern for plugin creation

---

## How to Use These Diagrams

1. **Understanding a Module**: Start with `state.uml` to see data structures
2. **Learning the API**: Read `interface.uml` to understand public methods
3. **Tracing Execution**: Follow `code.uml` to understand workflow and timing

### Viewing Diagrams

PlantUML files can be viewed using:

- VS Code PlantUML extension
- Online PlantUML editors (plantuml.com)
- Command-line PlantUML tools

Example command:

```bash
plantuml src/API/docs/state.uml -o output/
```

---

## Key Design Patterns

### Lock-Free Synchronization

- Double buffering with atomic flags for dirty checking
- Separate read and write buffers minimize contention

### Template-Based Generics

- `TGraphiteApplication<AppState, ActionEnum>` - Customizable application state and actions
- `TLayer<AppState, ActionEnum>` - Type-safe layer implementation
- `TDoubleBuffer<T, Policy>` - Flexible synchronization strategies

### Singleton Pattern

- `Logger::Instance()` - Global logging infrastructure

### Factory Pattern

- `CreateRenderer()` - Platform-specific renderer instantiation
- `CreateFluxionLogsPlugin()` - Plugin creation from dynamic library

### RAII (Resource Acquisition Is Initialization)

- `ScopeLogger` - Automatic timing and scope tracking
- Smart pointers for automatic cleanup

---

## Module Interaction Map

```
┌─────────────────────────────────────────────────────────┐
│         TGraphiteApplication (Main Framework)           │
├─────────────────────────────────────────────────────────┤
│  ├─ TLayer* → Application/Data (State Management)       │
│  ├─ IRenderer* (Graphite/Renderer)                      │
│  └─ TThreadSafeQueue (Action Queue)                     │
├─────────────────────────────────────────────────────────┤
│         Graphite/Common (Utilities)                      │
│  ├─ TDoubleBuffer (Thread-safe buffers)                 │
│  ├─ TThreadSafeQueue (Message passing)                  │
│  ├─ UniqueID (Identification)                           │
│  └─ Ref<T> (Reference counting)                         │
├─────────────────────────────────────────────────────────┤
│         Graphite/Logger (Logging)                        │
│  └─ Uses Graphite/Common utilities                      │
├─────────────────────────────────────────────────────────┤
│   API/Data & Application/Data (Core Models)             │
│  └─ IFluxionLogsPlugin (Plugin Interface)               │
└─────────────────────────────────────────────────────────┘
```

---

## Version Information

- **Documentation**: Created 2026-04-10
- **License**: See [LICENSE](../LICENSE)
