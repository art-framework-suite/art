.headers on
.mode column

-- Create temporary table that creates DeltaVsize value to be used
CREATE TEMPORARY TABLE delta AS SELECT Path,ModuleLabel,ModuleType,(a2.Vsize-a1.Vsize) AS DeltaVsize, Run,SubRun,Event FROM ModuleInfo AS a2
JOIN ModuleInfo AS a1
USING (Path,ModuleLabel,ModuleType,Run,SubRun,Event)
WHERE a2.Step='PostProcessModule' AND a1.Step='PreProcessModule';

-- Print top 3 (if possible) events per module that contribute to change in Vsize.
SELECT a.Path,
       a.ModuleLabel,
       a.ModuleType,
       a.DeltaVsize AS "DeltaVsize (MB)",
       a.Run,
       a.SubRun,
       a.Event
FROM delta AS a
WHERE (SELECT COUNT(*)+1
       FROM delta AS b
       WHERE b.Path = a.Path
       AND b.ModuleLabel = a.ModuleLabel
       AND b.DeltaVsize > a.DeltaVsize) < 4
AND a.DeltaVsize > 0
ORDER BY a.Path, a.ModuleLabel ASC, a.DeltaVsize DESC;
