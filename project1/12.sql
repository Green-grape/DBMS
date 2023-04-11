SELECT DISTINCT P.name,P.type
FROM CatchedPokemon AS C, Pokemon AS P
WHERE C.pid=P.id AND C.level>=30
ORDER BY P.name ASC