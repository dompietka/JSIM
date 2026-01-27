WITH chall AS (
	SELECT receiver, puzzle, MIN(received), MAX(solved), MAX(solved)-MIN(received)+1 AS time
	FROM Challenge 
	WHERE solved IS NOT NULL
	GROUP BY receiver, puzzle 
	ORDER BY receiver
	)
SELECT Player.name AS gracz, AVG(chall.time) 
FROM Player LEFT JOIN chall ON player.name = chall.receiver 
GROUP BY Player.name;