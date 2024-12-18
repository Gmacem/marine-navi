-- kCreateTables

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
lat             REAL NOT NULL, 
lon             REAL NOT NULL, 
wave_height     REAL, 
swell_height    REAL, 
forecast_id     INTEGER NOT NULL, 
FOREIGN KEY(forecast_id) REFERENCES forecasts(id));

CREATE UNIQUE INDEX IF NOT EXISTS forecast_records_idx ON 
forecast_records (
started_at, end_at, lat, lon, forecast_id);
