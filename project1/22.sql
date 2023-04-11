SELECT P.type,COUNT(p.id)
FROM Pokemon AS P
GROUP BY P.type
ORDER BY COUNT(P.id),P.type ASC