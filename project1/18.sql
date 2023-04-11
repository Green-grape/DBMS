SELECT AVG(C.level)
FROM CatchedPokemon AS C, Gym AS G,Trainer AS T
WHERE C.owner_id=T.id AND G.leader_id=T.id