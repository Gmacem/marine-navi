-- kSelectForecastLocation

SELECT lat, lon FROM forecast_records
WHERE id = $1;
