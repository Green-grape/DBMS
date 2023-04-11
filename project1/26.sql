SELECT P.name
FROM CatchedPokemon AS C,Pokemon AS P
WHERE P.id=C.pid AND C.nickname LIKE '% %'
ORDER BY P.name DESC