# MiniDB
2024.12.10

## Features
- **Full implementation:** including `CREATE`, `USE`, `INSERT`, `UPDATE`, `DELETE`, `SELECT` with `INNER JOIN`, and support for `WHERE`
- Support for **multi-`INNER JOIN`** was implemented:\
   E.g.
   ```SQL
   SELECT Employees.Name, Departments.DepartmentName, Projects.ProjectName
   FROM Employees
   INNER JOIN Departments
   ON Employees.DepartmentID = Departments.DepartmentID
   INNER JOIN Projects
   ON Departments.DepartmentID = Projects.DepartmentID;
  ```
- Support for complex `WHERE` condition is implemented by building a tree-like condition structure (**Note:** priority represented by parentheses or priority for AND is **not** implemented, currently the tree is built **from left to right**)\
   E.g.
   ```SQL
   SELECT Name, Age, Salary
   FROM Employees
   WHERE Age > 30 AND DepartmentID = 101 OR Salary > 70000;
  ```
  This will build a structure as below:
   ```
                OR
             /  \
          AND    (Salary > 70000)
         /   \
    (Age > 30) (DepartmentID = 101)
  ```

## Implementation

### Overall Design

The system is designed with modular components to handle the core functionalities of a relational database, including command parsing, data storage, and query execution.

### Framework Structure
```plaintext
+-------------+
| User Input  |
+-------------+
       |
       v
+-------------+
|   Parser    |
+-------------+
       |
       v
+-------------+          +-----------------+
|  Executor   | <------> |  DB Manager     |
+-------------+          +-----------------+
       |
       v
+-----------------+
| System Output   |
+-----------------+

```
- DB Manager:
  - DB (Database)
    - Tables
       - Rows
       - Columns
         - Values

### Components

1. **Database Manager (DatabaseManager)**
   - manages multiple databases, allowing the user to create, load, and switch between databases.
   - Handles persistence by saving databases to disk and loading them when needed.

2. **Database (Database)**
   - Represents a collection of tables within a database.
   - Handles table creation, deletion, and persistence.
   - Manages saving and loading data from file

3. **Table (Table)**
   - Represents a single table within a database.
   - Manages rows and columns, ensuring consistency when rows are added, updated, or deleted.

4. **Row and Column (Row, Column)**
   - Row: Represents a single record in a table, ensuring that its values match the column types defined in the table.
   - Column: Manages the values of a specific attribute across all rows in the table.

5. **Value (Value)**
   - Represents an individual cell in a table.
   - Supports basic initialization and comparisons for INT, FLOAT, and TEXT data types.

6. **Parser (Parser)**
   - Parses input SQL strings into command objects.

7. **Executor (Executor)**
   - Executes parsed SQL commands.
   - Implements logic for filtering rows based on conditions, applying joins, and updating or deleting rows.

8. **Utilities (Utils)**
   - Provides helper functions for handling data types.

## Generative AI Usage and Other References

The development of MiniDB was a collaborative effort involving human expertise and AI assistance. AI tools were utilized at various stages of the project to streamline coding tasks, identify potential issues, and provide suggestions for improvement. Human oversight ensured that the design, functionality, and implementation aligned with project goals. Specific contributions include:

1. **Framework Design:** Developed manually, drawing on references from online resources and AI-provided suggestions for structure and modularity.

2. **Module Implementation:**
   - Database Module: Mainly coded manually, with AI refining specific functionalities.
   - Parser and Commands: Drafted manually and completed with AI assistance for parsing logic.
   - Executor Module: Developed through AI-driven coding with continuous human guidance and review.

3.  **Debugging and Optimization:** AI helped identify bugs and potential inefficiencies, while human analysis provided context and final fixes.

4. **README file:** drafted and written by human, revised by AI, finally adjusted and formated by human.
