//
// Created by zhaoj on 2024/11/19.
//

#ifndef COMANDS_H
#define COMANDS_H

#include <string>
#include "Utils.h"
#include <vector>


class Command {
public:
  virtual ~Command() = default;

  [[nodiscard]] virtual std::string getType() const = 0;
};


class CreateDatabaseCommand : public Command {
public:
  explicit CreateDatabaseCommand(const std::string& dbName)
      : databaseName(dbName) {}

  std::string getType() const override {
    return "CREATE_DATABASE";
  }

  const std::string& getDatabaseName() const {
    return databaseName;
  }

private:
  std::string databaseName;
};


class UseDatabaseCommand : public Command {
public:
  explicit UseDatabaseCommand(const std::string& dbName)
      : databaseName(dbName) {}

  std::string getType() const override {
    return "USE_DATABASE";
  }

  const std::string& getDatabaseName() const {
    return databaseName;
  }

private:
  std::string databaseName;
};


class CreateTableCommand : public Command {
public:
  CreateTableCommand(const std::string& tblName,
                     const std::vector<std::pair<std::string, DataType>>& cols)
      : tableName(tblName), columns(cols) {}

  std::string getType() const override {
    return "CREATE_TABLE";
  }

  const std::string& getTableName() const {
    return tableName;
  }

  const std::vector<std::pair<std::string, DataType>>& getColumns() const {
    return columns;
  }

private:
  std::string tableName;
  std::vector<std::pair<std::string, DataType>> columns;
};


class DropTableCommand : public Command {
public:
  explicit DropTableCommand(const std::string& tblName)
      : tableName(tblName) {}

  std::string getType() const override {
    return "DROP_TABLE";
  }

  const std::string& getTableName() const {
    return tableName;
  }

private:
  std::string tableName;
};


class InsertCommand : public Command {
public:
  InsertCommand(const std::string& tblName, const std::vector<std::string>& vals)
      : tableName(tblName), values(vals) {}

  std::string getType() const override {
    return "INSERT";
  }

  const std::string& getTableName() const {
    return tableName;
  }

  const std::vector<std::string>& getValues() const {
    return values;
  }

private:
  std::string tableName;
  std::vector<std::string> values;
};


class SelectCommand : public Command {
public:
  struct JoinClause {
    std::string joinType;    // e.g., "INNER JOIN"
    std::string tableName;
    std::string condition;
  };

  SelectCommand(const std::vector<std::string>& cols,
                const std::string& tblName,
                const std::string& whereClause = "",
                const std::vector<JoinClause>& joins = {})
      : columns(cols), tableName(tblName), where(whereClause), joins(joins) {}

  std::string getType() const override {
    return "SELECT";
  }

  const std::vector<std::string>& getColumns() const { return columns; }
  const std::string& getTableName() const { return tableName; }
  const std::string& getWhereClause() const { return where; }
  const std::vector<JoinClause>& getJoins() const { return joins; }

private:
  std::vector<std::string> columns;
  std::string tableName;
  std::string where;
  std::vector<JoinClause> joins;
};


class UpdateCommand : public Command {
public:
  UpdateCommand(const std::string& tblName,
                const std::vector<std::pair<std::string, std::string>>& updates,
                const std::string& whereClause = "")
      : tableName(tblName), setClauses(updates), where(whereClause) {}

  std::string getType() const override {
    return "UPDATE";
  }

  const std::string& getTableName() const {
    return tableName;
  }

  const std::vector<std::pair<std::string, std::string>>& getSetClauses() const {
    return setClauses;
  }

  const std::string& getWhereClause() const {
    return where;
  }

private:
  std::string tableName;
  std::vector<std::pair<std::string, std::string>> setClauses;
  std::string where;
};


class DeleteCommand : public Command {
public:
  DeleteCommand(const std::string& tblName, const std::string& whereClause = "")
      : tableName(tblName), where(whereClause) {}

  std::string getType() const override {
    return "DELETE";
  }

  const std::string& getTableName() const {
    return tableName;
  }

  const std::string& getWhereClause() const {
    return where;
  }

private:
  std::string tableName;
  std::string where;
};


#endif //COMANDS_H
