.headers on
.mode column
.width 10 15
SELECT
        Path,ModuleLabel,ModuleType,
        min(Time) AS "Min. (s)",
        avg(Time) AS "Avg. (s)",
        max(Time) AS "Max. (s)",
        count(*) AS "Sample size (events)"
FROM TimeModule
GROUP BY Path,ModuleLabel,ModuleType;
