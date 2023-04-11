SELECT T.name,COUNT(C.owner_id)
FROM Trainer AS T,CatchedPokemon AS C, Gym AS G
WHERE T.id=C.owner_id AND G.leader_id=T.id
GROUP BY G.leader_id
ORDER BY T.name ASC