.headers on
.mode column
SELECT
        Run,SubRun,Event,Time AS "Time (s)"
FROM TimeEvent
ORDER BY Time DESC
LIMIT 5;
