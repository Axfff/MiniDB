//
// Created by zhaoj on 2024/11/14.
//

#ifndef VALUE_H
#define VALUE_H



#include <string>
#include <iomanip>
#include <sstream>
#include "Utils.h"

class Value {
public:
    Value(const DataType& type, const std::string& value): type(type) {set(value);}
    // ~Value() {}

    bool operator==(const Value& other) const {
        if (type == DataType::TEXT && other.type == DataType::TEXT) {
            return value == other.value;
        }
        if ((type == DataType::INT || type == DataType::FLOAT) && (other.type == DataType::INT || other.type == DataType::FLOAT)) {
            return compareValues(other);
        }
        throw std::invalid_argument("Cannot compare values of incompatible types");
    }

    bool operator!=(const Value& other) const {
        return !(*this == other);
    }

    bool operator<(const Value& other) const {
        if (type == DataType::TEXT && other.type == DataType::TEXT) {
            return value < other.value;
        }
        if ((type == DataType::INT || type == DataType::FLOAT) && (other.type == DataType::INT || other.type == DataType::FLOAT)) {
            return compareLessThan(other);
        }
        throw std::invalid_argument("Cannot compare values of incompatible types");
    }

    bool operator>(const Value& other) const {
        if (type == DataType::TEXT && other.type == DataType::TEXT) {
            return value > other.value;
        }
        if ((type == DataType::INT || type == DataType::FLOAT) && (other.type == DataType::INT || other.type == DataType::FLOAT)) {
            return compareGreaterThan(other);
        }
        throw std::invalid_argument("Cannot compare values of incompatible types");
    }

    bool operator<=(const Value& other) const {
        return !(*this > other);
    }

    bool operator>=(const Value& other) const {
        return !(*this < other);
    }

    bool set(const std::string& newValue) {
        try {
            // Check the current type and convert the new value accordingly
            if (type == DataType::TEXT) {
                // If the type is TEXT, we can directly assign the string
                value = newValue;
            } else if (type == DataType::INT) {
                // If the type is INT, try to convert to an integer
                try {
                    value = std::to_string(std::stoll(newValue));
                } catch (const std::invalid_argument& e) {
                    throw std::invalid_argument("Invalid value for INT: " + newValue);
                } catch (const std::out_of_range& e) {
                    throw std::out_of_range("Out of range value for INT: " + newValue);
                }
            } else if (type == DataType::FLOAT) {
                // If the type is FLOAT, try to convert to a float
                try {
                    value = std::to_string(std::stod(newValue));
                } catch (const std::invalid_argument& e) {
                    throw std::invalid_argument("Invalid value for FLOAT: " + newValue);
                } catch (const std::out_of_range& e) {
                    throw std::out_of_range("Out of range value for FLOAT: " + newValue);
                }
            } else {
                throw std::invalid_argument("Unknown DataType for set operation");
            }
            return true;  // Successfully updated the value
        } catch (const std::exception& e) {
            // Catch any exceptions and propagate them
            throw std::invalid_argument("Error setting value: " + std::string(e.what()));
        }
    }

    [[nodiscard]] DataType getType() const {return type;}

    [[nodiscard]] std::string getDisplayValue() const {
        if (type == DataType::TEXT) {
            return "'" + value + "'";
        }
        else if (type == DataType::INT) {
            return std::to_string(std::stoll(value));
        }
        else if (type == DataType::FLOAT) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << std::stod(value);

            return oss.str();
        }
        else {
            throw std::invalid_argument("Unknown DataType for get value");
        }
    }

    [[nodiscard]] std::string getRawValue() const { return value; }

private:
    DataType type;
    std::string value;

    // Helper function to handle value comparison
    bool compareValues(const Value& other) const {
        if (type == DataType::INT && other.type == DataType::INT) {
            return std::stoi(value) == std::stoi(other.value);
        } else if (type == DataType::FLOAT && other.type == DataType::FLOAT) {
            return std::stof(value) == std::stof(other.value);
        } else if (type == DataType::INT && other.type == DataType::FLOAT) {
            return std::stof(value) == std::stof(other.value);  // Promote INT to FLOAT
        } else if (type == DataType::FLOAT && other.type == DataType::INT) {
            return std::stof(value) == std::stoi(other.value);  // Promote INT to FLOAT
        }
        throw std::invalid_argument("Unsupported DataType for comparison");
    }

    bool compareLessThan(const Value& other) const {
        if (type == DataType::INT && other.type == DataType::INT) {
            return std::stoi(value) < std::stoi(other.value);
        } else if (type == DataType::FLOAT && other.type == DataType::FLOAT) {
            return std::stof(value) < std::stof(other.value);
        } else if (type == DataType::INT && other.type == DataType::FLOAT) {
            return std::stof(value) < std::stof(other.value);  // Promote INT to FLOAT
        } else if (type == DataType::FLOAT && other.type == DataType::INT) {
            return std::stof(value) < std::stoi(other.value);  // Promote INT to FLOAT
        }
        throw std::invalid_argument("Unsupported DataType for comparison");
    }

    bool compareGreaterThan(const Value& other) const {
        if (type == DataType::INT && other.type == DataType::INT) {
            return std::stoi(value) > std::stoi(other.value);
        } else if (type == DataType::FLOAT && other.type == DataType::FLOAT) {
            return std::stof(value) > std::stof(other.value);
        } else if (type == DataType::INT && other.type == DataType::FLOAT) {
            return std::stof(value) > std::stof(other.value);  // Promote INT to FLOAT
        } else if (type == DataType::FLOAT && other.type == DataType::INT) {
            return std::stof(value) > std::stoi(other.value);  // Promote INT to FLOAT
        }
        throw std::invalid_argument("Unsupported DataType for comparison");
    }
};



#endif //VALUE_H
