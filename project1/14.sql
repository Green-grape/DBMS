SELECT P.name
FROM Pokemon AS P,Evolution AS E
WHERE P.id=E.before_id AND P.type='Grass'