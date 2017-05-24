.headers on
.mode column
SELECT Run,SubRun,Event,SUM(Time) AS "Full event time (sec)" FROM (
       SELECT Run,SubRun,Event,Time FROM TimeEvent
       UNION
       SELECT Run,SubRun,Event,Time FROM TimeModule WHERE ModuleType LIKE '%(write)'
       UNION
       SELECT Run,SubRun,Event,Time FROM TimeSource)
GROUP BY Run,Subrun,Event
ORDER BY Time DESC
LIMIT 5;
