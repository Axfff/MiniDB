//
// Created by zhaoj on 2024/11/12.
//

#ifndef COLUMN_H
#define COLUMN_H


#include <vector>
#include <memory>
#include "Utils.h"
#include "Value.h"

class Column {
public:
    Column(std::string title, const DataType& type): type(type), title(std::move(title)) {};

    [[nodiscard]] std::string getTitle() const {return title;}

    [[nodiscard]] bool addValue(std::shared_ptr<Value> value) {
        if (value->getType() != type) {
            std::cerr << "type not match when adding value to column " << this->title << std::endl;
            return false;
        }
        values.push_back(std::move(value));
        return true;
    }

    // Remove the value at the given index
    bool removeValueAt(size_t index) {
        if (index >= values.size()) {
            std::cerr << "Index out of range in removeValueAt for column " << title << "\n";
            return false;
        }
        values.erase(values.begin() + index);
        return true;
    }

    // Update the value at the given index
    bool updateValueAt(size_t index, std::shared_ptr<Value> newValue) {
        if (index >= values.size()) {
            std::cerr << "Index out of range in updateValueAt for column " << title << "\n";
            return false;
        }
        if (newValue->getType() != type) {
            std::cerr << "Type mismatch in updateValueAt for column " << title << "\n";
            return false;
        }
        values[index] = std::move(newValue);
        return true;
    }

    [[nodiscard]] DataType getType() const { return type; }

private:
    DataType type;
    std::string title;
    std::vector<std::shared_ptr<Value>> values;
};



#endif //COLUMN_H
