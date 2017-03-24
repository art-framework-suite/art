.headers on
.mode column

-- Create temporary table that creates DeltaRSS value to be used
CREATE TEMPORARY TABLE delta AS SELECT Path,ModuleLabel,ModuleType,(a2.RSS-a1.RSS) AS DeltaRSS, Run,SubRun,Event FROM ModuleInfo AS a2
JOIN ModuleInfo AS a1
USING (Path,ModuleLabel,ModuleType,Run,SubRun,Event)
WHERE a2.Step='PostProcessModule' AND a1.Step='PreProcessModule';

-- Print top 3 (if possible) events that contribute per module to change in RSS.
SELECT a.Path,
       a.ModuleLabel,
       a.ModuleType,
       a.DeltaRSS AS "DeltaRSS (MB)",
       a.Run,
       a.SubRun,
       a.Event
FROM delta AS a
WHERE (SELECT COUNT(*)+1
       FROM delta AS b
       WHERE b.Path = a.Path
       AND b.ModuleLabel = a.ModuleLabel
       AND b.DeltaRSS > a.DeltaRSS) < 4
AND a.DeltaRSS > 0
ORDER BY a.Path, a.ModuleLabel ASC, a.DeltaRSS DESC;
