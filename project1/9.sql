SELECT T.name,AVG(C.level)
FROM Gym AS G,Trainer AS T, CatchedPokemon AS C
WHERE G.leader_id=T.id AND C.owner_id=T.id
GROUP BY G.leader_id
ORDER BY T.name ASC