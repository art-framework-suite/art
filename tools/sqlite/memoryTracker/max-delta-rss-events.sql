.headers on
.mode column
SELECT Run,SubRun,Event,(b.RSS-a.RSS) AS "DeltaRSS (MB)" FROM EventInfo AS a
JOIN EventInfo AS b
USING (Run,SubRun,Event)
WHERE a.Step='PreProcessEvent' AND b.Step='PostProcessEvent'
ORDER BY "DeltaRSS (MB)" DESC LIMIT 5;
