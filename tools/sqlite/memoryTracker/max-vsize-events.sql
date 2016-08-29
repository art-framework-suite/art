.headers on
.mode column
SELECT Run,SubRun,Event,Vsize AS "Vsize (MiB)" FROM EventInfo ORDER BY Vsize DESC LIMIT 5;
