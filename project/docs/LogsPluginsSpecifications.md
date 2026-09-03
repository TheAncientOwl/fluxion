# [@] Fluxion LogsPlugins Specifications

## 1. `Plugins/Logs/Text/RegexTags/V1`

- Base plugin to make sure fluxion works.
- Regex engine: std::regex
- Storage: CSV via MIOCSV

## 2. `Plugins/Logs/Text/RegexTags/V2`

- Improvement over `Plugins/Logs/Text/RegexTags/V1`
- Regex engine: google RE2
- Storage: CSV via MIOCSV

## 3. `Plugins/Logs/Text/RegexTags/V3`

- Improvement over `Plugins/Logs/Text/RegexTags/V2`
- Regex engine: google RE2
- Storage: SQLite3

## 4. `Plugins/Logs/Text/RegexTags/V4`

- Improvement over `Plugins/Logs/Text/RegexTags/V3`
- Regex engine: google RE2
- Storage: SQLite3
- Faster data imports: use mmap for input file + parallelism for count lines over mmap

## 5. `Plugins/Logs/Text/RegexTags/V5`

- Improvement over `Plugins/Logs/Text/RegexTags/V4`
- Regex engine: google RE2
- Storage: SQLite3
- No longer count lines to display logs progress, use bytes processed instead
- Faster data imports: use mmap for input file
- Split mmapped file into slices
- Parse the slices on multiple threads using google RE2
- Each thread submits the parsed logs to the sqlite writer

```
[ mmap File Buffer ]
        │
        ▼ (SplitFile ~4MB Slices)
[ File Slices: 0 .. S ] ───> Atomic Work Stealer (fetch_add)
                                   │
         ┌─────────────────────────┼─────────────────────────┐
         ▼                         ▼                         ▼
   Worker 1 (RE2)            Worker 2 (RE2)            Worker K (RE2)
         │                         │                         │
         └─────────────────────────┼─────────────────────────┘
                                   │ (SubmitFilledChunk)
                                   ▼
          [ DynamicChunkQueue (Slice-Ordered Ready Queues) ]
          [ Free Pool: workers * 10 Chunks (5,000 rows ea) ]
                                   │
                                   ▼ (PopNextChunk in strict order)
                        [ Single Writer Thread ]
                                   │
                                   ▼
                            [ SQLite Database ]
```

## 6. `Plugins/Logs/Text/RegexTags/V6`

- Improvement over `Plugins/Logs/Text/RegexTags/V5`
- Regex engine: google RE2
- Storage: SQLite3
- Updates: dropped filtered_logs table, uses internal vector<FilteredLog> instead
- Now split input logs file in reasonable chunks -> apply previous importing architecture for each chunk since `Scrolls` engine supports multi storage files, so we can truly split importing logs on threads (writing included).

```cpp
struct FilteredLog
{
    std::size_t log_id{0};
    Graphite::Common::Utility::UniqueID filter_id{/* <Default-ID> */};
    Graphite::Common::Utility::UniqueID highlight_filter_id{/* <Default-ID */>};
};
```

## 7. `Plugins/Logs/Text/RegexTags/V7`

- Improvement over `Plugins/Logs/Text/RegexTags/V6`
- Regex engine: google RE2
- Storage: Custom `Scrolls` engine
-

# 7.0. 📜 Scrolls ~ storage handler

## 7.1. Overall Architecture

```
+-----------------------------------------------------------------------------------------+
|                                      SCRIBE                                             |
|  - m_papyrus_size: Bytes                                                                |
|  - m_row_offsets:     std::vector<Bytes>  ---> [Row 0 Offset, Row 1 Offset, ...]        |
|  - m_scrolls_offsets: std::vector<Bytes>  ---> [Scroll 0 Start, Scroll 1 Start, ...]    |
+-----------------------------------------------------------------------------------------+
                                            |
                   +------------------------+------------------------+
                   | Owns std::vector<std::unique_ptr<Papyrus>>      |
                   v                                                 v
        +---------------------+                           +---------------------+
        |  Papyrus (Chunk 0)  |        ... ... ...        |  Papyrus (Chunk N)  |
        +---------------------+                           +---------------------+
        | - PapyrusReader     |                           | - PapyrusReader     |
        | - PapyrusWriter     |                           | - PapyrusWriter     |
        | - MappedFile m_file |                           | - MappedFile m_file |
        +----------+----------+                           +----------+----------+
                   |                                                 |
                   v                                                 v
        [ OS Memory Mapping ]                             [ OS Memory Mapping ]
                   |                                                 |
                   v                                                 v
        [( Disk: papyrus_00.papy )]                     [( Disk: papyrus_N.papy )]
```

## 7.2. Writing Architecture

```
[ Client ]
    |
    |-- 1. Open(home_path, papyrus_size)
    v
[ Scribe ] ---> Pre-allocates N papyrus files on disk & memory-maps them
    |           (PROT_READ | PROT_WRITE / MAP_SHARED)
    |
    |-- 2. GetWriters() -> std::vector<std::reference_wrapper<PapyrusWriter>>
    v
+---------------------------------------------------------------------------------+
|                         PARALLEL WORKER THREADS                                 |
|                                                                                 |
|   Thread 0               Thread 1                       Thread N                |
|      |                      |                              |                    |
|      v                      v                              v                    |
|   Write(lines)           Write(lines)                   Write(lines)            |
|      |                      |                              |                    |
|      v                      v                              v                    |
|   [ Papyrus 0 ]          [ Papyrus 1 ]                  [ Papyrus N ]           |
|   (memcpy to MappedFile) (memcpy to MappedFile)         (memcpy to MappedFile)  |
+---------------------------------------------------------------------------------+
    |
    |-- 3. Worker threads join
    v
[ Scribe ] ---> Reconciles written sizes and builds global index:
                m_row_offsets & m_scrolls_offsets
```

## 7.3. Sequential Reading Architecture

```
[ SequentialReader ]
         |
         +---> Read lines from [ Papyrus 0 ] (Offset 0 -> EOF)
         |                               |
         | (Reached EOF)                 v
         +---> Read lines from [ Papyrus 1 ] (Offset 0 -> EOF)
         |                               |
         | (Reached EOF)                 v
         +---> Read lines from [ Papyrus N ] (Offset 0 -> EOF)
```

## 7.4. Ranged based reading Architecture

```
[ Client ] ---> RangeReader(Range { begin_id: 1500, end_id: 2000 })
                      |
                      v
                [ Index Lookup ]
                      |
                      +---> Binary Search in m_row_offsets (Target: Row 1500)
                      |
                      v
                [ Scroll & Offset Resolution ]
                      |
                      +---> Resolved: Scroll Index 2, Byte Offset 0x41B0
                      |
                      v
           +-----------------------+
           |  Papyrus 2 MappedFile | ---> Direct Pointer: Base + 0x41B0
           +-----------------------+      (Zero-copy string_view reads)
```

## 8. `Plugins/Logs/Text/RegexTags/V8`

- Comparable performance to `Plugins/Logs/Text/RegexTags/V7`
- Regex engine: google RE2
- Storage: SQLite3
- Same architecture as `V7`, but this time using SQLite3 as storage
- Comparable speed in imports
- `V8` faster filtering than `V7`
- `V7` faster filtering disable than `V8`
