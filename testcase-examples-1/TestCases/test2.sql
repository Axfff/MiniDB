CREATE DATABASE TestDB1;

USE DATABASE TestDB1;

CREATE TABLE Employees (
                           EmployeeID INTEGER,
                           Name TEXT,
                           Age INTEGER,
                           Salary FLOAT,
                           DepartmentID INTEGER
);

CREATE TABLE Departments (
                             DepartmentID INTEGER,
                             DepartmentName TEXT
);

INSERT INTO Employees VALUES (1, 'Alice', 30, 60000, 101);
INSERT INTO Employees VALUES (2, 'Bob', 40, 80000, 102);
INSERT INTO Employees VALUES (3, 'Charlie', 25, 50000, 101);
INSERT INTO Employees VALUES (4, 'David', 35, 70000, 103);
INSERT INTO Employees VALUES (5, 'Eve', 28, 65000, 102);

INSERT INTO Departments VALUES (101, 'HR');
INSERT INTO Departments VALUES (102, 'Engineering');
INSERT INTO Departments VALUES (103, 'Marketing');

SELECT Name, Age, Salary
FROM Employees
WHERE Age > 30 AND Salary > 60000;

SELECT Name, Age, Salary FROM Employees WHERE Age > 30 OR Salary > 60000;

SELECT Name, Age, Salary FROM Employees WHERE Age > 30 AND Salary > 65000 AND DepartmentID = 102;

SELECT Name, Age, Salary
FROM Employees
WHERE Age > 30 AND DepartmentID = 103 OR Salary < 70000;

SELECT Name, Age, Salary
FROM Employees
WHERE DepartmentID = 101 OR DepartmentID = 102 AND Age < 35;



