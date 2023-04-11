SELECT T1.name
FROM Trainer AS T1
WHERE T1.id NOT IN(
  SELECT T.id
  FROM Trainer AS T, Gym AS G
  WHERE T.id=G.leader_id
  ORDER BY T.id)
ORDER BY T1.name ASC