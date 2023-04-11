SELECT DISTINCT P.name
FROM CatchedPokemon AS C,Pokemon AS P
WHERE P.id=C.pid AND C.pid IN (
  SELECT C1.pid
  FROM Trainer AS T1,CatchedPokemon AS C1
  WHERE C1.owner_id=T1.id AND T1.hometown='Sangnok City'
) AND C.pid IN (
  SELECT C2.pid
  FROM Trainer AS T2,CatchedPokemon AS C2
  WHERE C2.owner_id=T2.id AND T2.hometown='brown City'
)
ORDER BY P.name