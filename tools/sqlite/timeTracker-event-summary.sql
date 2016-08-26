.headers on
.mode column
SELECT
        'Full Event' as '',
        min(Time) AS "Min. (s)",
        avg(Time) AS "Avg. (s)",
        max(Time) AS "Max. (s)",
        count(*) AS "Sample size (events)"
FROM TimeEvent;
