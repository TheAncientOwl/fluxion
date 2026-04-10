# Fluxion Component Detailed Design (CDD)

## Component Architecture Overview

Fluxion is organized into three primary component hierarchies:

1. **Graphite Framework** - Foundation layer providing core application infrastructure
2. **API** - Plugin interface and shared data structures
3. **Application** - Fluxion-specific implementation built on Graphite and API

### Component Dependency Hierarchy

```
Graphite (Framework)
├── Logger: Logging system with worker thread
├── Common: Utilities (TDoubleBuffer, TThreadSafeQueue, UniqueID, TWithFlags)
├── Application: TGraphiteApplication template, TLayer, TSoftCloseableLayer
└── Renderer: Graphics rendering backend (Vulkan/Metal)

API (Plugin Interface)
├── Data: Plugin interface (IFluxionLogsPlugin, PluginBridge)
└── Logs: Common data structures (Highlight, Condition, Filter, LogRow)

Application (Fluxion)
├── Fluxion: Main application orchestrator
├── Data: Central app state (AppState, filter management)
└── Layers: UI layer system
    ├── BaseLayer: Foundation for all layers
    ├── MainMenuLayer: Menu bar rendering
    ├── DevLayer: Debug/logger configuration UI
    ├── FiltersLayer: Filter management and querying
    └── LogsViewLayer: Log table visualization
```

---

## 1. Graphite Framework Components

### 1.1 Logger Component

**Location:** `/src/Graphite/Logger/`

**Functionality:**

- Centralized logging system with asynchronous logging via worker thread
- Hierarchical log scoping with RAII wrapper (ScopeLogger)
- Log level filtering (Trace, Debug, Info, Warning, Error, Critical)
- Thread-safe log message queueing
- Formatted output with ANSI color codes

**Key Structures:**

- `Logger`: Singleton manager with worker thread
- `LogMessage`: Contains message, level, scope stack, timestamp
- `LogScopeFlags`: Bit flags for scope properties (uses TWithFlags)
- `ScopeLogger`: RAII wrapper for scope entry/exit

**Interface:**

```
Logger::GetInstance() -> Logger&
Logger::Log(level, message)
Logger::EnterScope(name)
Logger::ExitScope()
ScopeLogger(scope_name) // Auto enters/exits scope
```

**Dependencies:**

- None (Foundation component)

**Used By:**

- Everything (centralized logging)

---

### 1.2 Common Component

**Location:** `/src/Graphite/Common/`

**Functionality:**

- Thread-safe double buffer for lock-free producer-consumer patterns
- Thread-safe queue for action dispatching
- Dynamic array with automatic growth
- Unique ID generation with monotonic increasing values
- Compile-time flag trait system (TWithFlags)
- Lightweight reference counting (Ref, RefCounted)
- Dynamic library loading (DynamicLibrary)

**Key Structures:**

- `TDoubleBuffer<T>`: Front/back buffers with read/write locks
  - `EDoubleBufferSyncPolicy`: Defines read/write synchronization
- `TThreadSafeQueue<T>`: Lock-free MPMC queue
- `DynArray<T>`: Dynamic storage with capacity management
- `UniqueID`: Thread-safe monotonic ID generator
- `TWithFlags<Derived, Enum>`: Trait for bit-packed enums
- `Ref<T>` / `RefCounted<T>`: Reference counting
- `DynamicLibrary`: Load .so/.dylib/.dll at runtime

**Interface:**

```
TDoubleBuffer<T>::Read() -> const T&
TDoubleBuffer<T>::Write() -> T&
TThreadSafeQueue<T>::Push(item)
TThreadSafeQueue<T>::TryPop() -> optional<T>
TWithFlags<T, Enum>::SetFlag(enum_value)
TWithFlags<T, Enum>::HasFlag(enum_value) -> bool
Ref<T>::Get() -> T*
UniqueID::Generate() -> uint64_t
```

**Dependencies:**

- Logger (for debug output)

**Used By:**

- All components (foundational utilities)

---

### 1.3 Application Component

**Location:** `/src/Graphite/Application/`

**Functionality:**

- Template application framework orchestrating initialization, update loop, and cleanup
- Layer management system for composable UI/game logic
- Soft-closeable layers with visibility toggle
- Action dispatcher for layer communication
- Window configuration and lifecycle management
- Renderer interface abstraction

**Key Structures:**

- `TGraphiteApplication<AppState>`: Main app orchestrator
  - Manages layer stack
  - Runs update loop with OnIterate/OnRender callbacks
  - Handles window events
- `TLayer<AppState, ActionEnum>`: Base layer template
  - Lifecycle hooks: OnAdd, OnIterate, OnRender, OnRemove
  - Access to global AppState
- `TSoftCloseableLayer<AppState, ActionEnum>`: Layer with toggle
  - `is_active` boolean flag
  - `IsActive()`, `SetIsActive()`, `GetDisplayName()`
- `TDispatcher<Layer, ActionEnum, PayloadType>`: Action queue
  - Queue actions with payloads
  - Process in OnIterate
- `WindowConfiguration`: Display settings (resolution, vsync, etc.)
- `IRenderer`: Abstract renderer interface
- `IRenderable`: Interface for renderable objects

**Interface:**

```
TGraphiteApplication<State>::Run()
TGraphiteApplication<State>::AddLayer<LayerType>(args...)
TGraphiteApplication<State>::GetAppState() -> State&
TLayer<State, Action>::OnAdd() -> void
TLayer<State, Action>::OnIterate() -> void
TLayer<State, Action>::OnRender() -> void
TLayer<State, Action>::OnRemove() -> void
TDispatcher<Layer, Action, Payload>::Dispatch(action, payload)
TSoftCloseableLayer<State, Action>::SetIsActive(bool)
```

**Dependencies:**

- Logger (logging)
- Common (Ref, DynArray, UniqueID)
- Renderer (IRenderer interface)

**Used By:**

- Fluxion (Application inherits from TGraphiteApplication)

---

### 1.4 Renderer Component

**Location:** `/src/Graphite/Application/Renderer/`

**Functionality:**

- Graphics API abstraction with Vulkan and Metal implementations
- Command buffer management
- Swap chain handling
- Render pass coordination with ImGui
- Device and queue management

**Key Structures:**

- `VulkanRenderer`: Vulkan backend implementation
  - VkInstance, VkPhysicalDevice, VkDevice, VkQueue
  - VkSwapchain, VkRenderPass, VkFramebuffer
  - Command buffer management
- `MetalRenderer`: Metal backend implementation (macOS)
  - MTLDevice, MTLQueue, MTLCommandQueue
  - CAMetalLayer for window integration

**Interface:**

```
Renderer::Begin() -> void
Renderer::End() -> void
Renderer::Present() -> void
Renderer::GetSwapchainExtent() -> Extent2D
```

**Dependencies:**

- Logger (logging)
- Common (UniqueID)

**Used By:**

- TGraphiteApplication (rendering backend)

---

## 2. API Components

### 2.1 Data Component

**Location:** `/src/API/include/Fluxion/API/Data/`

**Functionality:**

- Defines common data structures shared between Fluxion and plugins
- Logging-related types (color, highlighting, filtering)
- Filter composition structures
- Log row and column metadata

**Key Structures:**

- `Highlight`: Color pair (foreground/background)
- `Condition`: Single filter condition
  - `EConditionFlag`: IsRegex, IsEquals, IsCaseSensitive (TWithFlags)
  - `target`, `pattern`, `regex_flags`
- `Filter`: Collection of conditions
  - `EFilterFlag`: IsActive, IsHighlightOnly, IsCollapsed (TWithFlags)
  - `conditions`, `highlight`
- `Range`: Start/end pairs for log row indices
- `ColumnDetails`: Column definition (name, width, alignment)
- `LogRow`: Single log entry
  - `metadata`, `message`, `colored_message`
- `LogRowMetadata`: Log attributes
  - `level`, `timestamp`, `scope`, `thread_id`

**Interface:**

```
Highlight: color_fg, color_bg
Condition: + flag handling (SetFlag, HasFlag, ClearFlag via TWithFlags)
Filter: + flag handling + conditions management
LogRow: Contains all log attributes
AppState: Centralized state management
```

**Dependencies:**

- Graphite::Common (TWithFlags, RefCounted)

**Used By:**

- API::LogsPlugin (implementing interface)
- Application::Data (using data structures)
- Application::Layers (reading/filtering logs)

---

### 2.2 LogsPlugin Component

**Location:** `/src/API/include/Fluxion/API/LogsPlugin/`

**Functionality:**

- Plugin interface definition for log providers
- Dynamic plugin loading bridge
- Two-way communication between Fluxion and plugins

**Key Structures:**

- `IFluxionLogsPlugin`: Abstract interface
  - `OnEnable(context)`: Initialize plugin
  - `OnDisable()`: Cleanup
  - `GetLogsByIndices(ranges)`: Fetch logs for visible rows
  - `SupportsFilter(filter_type)`: Feature detection
- `PluginBridge`: Factory and instance management

**Interface:**

```
IFluxionLogsPlugin::OnEnable(OnEnableData) -> void
IFluxionLogsPlugin::OnDisable(OnDisableData) -> void
IFluxionLogsPlugin::GetLogsByIndices(vector<Range>) -> vector<LogRow>
IFluxionLogsPlugin::SupportsFilter(EFilterType) -> bool
PluginBridge::LoadPlugin(path) -> unique_ptr<IFluxionLogsPlugin>
```

**Dependencies:**

- API::Data (uses data structures)

**Used By:**

- Application::Data (loading plugins)
- Application::Layers::LogsViewLayer (getting logs)

---

## 3. Application Components

### 3.1 Fluxion Component

**Location:** `/src/Application/Fluxion.hpp/cpp`

**Functionality:**

- Main application class orchestrating the entire Fluxion system
- Extends TGraphiteApplication<AppState> from Graphite framework
- Initializes all layers and dependencies
- Manages application lifecycle

**Key Structures:**

- `Fluxion(WindowConfiguration)`: Constructor
  - Loads plugin via PluginBridge
  - Creates AppState with plugin
  - Creates and registers all layers
  - Sets up event handlers
- Application layering:
  - MainMenuLayer
  - FiltersLayer
  - LogsViewLayer
  - DevLayer
  - BaseLayer

**Interface:**

```
Fluxion::Fluxion(window_config)
Fluxion::Run() // Inherited from TGraphiteApplication
```

**Dependencies:**

- Graphite::Application (TGraphiteApplication, TLayer)
- Graphite::Logger
- API::LogsPlugin (IFluxionLogsPlugin, PluginBridge)
- Application::Data (AppState)
- Application::Layers (all layers)

**Used By:**

- main.cpp (entry point)

---

### 3.2 Data Component

**Location:** `/src/Application/Data/`

**Functionality:**

- Central application state (AppState) holding all mutable data
- Filter and tab management system
- Plugin instance ownership
- Visibility and search state tracking

**Key Structures:**

- `AppState`:
  - `logs_plugin`: Unique pointer to loaded plugin
  - `filters_tabs`: Vector of Tab definitions
  - `visible_chunk`: Current visible logs (VisibleLogsChunk)
  - `searched_log`: Search results state (SearchedLog)
  - `layers_active`: Boolean flags for layer visibility
  - `shared_filter_metadata`: FiltersGeneralMetadata
- `Tab`: Filter tab container
  - `ETabFlag`: IsActive (TWithFlags)
  - `filters`: Vector of Filter objects
- `FiltersGeneralMetadata`: Filter state flags
  - `EFiltersMetadataFlag`: Applied, SavedToDisk (TWithFlags)
- `VisibleLogsChunk`: Range+log data for rendered rows
- `SearchedLog`: Current filtered log indices
- `LayersActive`: Boolean visibility state for DevLayer, FiltersLayer, LogsViewLayer

**Interface:**

```
AppState: Public mutable state accessed by layers
Tab: + flag handling via TWithFlags
Filter: + flag handling via TWithFlags
FiltersGeneralMetadata: + flag handling via TWithFlags
```

**Dependencies:**

- Graphite::Common (TDoubleBuffer, TWithFlags, Ref, unique_ptr)
- Graphite::Logger
- API::Data (Condition, Filter, LogRow, etc.)
- API::LogsPlugin (IFluxionLogsPlugin)

**Used By:**

- Application::Fluxion (created at initialization)
- Application::Layers::\* (reading/modifying state in OnIterate/OnRender)

---

### 3.3 Layers Component

**Location:** `/src/Application/Layers/`

**Functionality:**

- UI layer system decomposing application concerns
- Each layer handles specific UI and logic responsibilities
- Structured as Graphite TLayer<AppState, EFluxionAction> subclasses

#### 3.3.1 BaseLayer

**Functionality:**

- Foundation UI layer for all other layers
- Provides basic layer infrastructure
- Sets up ImGui rendering context

**Key Structures:**

- `BaseLayer`: Inherits from `TLayer<AppState, EFluxionAction>`

**Interface:**

```
BaseLayer::OnAdd() -> void
BaseLayer::OnIterate() -> void
BaseLayer::OnRender() -> void
BaseLayer::OnRemove() -> void
```

**Dependencies:**

- Graphite::Application (TLayer)
- Application::Data (AppState)

**Used By:**

- Fluxion (registered as layer)

---

#### 3.3.2 MainMenuLayer

**Functionality:**

- Renders application menu bar
- Provides navigation and global controls

**Key Structures:**

- `MainMenuLayer`: Inherits from `TLayer<AppState, EFluxionAction>`

**Interface:**

```
MainMenuLayer::RenderMenu() -> void
MainMenuLayer::OnRender() -> void
```

**Dependencies:**

- Graphite::Application (TLayer)
- Application::Data (AppState)
- ImGui

**Used By:**

- Fluxion (registered as layer)

---

#### 3.3.3 DevLayer

**Functionality:**

- Debug and configuration panel
- Logger display and level control
- Toggleable visibility (TSoftCloseableLayer)

**Key Structures:**

- `DevLayer`: Inherits from `TSoftCloseableLayer<AppState, EFluxionAction>`
  - `is_active`: Visibility flag
- **Submodules:**
  - `Logger`: Renders logger configuration and output
  - `Theme`: Theme management and customization

**Interface:**

```
DevLayer::IsActive() -> bool
DevLayer::SetIsActive(bool) -> void
DevLayer::GetDisplayName() -> string_view
DevLayer::RenderLogger() -> void
DevLayer::OnRender() -> void
```

**Dependencies:**

- Graphite::Application (TSoftCloseableLayer)
- Graphite::Logger
- Application::Data (AppState)
- ImGui

**Used By:**

- Fluxion (registered as layer)

---

#### 3.3.4 FiltersLayer

**Functionality:**

- Filter management UI (add/remove/duplicate tabs and filters)
- Condition management for each filter
- Filter application and search execution
- Drag & drop reordering of filters and conditions
- Persistent storage: save/load filters to/from disk
- Implements action dispatcher for UI interactions

**Key Structures:**

- `FiltersLayer`: Inherits from `TSoftCloseableLayer<AppState, EFluxionAction>` + implements `TDispatcher<FiltersLayer, FilterAction, FiltersLayerActionPayload>`
  - `is_active`: Visibility flag
- **Actions** (17 total):
  - Tab operations: AddTab, RemoveTab, DuplicateTab
  - Filter operations: AddFilter, RemoveFilter, DuplicateFilter, MoveFilter
  - Condition operations: AddCondition, RemoveCondition, MoveCondition
  - Filter control: ApplyFilters, DisableFilters
  - Search: NextLog, PrevLog
  - Persistence: SaveFilters, LoadFilters
- **Payloads:**
  - `FiltersDataModify`: Optional IDs (tab_id, filter_id, condition_id)
  - `SearchLog`: Target filter_id for searching
  - `MoveFilter`: Source tab_id, filter_id, and target_filter_id for swapping
  - `MoveCondition`: Source tab_id, filter_id, condition_id, and target_condition_id for swapping

**Interface:**

```
FiltersLayer::RenderToolbar() -> void
FiltersLayer::RenderTabs() -> void
FiltersLayer::RenderTab(tab) -> void
FiltersLayer::RenderFilter(tab_id, filter) -> void    // Drag & drop source/target
FiltersLayer::RenderCondition(tab_id, filter_id, condition) -> void  // Drag & drop source/target
FiltersLayer::MarkFiltersMetadataDirty() -> void
FiltersLayer::OnIterate() -> void // Process action dispatcher
FiltersLayer::OnRender() -> void
FilterAction::HandleFiltersLayerAction(state, action) -> void
SaveFiltersToFile(state) -> void  // Serializes to data/filters/filters.txt
LoadFiltersFromFile(state) -> void  // Deserializes from disk
```

**Dependencies:**

- Graphite::Application (TSoftCloseableLayer, TDispatcher)
- Graphite::Common (TDispatcher, UniqueID)
- Application::Data (AppState, Tab, Filter, Condition)
- API::Data (Filter, Condition structures)
- ImGui (including drag & drop API)
- C++ Standard Library (filesystem, fstream for persistence)

**Used By:**

- Fluxion (registered as layer)

**Action Flow:**

```
User UI (RenderTab) → Dispatcher.Dispatch(action, payload)
→ OnIterate processes queue
→ HandleFiltersLayerAction modifies AppState
→ MarkFiltersMetadataDirty() flags state
→ Next iteration updates UI
```

**Drag & Drop Workflow:**

```
User drags gripper on filter/condition
→ ImGui::BeginDragDropSource() captures drag
→ Payload contains source ID
→ User hovers over target filter/condition
→ ImGui::BeginDragDropTarget() accepts drop
→ ImGui::AcceptDragDropPayload() processes
→ Dispatcher.Dispatch(MoveFilter/MoveCondition)
→ Handler swaps positions via std::iter_swap
→ MarkFiltersMetadataDirty() flags state change
```

**Persistence Workflow:**

```
Application startup
→ Fluxion::AppInit()
→ LoadFiltersFromFile(AppState)
→ If file exists: Deserialize all tabs/filters/conditions
→ If file missing: Use default filters
→ Both cases mark SavedToDisk flag

User modifies filters
→ MarkFiltersMetadataDirty() clears SavedToDisk flag

User triggers SaveFilters action
→ SaveFiltersToFile(AppState)
→ Serializes complete filter tree to data/filters/filters.txt
→ Marks SavedToDisk flag true
```

**File Format:**

- Location: `~/fluxion/filters.flx`
- Format: Plain text with format version (FILTERS_FILE_V1)
- Structure: Header → Tab count → [Tab name, flags, filters...] → [Filter details, conditions...]
- Preserves: All filter settings, colors, condition flags, priorities

---

#### 3.3.5 LogsViewLayer

**Functionality:**

- Log table visualization with lazy-loading
- Range-based log fetching from plugin
- Pagination support via scrolling
- Highlights and filters applied to display
- Implements action dispatcher for visible range updates

**Key Structures:**

- `LogsViewLayer`: Inherits from `TSoftCloseableLayer<AppState, EFluxionAction>` + implements `TDispatcher<LogsViewLayer, LogsViewLayerAction, LogsViewLayerActionPayload>`
  - `is_active`: Visibility flag
- **Actions** (2 total):
  - `UpdateVisibleLogs`: Push new visible ranges to display
  - `None`: No-op
- **Payloads:**
  - `LogsViewLayerActionPayload`: vector of Range indices for visible logs

**Interface:**

```
LogsViewLayer::RenderLogsTable() -> void
LogsViewLayer::OnIterate() -> void // Process action dispatcher
LogsViewLayer::OnRender() -> void
LogsViewLayerAction::HandleLogsViewLayersLayerAction(state, action) -> void
```

**Dependencies:**

- Graphite::Application (TSoftCloseableLayer, TDispatcher)
- Graphite::Common (TDispatcher)
- Application::Data (AppState, VisibleLogsChunk)
- API::Data (LogRow, Highlight, Filter)
- API::LogsPlugin (IFluxionLogsPlugin)
- ImGui

**Used By:**

- Fluxion (registered as layer)

**Lazy-Loading Workflow:**

```
User scrolls table
→ Visible range changes
→ Dispatcher.Dispatch(UpdateVisibleLogs, {new_ranges})
→ OnIterate calls plugin.GetLogsByIndices(ranges)
→ Plugin returns LogRow data
→ AppState.visible_chunk updated
→ OnRender draws updated table with colors/highlights
```

---

## Component Interaction Patterns

### Pattern 1: Plugin Architecture

```
Fluxion
  ├─> PluginBridge::LoadPlugin(path)
  └─> Creates AppState with IFluxionLogsPlugin instance
      └─> LogsViewLayer uses plugin for GetLogsByIndices
```

### Pattern 2: Dispatcher-Based Action Handling

```
UI Layer (Render methods)
  └─> User interaction (ImGui button click)
    └─> Dispatcher.Dispatch(action, payload)
      └─> OnIterate processes queue
        └─> HandleAction modifies AppState
          └─> Next frame OnRender shows updated state
```

### Pattern 3: State-Driven Rendering

```
AppState (central mutable state)
  ├─> FiltersLayer::OnRender reads filters_tabs
  ├─> FiltersLayer::OnRender reads shared_filter_metadata
  ├─> LogsViewLayer::OnRender reads visible_chunk
  ├─> LogsViewLayer::OnRender reads layers_active
  └─> DevLayer::OnRender reads is_active + logger state
```

### Pattern 4: Lazy-Loading with Ranges

```
LogsViewLayer detects scroll
  └─> Calculates new visible range
    └─> Dispatches UpdateVisibleLogs action
      └─> OnIterate requests logs from plugin
        └─> Plugin returns only needed LogRows
          └─> Updates AppState.visible_chunk
            └─> OnRender displays with applied highlights/filters
```

### Pattern 5: Double-Buffered Concurrent Access

```
TDoubleBuffer<AppState>
  ├─> UI threads (layers) read from front buffer
  ├─> Data modification writes to back buffer
  └─> Synchronized swap on frame boundary
      └─> Ensures consistency across layers
```

---

## Dependency Summary Matrix

| Component                 | Depends On                       | Used By                          |
| ------------------------- | -------------------------------- | -------------------------------- |
| **Graphite::Logger**      | None                             | Everything                       |
| **Graphite::Common**      | Logger                           | Everything                       |
| **Graphite::Application** | Logger, Common, Renderer         | Fluxion, Layers                  |
| **Graphite::Renderer**    | Logger, Common                   | TGraphiteApplication             |
| **API::Data**             | Common                           | Application::Data, Layers        |
| **API::LogsPlugin**       | API::Data                        | Application::Data, LogsViewLayer |
| **Application::Fluxion**  | All Graphite, API, Data, Layers  | main.cpp                         |
| **Application::Data**     | Graphite, API                    | Fluxion, All Layers              |
| **Application::Layers**   | Graphite::Application, Data, API | Fluxion                          |

---

## Component Responsibilities Summary

| Component             | Primary Responsibility                       |
| --------------------- | -------------------------------------------- |
| **Logger**            | Asynchronous logging with scopes             |
| **Common**            | Threading, memory, ID generation utilities   |
| **Application**       | Framework (app lifecycle, layers, rendering) |
| **Renderer**          | Graphics API abstraction                     |
| **API::Data**         | Shared data definitions                      |
| **API::LogsPlugin**   | Plugin interface contract                    |
| **Fluxion**           | Application orchestration                    |
| **Application::Data** | Central state management                     |
| **BaseLayer**         | Layer foundation                             |
| **MainMenuLayer**     | Menu bar rendering                           |
| **DevLayer**          | Debug UI                                     |
| **FiltersLayer**      | Filter management UI                         |
| **LogsViewLayer**     | Log visualization and lazy-loading           |
