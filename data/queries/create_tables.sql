-- kCreateTables

SELECT load_extension('mod_spatialite');
PRAGMA trusted_schema=1;
SELECT InitSpatialMetaData()
WHERE NOT EXISTS (
    SELECT 1 
    FROM sqlite_master 
    WHERE type = 'table' 
      AND name = 'spatial_ref_sys'
);

CREATE TABLE IF NOT EXISTS forecasts (
id         INTEGER PRIMARY KEY AUTOINCREMENT, 
created_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL, 
source     INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS forecast_records (
id              INTEGER PRIMARY KEY AUTOINCREMENT, 
created_at      TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL, 
started_at      TEXT NOT NULL, 
end_at          TEXT NOT NULL, 
wave_height     REAL, 
swell_height    REAL, 
forecast_id     INTEGER NOT NULL, 
FOREIGN KEY(forecast_id) REFERENCES forecasts(id));

SELECT AddGeometryColumn(
    'forecast_records', 'geom', 4326, 'POINT', 'XY'
), CreateSpatialIndex('forecast_records', 'geom')
WHERE NOT EXISTS (
    SELECT 1 
    FROM geometry_columns 
    WHERE f_table_name = 'forecast_records' 
      AND f_geometry_column = 'geom'
);

CREATE TABLE IF NOT EXISTS depths (
    id      INTEGER PRIMARY KEY AUTOINCREMENT,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL,
    depth   REAL NOT NULL
);

SELECT AddGeometryColumn(
    'depths', 'geom', 4326, 'POINT', 'XY'
), CreateSpatialIndex('depths', 'geom')
WHERE NOT EXISTS (
    SELECT 1 
    FROM geometry_columns 
    WHERE f_table_name = 'depths' 
      AND f_geometry_column = 'geom'
);
