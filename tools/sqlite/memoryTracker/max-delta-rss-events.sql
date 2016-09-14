.headers on
.mode column
SELECT Run,SubRun,Event,DeltaRSS AS "DeltaRSS (MiB)" FROM EventInfo ORDER BY DeltaRSS DESC LIMIT 5;
