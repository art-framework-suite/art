.headers on
.mode column
SELECT Run,SubRun,Event,RSS AS "RSS (MiB)" FROM EventInfo ORDER BY RSS DESC LIMIT 5;
