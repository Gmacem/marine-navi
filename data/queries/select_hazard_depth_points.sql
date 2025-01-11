-- kSelectHazardDepthPoints

WITH triangles(id, geom, height) AS (
    VALUES $1
)
SELECT 
    d.depth, 
    ST_AsText(d.geom) as geom, 
    triangles.id 
FROM depths d 
INNER JOIN 
    triangles
ON 
    ST_Within(d.geom, triangles.geom) 
WHERE 
    -d.depth <= triangles.height
ORDER BY triangles.id ASC;
