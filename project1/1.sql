SELECT T.name
FROM Trainer AS T, CatchedPokemon AS C
WHERE T.id=C.owner_id
GROUP BY T.id
HAVING COUNT(C.id)>=3
ORDER BY COUNT(C.id) DESC