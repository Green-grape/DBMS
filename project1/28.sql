SELECT T.name,AVG(C.level)
FROM CatchedPokemon AS C,Trainer AS T,Pokemon AS P
WHERE C.owner_id=T.id AND 
P.id=C.pid AND
P.type IN ('Normal','Electric')
GROUP BY T.id
ORDER BY AVG(C.level)