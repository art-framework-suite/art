.headers on
.mode column
SELECT Run,SubRun,Event,DeltaVsize AS "DeltaVsize (MiB)" FROM EventInfo ORDER BY DeltaVsize DESC LIMIT 5;
