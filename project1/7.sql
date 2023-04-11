SELECT T.name
FROM Trainer AS T, City AS C
WHERE T.hometown=C.name
ORDER BY T.hometown ASC