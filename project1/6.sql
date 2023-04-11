SELECT T.name
FROM Trainer AS T, City AS C
WHERE T.hometown=C.name AND T.hometown='Blue City'
ORDER BY T.name ASC