.headers on
.mode column
SELECT Run,SubRun,Event,Vsize AS "Vsize (MB)" FROM EventInfo WHERE Step='PostProcessEvent' ORDER BY Vsize DESC LIMIT 5;
