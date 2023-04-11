SELECT COUNT(DISTINCT P.type)
FROM Trainer AS T,Gym AS G,CatchedPokemon AS C,Pokemon AS P
WHERE T.id=G.leader_id 
AND C.owner_id=T.id
AND P.id=C.pid
AND T.hometown='Sangnok City'