-- SELECT field_39added50cec43059d7c203566a3913b, field_be41f68fcdc8403f9e6a9b62f8246806, field_f992b70793644b318989171528fd6da6, field_f0bd3cec8dc0481c9cd452f82635a70b, filter_id, highlight_filter_id, view_index
-- FROM logs
-- WHERE (view_index >= 0 AND view_index < 38)
-- ORDER BY view_index ASC;

-- PRAGMA table_info(logs);

-- SELECT MIN(view_index), MAX(view_index), COUNT(*) FROM filtered_logs;

-- SELECT COUNT(*)
-- FROM filtered_logs;

SELECT * FROM (
    SELECT * FROM logs 
    ORDER BY id DESC
    LIMIT 50
) 
ORDER BY id ASC;

-- SELECT COUNT(*)
-- from logs;
