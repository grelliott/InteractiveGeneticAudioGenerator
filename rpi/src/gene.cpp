#include "gene.hpp"

namespace audiogen {

Gene::Gene() {}

Gene::Gene(GeneName name, std::map<std::string, std::string> expression) :
    mName(name) {
        mExpression = {};
        mExpression.min = std::stoi(expression["min"]);
        mExpression.max = std::stoi(expression["min"]);
        mExpression.current = std::stoi(expression["current"]);
        if (expression["activates"] == "OnBar") {
            mExpression.activates = ExpressionActivates::OnBar;
        } else if (expression["activates"] == "OverBar") {
            mExpression.activates = ExpressionActivates::OverBar;
        } else {
            mExpression.activates = ExpressionActivates::OnBar;
        }
    }

GeneName Gene::name() {
    return mName;
}

Expression Gene::expression() {
    return mExpression;
}

}  // end audiogen
