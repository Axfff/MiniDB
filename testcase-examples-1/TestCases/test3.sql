CREATE DATABASE TestDB;

USE DATABASE TestDB;

CREATE TABLE Employees (
                           EmployeeID INTEGER,
                           Name TEXT,
                           DepartmentID INTEGER
);

CREATE TABLE Departments (
                             DepartmentID INTEGER,
                             DepartmentName TEXT
);

INSERT INTO Employees VALUES (1, 'Alice', 101);
INSERT INTO Employees VALUES (2, 'Bob', 102);
INSERT INTO Employees VALUES (3, 'Charlie', 101);
INSERT INTO Employees VALUES (4, 'David', 103);

INSERT INTO Departments VALUES (101, 'HR');
INSERT INTO Departments VALUES (102, 'Engineering');
INSERT INTO Departments VALUES (103, 'Marketing');

SELECT Name, DepartmentName
FROM Employees
    INNER JOIN Departments
        ON Employees.DepartmentID = Departments.DepartmentID;

SELECT Name, DepartmentName
FROM Employees
    INNER JOIN Departments
        ON Employees.DepartmentID = Departments.DepartmentID
WHERE Employees.DepartmentID = 101;

CREATE TABLE Projects (
                          ProjectID INTEGER,
                          ProjectName TEXT,
                          DepartmentID INTEGER
);

INSERT INTO Projects VALUES (1, 'Project A', 101);
INSERT INTO Projects VALUES (2, 'Project B', 102);
INSERT INTO Projects VALUES (3, 'Project C', 103);

SELECT Employees.Name, Departments.DepartmentName, Projects.ProjectName
FROM Employees
    INNER JOIN Departments
        ON Employees.DepartmentID = Departments.DepartmentID
    INNER JOIN Projects
        ON Departments.DepartmentID = Projects.DepartmentID;


