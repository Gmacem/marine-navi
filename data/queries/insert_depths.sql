-- kInsertDepths

SELECT load_extension('mod_spatialite');

INSERT INTO depths (depth, geom)
VALUES $1;
