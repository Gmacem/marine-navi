-- kSelectHazardDepthPoints

WITH l AS (
    SELECT $1 as geom
)
SELECT 
    p.depth,
    AsText(p.geom) AS point_wkt,
    ST_Distance(
        Transform(p.geom, 3857),
        Transform(l.geom, 3857)
    ) AS distance_meters
FROM 
    depths p
WHERE 
    ST_Distance(
        Transform(p.geom, 3857),
        Transform(l.geom, 3857)
    ) <= $2;
