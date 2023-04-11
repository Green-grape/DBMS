SELECT T1.name,MAX(C1.level)
FROM CatchedPokemon AS C1,Trainer AS T1
WHERE C1.owner_id=T1.id
GROUP BY T1.id
HAVING COUNT(C1.pid)>=4
ORDER BY T1.name