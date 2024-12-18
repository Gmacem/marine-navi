-- kInserForecastRecordBatchQuery

INSERT INTO forecast_records (started_at, end_at, lat, lon, wave_height, swell_height, forecast_id)
VALUES $1;
