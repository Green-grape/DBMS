SELECT T.hometown,AVG(C.level)
FROM Trainer AS T,CatchedPokemon AS C
WHERE T.id=C.owner_id
GROUP BY T.hometown
ORDER BY AVG(C.level)