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
