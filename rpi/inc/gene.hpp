#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace audiogen {
    enum class ExpressionActivates {
        OnBar,
        OverBar
    };
    struct Expression {
        uint32_t min;
        uint32_t max;
        uint32_t current;
        ExpressionActivates activates;
    };
    typedef std::string GeneName;

    class Gene {
        GeneName mName;
        Expression mExpression;
       public:
        Gene();
        Gene(GeneName name, std::map<std::string, std::string> expression);
        ~Gene() {};
        GeneName name();
        Expression expression();
    };
}  // audiogen
