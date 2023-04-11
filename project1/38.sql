SELECT P.name
FROM Pokemon AS P,Evolution AS E
WHERE P.id=E.after_id
AND P.id NOT IN (
  SELECT E1.before_id
  FROM Evolution AS E1
)
ORDER BY P.name