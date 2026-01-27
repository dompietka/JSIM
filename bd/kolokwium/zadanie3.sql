WITH solved AS (
    SELECT puzzle, receiver
	FROM Challenge 
	WHERE solved IS NOT NULL
	GROUP BY receiver, puzzle
)
SELECT puzzle --, COUNT(*)
FROM solved
GROUP BY puzzle
HAVING COUNT(*)+1 = (SELECT COUNT(*) FROM Player);

-- uzasadnienie poprawności: w bazie nie może być niepoprawnego wyzwania, 
-- zatem nie pojawi się wyzwanie ktorego odbiorcą jest autor
-- zatem wystarczy policzyc ile osob rozwiazalo dana zagadke, i to musi być liczba osob - 1
