.headers on
.mode column
SELECT
        'Full Event' as '',
        min(FullTime) AS "Min. (sec)",
        avg(FullTime) AS "Avg. (sec)",
        max(FullTime) AS "Max. (sec)",
        count(*) AS "Sample size (events)"
FROM (
     SELECT Run,SubRun,Event,SUM(Time) AS FullTime FROM (
            SELECT Run,SubRun,Event,Time FROM TimeEvent
            UNION
            SELECT Run,SubRun,Event,Time FROM TimeModule WHERE ModuleType LIKE '%(write)'
            UNION
            SELECT Run,SubRun,Event,Time FROM TimeSource)
     GROUP BY Run,SubRun,Event
)
