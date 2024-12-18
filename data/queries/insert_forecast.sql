-- kInsertForecast

INSERT INTO forecasts (source)
VALUES ($1)
RETURNING id;
