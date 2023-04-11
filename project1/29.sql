SELECT COUNT(P.id)
FROM CatchedPokemon AS C,Pokemon AS P
WHERE C.pid=P.id
GROUP BY P.type
ORDER BY P.type ASC