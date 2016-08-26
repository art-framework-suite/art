.headers on
.mode column
SELECT Run,SubRun,Event,DeltaRSS AS "Δ RSS (MiB)" FROM EventInfo ORDER BY DeltaRSS DESC LIMIT 5;
