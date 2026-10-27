WITH withauthors AS (
    SELECT puzzle, sender, receiver, author, received
    FROM Challenge LEFT JOIN Puzzle ON Challenge.puzzle = Puzzle.id
), chains AS (
    SELECT puzzle, sender, receiver, PRIOR receiver AS prev_rec, PRIOR sender AS prev_from, PRIOR puzzle AS prev_puz, LEVEL AS lenght
    FROM withauthors
    START WITH sender = author
    CONNECT BY PRIOR receiver = sender 
    AND PRIOR puzzle = puzzle
    ORDER BY received
)
SELECT MAX(lenght) AS longest_chain --, puzzle AS puzzle_nr
FROM chains
GROUP BY puzzle
HAVING MAX(lenght) = (SELECT MAX(lenght) FROM chains);

-- zakładając że recordy w bazie są zgodne z treścią, nie musimy sprawdzać stempla czasowego, 
-- wystarczy sprawdzic czy odbiorca jest nadawcą kolejnego i wybrać maksimum
-- lancuch zawsze musi zaczynac sie od autora bo nikt inny nie moze po raz 1 wyslac zadania