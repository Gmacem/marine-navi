-- kSelectClosestForecasts

WITH path(id, X, Y) AS (
    VALUES $1
), fwd as (
    SELECT forecast_records.id as fid, path.id as pid,
    (forecast_records.lon - path.X) * (forecast_records.lon - path.X) 
        + (forecast_records.lat - path.Y) * (forecast_records.lat - path.Y) 
        as dst, forecast_records.wave_height, forecast_records.swell_height 
    from forecast_records 
    CROSS JOIN path 
    WHERE $2 < forecast_records.end_at) 
SELECT mfwd.dst, fwd.dst, fwd.fid, fwd.pid, fwd.wave_height, 
fwd.swell_height from fwd 
INNER JOIN ( 
    select fid, min(dst) as dst FROM fwd GROUP BY pid) mfwd 
on fwd.dst = mfwd.dst and fwd.fid = mfwd.fid;
