WITH nastepny_rejs AS(
	SELECT numer,
		LEAD (numer, 1) OVER (
			PARTITION BY statek
			ORDER BY poczatek
			) AS kolejny_numer
	FROM rejs
	)
	
SELECT rejs1.statek, rejs1.dokad, rejs1.koniec, rejs2.skad, rejs2.poczatek
FROM 
	(SELECT rejs.numer AS numer, dokad, koniec, statek, kolejny_numer 
	FROM rejs 
	RIGHT JOIN nastepny_rejs ON rejs.numer = nastepny_rejs.numer) rejs1 
JOIN rejs rejs2 
	ON rejs1.kolejny_numer=rejs2.numer AND rejs1.statek = rejs2.statek
WHERE rejs1.dokad != rejs2.skad;   


