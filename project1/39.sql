SELECT T.name
FROM Trainer AS T,CatchedPokemon AS C
WHERE T.id=C.owner_id
GROUP BY T.id
HAVING COUNT(DISTINCT C.pid)<COUNT(C.pid)
ORDER BY T.name