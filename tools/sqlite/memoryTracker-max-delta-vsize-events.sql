.headers on
.mode column
SELECT Run,SubRun,Event,DeltaVsize AS "Δ Vsize (MiB)" FROM EventInfo ORDER BY DeltaVsize DESC LIMIT 5;
