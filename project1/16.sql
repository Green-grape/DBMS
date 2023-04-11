SELECT COUNT(P.id)
FROM Pokemon AS P
WHERE P.type IN ('Water','Electric','Psychic')