.headers on
.mode column
SELECT
        Source,
        min(Time) AS "Min. (sec)",
        avg(Time) AS "Avg. (sec)",
        max(Time) AS "Max. (sec)",
        count(*) AS "Sample size (events)"
FROM TimeSource
