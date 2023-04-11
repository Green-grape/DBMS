SELECT DISTINCT P.id,P.name
FROM Trainer AS T, CatchedPokemon AS C, Pokemon AS P
WHERE C.owner_id=T.id AND P.id=C.pid AND T.hometown='Sangnok City'
ORDER BY P.id ASC