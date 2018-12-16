-- SELECT quantileMomentsSketchMergeState(q) FROM (SELECT quantileMomentsSketchState(number) AS q from numbers(100) GROUP BY number) FORMAT RowBinary

SELECT quantileMomentsSketchState(toFloat64(number)) FROM numbers(10) FORMAT RowBinary

