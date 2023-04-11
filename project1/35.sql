SELECT P1.name
FROM Pokemon AS P1,Pokemon AS P2,Evolution AS E
WHERE P1.id=E.before_id AND P2.id=E.after_id
AND P1.id>P2.id
ORDER BY P1.name