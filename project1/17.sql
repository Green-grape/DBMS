SELECT COUNT(DISTINCT C.pid)
FROM Trainer AS T, CatchedPokemon AS C
WHERE T.id=C.owner_id AND T.hometown='Sangnok City'