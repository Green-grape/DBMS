SELECT P.name
FROM Pokemon AS P
WHERE P.type IN(
  SELECT P1.type
  FROM Pokemon AS P1
  GROUP BY P1.type
  HAVING COUNT(P1.type)>=ALL(
  SELECT COUNT(p3.id)
  FROM Pokemon AS P3
  GROUP BY P3.type
  HAVING COUNT(p3.id)<ALL(
    SELECT MAX(mycount)
    FROM (SELECT COUNT(P4.id) mycount FROM Pokemon AS P4 GROUP BY P4.type)CNT
  )
))
ORDER BY P.name 
