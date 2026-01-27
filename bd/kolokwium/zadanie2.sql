WITH solved AS (
    SELECT puzzle, MAX(solved)-MIN(received)+1 AS time
	FROM Challenge 
	WHERE solved IS NOT NULL
	GROUP BY receiver, puzzle
), min_time AS (
    SELECT puzzle, MIN(time) AS mintime
    FROM solved
    GROUP BY puzzle
)
SELECT puzzle 
FROM min_time
WHERE mintime = (SELECT MAX(mintime) FROM min_time);

-- uznaje ze czas rozwiazania przez osobe x liczy się od pierwszego otrzymania tej zagadki, 
-- tzn jesli osoba x otrzymala zagadke 1 w chwili 3 i w chwili 5, a rozwiazala w chwili 7,
-- to czas wynosi 7 - 3 + 1 = 5