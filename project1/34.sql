SELECT P.name,C.level,C.nickname
FROM CatchedPokemon AS C,Pokemon AS P,Gym AS G
WHERE P.id=C.pid 
AND G.leader_id=C.owner_id
AND C.nickname LIKE 'A%'
ORDER BY P.name DESC