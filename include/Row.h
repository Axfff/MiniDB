//
// Created by zhaoj on 2024/11/12.
//

#ifndef ROW_H
#define ROW_H


#include <vector>
#include "Utils.h"
#include "Value.h"

class Row {
public:
    Row(const std::vector<DataType>& config, const std::vector<std::string>& rawValues): typeConfig(config) {
        if (typeConfig.size() != rawValues.size()) {
            std::cout << "Row::Row(): Wrong number of values provided" << std::endl;
            throw std::runtime_error("Row::Row(): Wrong number of values provided");
        }

        for (int i = 0; i < typeConfig.size(); i++) {
            try {values.emplace_back(typeConfig[i], rawValues[i]);}
            catch (const std::exception& e) {
                DataType& type = typeConfig[i];
                switch (type) {
                    case DataType::INT: {
                        values.emplace_back(typeConfig[i], "114514");
                        break;
                    }
                    case DataType::FLOAT: {
                        values.emplace_back(typeConfig[i], "114.514");
                        break;
                    }
                    case DataType::TEXT: {
                        values.emplace_back(typeConfig[i], "NONE");
                        break;
                    }
                }
            }
        }
    }

    [[nodiscard]] bool isFormatFit(const std::vector<DataType>& config) const {
        if (config.size() != values.size()) return false;
        for (int i = 0; i < typeConfig.size(); i++) {
            if (typeConfig[i] != config[i]) return false;
        }
        return true;
    }

    [[nodiscard]] std::vector<Value> getValues() const {return values;}

private:
    std::vector<DataType> typeConfig;
    std::vector<Value> values;
};



#endif //ROW_H
