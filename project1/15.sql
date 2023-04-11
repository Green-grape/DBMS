SELECT T.id,COUNT(C.owner_id)
FROM Trainer AS T,CatchedPokemon AS C
WHERE T.id=C.owner_id
GROUP BY T.id
HAVING COUNT(C.owner_id)>=ALL(
  SELECT COUNT(C1.owner_id)
  FROM Trainer AS T1,CatchedPokemon AS C1
  WHERE T1.id=C1.owner_id
  GROUP BY T1.id)
ORDER BY T.id ASC