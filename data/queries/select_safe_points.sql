-- kSelectSafePoints

SELECT name, ST_AsText(geom) as geom FROM safe_points;
