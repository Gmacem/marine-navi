-- kSelectHazardDepthPoints

WITH points(id, geom) AS (
    VALUES $1
)
SELECT 
    d.depth, 
    ST_AsText(d.geom) as geom, 
    points.id 
FROM depths d 
INNER JOIN 
    points 
ON 
    ST_Distance(
        points.geom,
        d.geom
    ) <= $2
WHERE 
    -d.depth <= $3
ORDER BY points.id ASC;
