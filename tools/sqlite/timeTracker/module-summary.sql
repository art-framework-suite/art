.headers on
.mode column
.width 10 15
SELECT
        Path,ModuleLabel,ModuleType,
        min(Time) AS "Min. (sec)",
        avg(Time) AS "Avg. (sec)",
        max(Time) AS "Max. (sec)",
        count(*) AS "Sample size (events)"
FROM TimeModule
GROUP BY Path,ModuleLabel,ModuleType;
