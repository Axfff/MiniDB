//
// Created by zhaoj on 2024/11/14.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>

enum class DataType {
  INT,
  FLOAT,
  TEXT,
};


std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INT: return "INTEGER";
        case DataType::FLOAT: return "FLOAT";
        case DataType::TEXT: return "TEXT";
        default: return "UNKNOWN";
    }
}


DataType stringToDataType(const std::string& str) {
    if (str == "INTEGER") return DataType::INT;
    else if (str == "FLOAT") return DataType::FLOAT;
    else if (str == "TEXT") return DataType::TEXT;
    else throw std::invalid_argument("Unknown DataType string: " + str);
}


/* TODO: commands needs to be implemented:
CREATE DATABASE database_name;

USE DATABASE database_name;

CREATE TABLE table_name (
    column1_name column1_type,
    column2_name column2_type,
    column3_name column3_type,
    ...
);

DROP TABLE table_name;

INSERT INTO table_name VALUES (value1, value2, ...);

SELECT column1, column2, ... FROM table_name;

SELECT column1, column2 FROM table_name WHERE condition1 AND/OR condition2;

SELECT student.Name, course_enrollment.Course
FROM student
INNER JOIN course_enrollment
ON student.StudentID = course_enrollment.StudentID;

UPDATE table_name
SET column1 = new_value1, column2 = new_value2, ...
WHERE condition;

DELETE FROM table_name
WHERE condition;
*/



#endif //UTILS_H
