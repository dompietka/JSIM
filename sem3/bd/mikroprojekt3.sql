WITH paths AS (
	SELECT autorstwo1.Autor AS autor1, autorstwo2.Autor AS autor2
	FROM autorstwo autorstwo1
	JOIN autorstwo autorstwo2
		ON autorstwo1.Praca = autorstwo2.Praca
	WHERE autor1 != autor2
	)
	
bfs(autor, dist) AS (
	SELECT 'Pilipczuk Mi' AS autor, 0 AS dist 
	FROM dual
	UNION ALL
		SELECT paths.autor2, bfs.dist + 1
		FROM bfs 
		JOIN paths ON paths.autor1 = bfs.autor
		WHERE NOT EXISTS (SELECT autor FROM bfs WHERE autor = paths.autor2)
	)
	
SELECT autorzy.ID AS autor, bfs.dist AS liczba_pilipczuka
FROM autorzy 
LEFT JOIN bfs ON bfs.autor = autorzy.ID
ORDER BY a.ID;
