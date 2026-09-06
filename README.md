# ⚡ 🪵 Fluxion

[![License: MIT](https://img.shields.io/badge/License-MIT-purple.svg)](https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE)
[![C++23](https://img.shields.io/badge/C++-23-purple.svg)](https://en.cppreference.com/w/cpp/23)
[![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Windows%20%7C%20Linux-purple.svg)](https://github.com/TheAncientOwl/fluxion)

**Fluxion** is a lightning-fast, high-performance cross-platform log viewer built to effortlessly handle and analyze massive log files without breaking a sweat.

Designed with a modular architecture, Fluxion separates its rendering interface from its heavy lifting logic through a robust plugin API, ensuring fluid navigation and instant search capabilities even across gigabytes of data.

---

## 📑 Table of Contents

- [Goals](#-goals)
- [Architecture](#️-architecture)
- [Features](#-features)
- [Tech Stack](#️-tech-stack)
- [Plugin Interface Overview](#-plugin-interface-overview)
- [Building & Development](#-building--development)
- [License](#-license)
- [Gallery](#-gallery)

---

## 🎯 Goals

Fluxion aims to provide:

- **Extreme Performance:** Instant parsing, smooth scrolling, and zero-lag navigation through massive logs.
- **Modularity:** A clean plugin-based separation between the UI application and log processing logic.
- **Cross-Platform Compatibility:** Native performance on macOS, Windows, and Linux.
- **Precision Analysis:** Advanced regex-based structuring, custom filters, and fast highlight/match jumping.

---

## 🏛️ Architecture

Fluxion is split into two primary layers:

1. **Fluxion App (GUI):** The desktop frontend responsible for rendering the interface, managing view states, and handling user interactions.
2. **Logs Plugins:** Decoupled, high-performance backend libraries responsible for raw log I/O, regex parsing, chunking, filtering, and indexing.

The GUI communicates with the backend entirely through a standardized plugin interface (`IFluxionLogsPlugin`), allowing specialized log formats and parsing pipelines to be integrated seamlessly.

---

## ✨ Features

- **Massive File Handling:** Memory-mapped file I/O paired with 500MB (default) **sqlite3** databases segmentation to process enormous files safely and efficiently.
- **Multithreaded Parsing:** Leverages parallel worker threads and Google's **RE2** regular expression engine to slice and parse log lines concurrently.
- **Structured Table View:** Converts raw text lines into structured, column-aligned data using configurable regular expression captures.
- **Lightning-Fast Navigation:** Instant jump-to-next (`GetNextLog`) and jump-to-prev (`GetPrevLog`) matching across active filters and highlights.
- **Dynamic Chunking:** On-demand range requests (`GetLogs`) for smooth virtualized table rendering.
- **Cross-Platform Desktop App:** Built for native speeds across macOS, Windows, and Linux.

---

## 🛠 Tech Stack

- **C++23** – Core application and high-performance log parsing engine
- **Graphite** – Custom APP framework that uses ImGui (immediate-mode GUI) for rendering
- **Google RE2** – Thread-safe, high-performance regular expression matching
- **Google Test (gtest)** – Comprehensive unit and integration testing framework
- **CMake** – Cross-platform build configuration system

---

## 🧩 Plugin Interface Overview

The core log logic revolves around the `IFluxionLogsPlugin` contract, which handles the lifecycle of log operations:

```cpp
namespace Fluxion::API::LogsPlugin {

class IFluxionLogsPlugin
{
public:
    virtual std::string_view GetDisplayName() const = 0;
    virtual std::string_view GetDirectoryName() const = 0;

    virtual void OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& data) = 0;
    virtual void OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& data) = 0;

    virtual void RenderMenu() = 0;

    virtual void ImportLogs(std::filesystem::path const& path) = 0;

    /**
     * @brief Get the next log index relative to current item.
     *
     * @param filter_id the filter to search within.
     * @param current_index current selected log index (0 if no item selected).
     */
    virtual std::optional<std::size_t> GetNextLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index = 0) = 0;

    /**
     * @brief Get the previous log index relative to current item.
     *
     * @param filter_id the filter to search within.
     * @param current_index current selected log index (0 if no item selected).
     */
    virtual std::optional<std::size_t> GetPrevLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index = 0) = 0;

    /**
     * @brief Apply given filters.
     *
     * @param filters what logs are shown.
     * @param highlight_only override of @param filters for the colors.
     */
    virtual void ApplyFilters(
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> filters,
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> highlight_only) = 0;

    /**
     * @brief This should mark the filters as disabled.
     */
    virtual void DisableFilters() = 0;

    /**
     * @brief Get the Table Header.
     */
    virtual std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> GetTableHeader() const = 0;

    /**
     * @brief Get the Total Filtered Logs.
     */
    virtual std::size_t GetTotalLogs() const = 0;

    /**
     * @brief Request logs.
     *
     * @param ranges list of reequested chunks {begin inclusive, end exclusive}.
     * @param out_logs map<index, row> to be updated.
     *
     */
    virtual void GetLogs(
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs) = 0;

    /**
     * @brief Helper function to tell the total of logs in import file
     * @note Should be thread safe
     */
    virtual std::size_t GetLogsOperationTarget() const = 0;

    /**
     * @brief Helper function to tell the unit in wwhich logs operation progress is measured
     * @note Should be thread safe
     */
    virtual Fluxion::API::LogsPlugin::Data::ELogsOperationUnit GetLogsOperationUnit() const = 0;

    /**
     * @brief Helper function to track how many logs were processed at call time
     * @note Should be thread safe
     */
    virtual std::size_t GetLogsOperationProgress() const = 0;

    virtual ~IFluxionLogsPlugin() = default;
};

} // namespace Fluxion::API::LogsPlugin

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
```

## 🚀 Building & Development

### Prerequisites

- A modern C++23 compatible compiler (**Clang** recommended, GCC, or MSVC)
- **CMake** (version 3.25 or higher)

### Setup & Build

Clone the repository and build using CMake:

```bash
# Clone the project
git clone --recurse-submodules https://github.com/TheAncientOwl/fluxion.git
cd fluxion

# Configure the project
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build the application
cmake --build build --config Release
```

## 📄 License

Distributed under the [MIT License](https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE).
See [LICENSE-THIRD-PARTY.md](https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE-THIRD-PARTY.md) for details on third-party libraries and components.

---

## 🖼️ Gallery

Below are a few screenshots showcasing the Fluxion workflow, from importing large log files to configuring regex columns and navigating filtered results.

### » App UI

<img alt='Logs Import' src="./gallery/0.fluxion-first-view.png" width="600">

### » Logs Import & Regex Columns Configurator

- Split raw log lines into clean, structured columns using regular expressions.

<img alt='Logs Import' src="./gallery/1.raw-logs.png" width="600">

<img alt='Logs Import' src="./gallery/1.1.fluxion-logs-menu.png" width="600">

- Import massive log files with progress tracking.

<img alt='Logs Import' src="./gallery/1.2.fluxion-logs-import-select.png" width="600">
<img alt='Logs Import' src="./gallery/1.3.fluxion-logs-import-progress.png" width="600">
<img alt='Logs Import' src="./gallery/1.4.fluxion-logs-begin.png" width="600">
<img alt='Logs Import' src="./gallery/1.5.fluxion-logs-end.png" width="600">

### » Advanced Filtering, Highlights & Navigation

- Filter logs dynamically and apply priority color highlighting to crucial log levels.

<img alt='Filtered Logs' src="./gallery/2.1.fluxion-filtering.png" width="600">

- Sky's the limit when it comes to highlight colors for foreground/background.

<img alt='Filtered Logs' src="./gallery/2.2.fluxion-filtering-color-picker.png" width="600">

- Explore structured logs with smooth virtualized scrolling and instant find-next/find-prev navigation.

<img alt='Logs View' src="./gallery/2.3.fluxion-filtering-next-1.png" width="600">
<img alt='Logs View' src="./gallery/2.4.fluxion-filtering-next-2.png" width="600">
<img alt='Logs View' src="./gallery/2.5.fluxion-filtering-next-3.png" width="600">
<img alt='Logs View' src="./gallery/2.6.fluxion-filtering-next-4.png" width="600">
<img alt='Logs View' src="./gallery/2.7.fluxion-filtering-next-5.png" width="600">
