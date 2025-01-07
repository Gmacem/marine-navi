-- kInsertDepths

SELECT load_extension('mod_spatialite');

DROP TABLE IF EXISTS depths;

CREATE TABLE IF NOT EXISTS depths (
    id      INTEGER PRIMARY KEY AUTOINCREMENT,
    depth   REAL NOT NULL,
);
SELECT AddGeometryColumn('depths', 'geom', 4326, 'POINT', 'XY');
SELECT CreateSpatialIndex('depths', 'geom');

INSERT INTO depths (depth, geom)
VALUES $1;
