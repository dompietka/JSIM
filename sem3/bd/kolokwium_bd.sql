@zagadki-oracle.sql

WITH chall AS (
	SELECT receiver, puzzle, MIN(received), MAX(solved), MAX(solved)-MIN(received)+1 AS time
	FROM challenge 
	WHERE solved IS NOT NULL
	GROUP BY receiver, puzzle 
	ORDER BY receiver
	)
SELECT player.name AS gracz, AVG(chall.time) 
FROM player LEFT JOIN chall ON player.name = chall.receiver 
GROUP BY player.name;
