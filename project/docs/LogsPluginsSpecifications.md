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

- Improvement over `Plugins/Logs/Text/RegexTags/V2`
- Regex engine: google RE2
- Storage: SQLite3
- Faster data imports: use mmap for input file + parallelism for count lines over mmap
