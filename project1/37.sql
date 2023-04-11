SELECT T.name,SUM(C.level)
FROM Trainer AS T,CatchedPokemon AS C
WHERE T.id=C.owner_id
GROUP BY T.id
HAVING SUM(C.level)>=ALL(
  SELECT SUM(C1.level)
  FROM Trainer AS T1,CatchedPokemon AS C1
  WHERE T1.id=C1.owner_id
  GROUP BY T1.id
)
ORDER BY T.name