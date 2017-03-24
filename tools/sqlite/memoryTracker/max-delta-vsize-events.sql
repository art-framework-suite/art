.headers on
.mode column
SELECT Run,SubRun,Event,(b.Vsize-a.Vsize) AS "DeltaVsize (MB)" FROM EventInfo AS a
JOIN EventInfo AS b
USING (Run,SubRun,Event)
WHERE a.Step='PreProcessEvent' AND b.Step='PostProcessEvent'
ORDER BY "DeltaVsize (MB)" DESC LIMIT 5;
