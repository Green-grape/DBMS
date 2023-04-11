SELECT DISTINCT T.name
FROM Trainer AS T,CatchedPokemon AS C,Evolution AS E
WHERE T.id=C.owner_id AND
C.pid=E.after_id AND
C.pid NOT IN(
  SELECT E2.before_id
  FROM Evolution AS E2
)
ORDER BY T.name