SELECT C.nickname
FROM CatchedPokemon AS C, Pokemon AS P
WHERE C.pid=P.id AND C.level>=50
ORDER BY C.nickname ASC