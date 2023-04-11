SELECT SUM(C.level)
FROM Trainer AS T,CatchedPokemon AS C
WHERE T.id=C.owner_id AND T.name='Matis'