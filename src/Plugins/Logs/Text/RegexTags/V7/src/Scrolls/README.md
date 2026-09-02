# 📜 Scrolls ~ storage handler

## 1. Overall Architecture

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
        [( Disk: papyrus_00.papy )]                              [( Disk: papyrus_N.papy )]
```

## 2. Writing Architecture

```
[ Client ]
    |
    | 1. Open(home_path, papyrus_size)
    v
[ Scribe ] ---> Pre-allocates N papyrus files on disk & memory-maps them
    |           (PROT_READ | PROT_WRITE / MAP_SHARED)
    |
    | 2. GetWriters() -> std::vector<std::reference_wrapper<PapyrusWriter>>
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
    | 3. Worker threads join
    v
[ Scribe ] ---> Reconciles written sizes and builds global index:
                m_row_offsets & m_scrolls_offsets
```

## 3. Sequential Reading Architecture

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

## 4. Ranged based reading Architecture

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

## 5. Interfaces

```cpp
namespace Scrolls {

using Line = std::vector<std::string_view>;
using Bytes = std::size_t;

class PapyrusReader
{
public:
    virtual ~PapyrusReader() = default;

    class Reader
    {
    public:
        Reader(const char* begin, const char* end);

        /// @return true if there are any lines left, false otherwise
        bool Next(Line& line);

    private:
        const char* begin{nullptr};
        const char* end{nullptr};
    };

    virtual Reader ReadFrom(Bytes const offset) = 0;
};

class PapyrusWriter
{
public:
    virtual ~PapyrusWriter() = default;

    enum EWriteState : std::uint8_t
    {
        Failed,
        Success,
        OutOfSpace
    };

    struct WriteResult
    {
        EWriteState state{EWriteState::Failed};
        Bytes written{0};
    };

    virtual WriteResult Write(std::vector<Line> const& lines) = 0;
    static Bytes CalculateTotalBytes(Line const& line);
};

class Papyrus : public PapyrusReader, public PapyrusWriter
{
public:
    ~Papyrus() override = default;

    void Create(std::filesystem::path const& path, Bytes const size);
    [[nodiscard]] bool IsOpen() const;
    void Close();

    Reader ReadFrom(Bytes const offset) override;
    WriteResult Write(std::vector<Line> const& lines) override;

    Bytes GetTotalBytes();

private:
    MappedFile m_file{};
    Bytes m_file_size{0};
    Bytes m_total_written{0};
};

class Scribe
{
public:
    /// @brief Sequential reader through all scrolls
    class SequentialReader
    {
    public:
        explicit SequentialReader(std::vector<std::unique_ptr<Papyrus>> const& scrolls);

        /// @return true if there are any lines left, false otherwise
        bool Next(Line& line);

    private:
        std::vector<std::unique_ptr<Papyrus>> const& m_scrolls;
    };

    /// @brief Sequential reader through all requested ranges
    class RangeReader
    {
    public:
        struct Range
        {
            std::size_t begin_id{}; // inclusive
            std::size_t end_id{};   // exclusive
        };

        RangeReader(
            std::vector<std::unique_ptr<Papyrus>> const& scrolls,
            std::vector<Bytes> const& row_offsets,
            std::vector<Bytes> const& scrolls_offsets,
            std::vector<Range> ranges);

        /// @return true if there are any lines left, false otherwise
        bool Next(Line& line);

    private:
        std::vector<std::unique_ptr<Papyrus>> const& m_scrolls;
        std::vector<Bytes> const& m_row_offsets;
        std::vector<Bytes> const& m_scrolls_offsets;
        std::vector<Range> m_ranges{};
    };

public:
    bool Open(std::filesystem::path const& home_path, Bytes const papyrus_size);
    bool IsOpen();
    bool Close();

    /// @brief To facilitate writing on multiple threads
    /// @return writers to opened scrolls
    std::vector<std::reference_wrapper<PapyrusWriter>> GetWriters();

    SequentialReader GetSequentialReader() const;
    RangeReader GetRangeReader(std::vector<Range> ranges) const;

private:
    Bytes m_papyrus_size{0};
    std::vector<std::unique_ptr<Papyrus>> m_scrolls{};
    std::vector<Bytes> m_row_offsets{};
    std::vector<Bytes> m_scrolls_offsets{};
};

} // namespace Scrolls
```
