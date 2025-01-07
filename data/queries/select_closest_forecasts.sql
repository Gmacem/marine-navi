-- kSelectClosestForecasts

WITH tmp_route(id, geom) AS (
    VALUES $1
) 
SELECT
    ST_AsText(fr.geom) AS location,
    fr.started_at,
    fr.end_at,
    fr.wave_height,
    fr.swell_height,
    ST_Distance(
        Transform(tr.geom, 3857),
        Transform(fr.geom, 3857)
    ) AS distance,
    tr.id as pid
FROM
    forecast_records fr
INNER JOIN
    tmp_route tr
ON
    ST_DistanceWithin(
        Transform(fr.geom, 3857),
        Transform(tr.geom, 3857),
        $2
) 
WHERE fr.started_at >= $3;
