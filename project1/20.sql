SELECT T.name,COUNT(C.owner_id)
FROM Trainer AS T,CatchedPokemon AS C
WHERE T.id=C.owner_id AND T.hometown='Sangnok City'
GROUP BY T.id
ORDER BY COUNT(C.owner_id)