SELECT P1.id,P1.name,P2.name,P3.name
FROM Pokemon AS P1,Pokemon AS P2, Pokemon AS P3,Evolution AS E1,Evolution AS E2
WHERE P1.id=E1.before_id AND P2.id=E1.after_id
AND P2.id=E2.before_id AND P3.id=E2.after_id
AND P1.id NOT IN (
  SELECT E3.after_id
  FROM Evolution AS E3
)
AND P3.id NOT IN(
  SELECT E4.before_id
  FROM Evolution AS E4
)
ORDER BY P1.id