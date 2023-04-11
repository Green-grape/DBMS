SELECT P.name
FROM Pokemon AS P
WHERE P.id NOT IN(
  SELECT C.pid
  FROM CatchedPokemon AS C
)
ORDER BY P.name