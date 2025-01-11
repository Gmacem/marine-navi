-- kSelectHazardDepthPoints

SELECT load_extension('mod_spatialite');

WITH triangles(id, geom, height) AS (
    VALUES $1
)
SELECT 
    ST_AsText(d.geom) as geom 
    d.depth, 
    triangles.id 
FROM depths d 
INNER JOIN 
    triangles
ON 
    ST_Within(d.geom, triangles.geom) 
WHERE 
    d.depth <= triangles.height
ORDER BY triangles.id ASC;
