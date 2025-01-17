-- kSelectHazardDepthPoints

WITH points(id, geom) AS (
    VALUES $1
)
SELECT 
    depths.depth, 
    ST_AsText(depths.geom) as geom, 
    points.id 
FROM depths 
INNER JOIN 
    points 
ON 
    ST_Distance(
        points.geom,
        depths.geom
    ) <= $2
WHERE 
    -depths.depth <= $3
ORDER BY points.id ASC;
