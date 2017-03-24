.headers on
.mode column
SELECT Run,SubRun,Event,RSS AS "RSS (MB)" FROM EventInfo WHERE Step='PostProcessEvent' ORDER BY RSS DESC LIMIT 5;
