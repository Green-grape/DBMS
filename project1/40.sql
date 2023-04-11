SELECT T.hometown,C.nickname
FROM Trainer AS T,CatchedPokemon AS C
WHERE T.id=C.owner_id AND
(C.level,T.hometown) IN (
  SELECT MAX(C1.level),T1.hometown
  FROM Trainer AS T1,CatchedPokemon AS C1
  WHERE T1.id=C1.owner_id
  GROUP BY T1.hometown
)
ORDER BY T.hometown