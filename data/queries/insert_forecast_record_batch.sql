-- kInserForecastRecordBatchQuery

SELECT load_extension('mod_spatialite');

INSERT INTO forecast_records (started_at, end_at, wave_height, swell_height, forecast_id, geom)
VALUES $1;
