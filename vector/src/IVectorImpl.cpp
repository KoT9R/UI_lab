//
// Created by Денис Попеску on 08/06/2020.
//
#include <cmath>
#include <string>
#include "../include/IVector.h"

namespace {
    class VectorImpl : public IVector {
    private:

        double *coords;
        ILogger *logger;
        size_t dim;

        inline double firstNorm() const {
            double res = 0;
            for (size_t i = 0; i < dim; ++i) {
                res += std::abs(coords[i]);
            }
            return res;
        }

        inline double secondNorm() const {
            double res = 0;
            for (size_t i = 0; i < dim; ++i) {
                res += coords[i] * coords[i];
            }
            return std::sqrt(res);
        }

        inline double infNorm() const {
            double res = coords[0];
            for (size_t i = 0; i < dim; ++i) {
                if (std::abs(coords[i]) > res)
                    res = std::abs(coords[i]);
            }
            return res;
        }

    public:
        VectorImpl(size_t dim, double *coords, ILogger *pLogger):
            coords(coords), logger(pLogger), dim(dim) {}

            ~VectorImpl() override {
                delete [] coords;
        }

        IVector * clone() const override {
            return IVector::createVector(dim, coords, logger);
        }

        double getCoord(size_t index) const override {
            if (index >= dim) {
                return std::numeric_limits<double>::quiet_NaN();
            }
            return coords[index];
        }

        double * getCoords() const {
            return coords;
        }

        RESULT_CODE setCoord(size_t index, double value) override {
            if (index >= dim) {
                return RESULT_CODE::WRONG_ARGUMENT;
            }
            coords[index] = value;
            return RESULT_CODE::SUCCESS;
        }

        size_t getDim() const override {
            return dim;
        }

        double norm(NORM norm) const override {
            double res = 0;
            switch (norm) {
                case NORM::NORM_1:
                    res = firstNorm();
                    break;
                case NORM::NORM_2:
                    res = secondNorm();
                    break;
                case NORM::NORM_INF:
                    res = infNorm();
                    break;
            }
            return res;
        }
    };
}

IVector * IVector::createVector(size_t dim, double *pData, ILogger *pLogger) {
    if (pData == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < dim; ++i) {
        if (std::isnan(pData[i])) {
            if (pLogger != nullptr) {
                pLogger->log((std::to_string(i) + " is Nan ").c_str(),
                        RESULT_CODE::NAN_VALUE);
            }
            return nullptr;
        }
    }

    size_t size = dim * sizeof(double ) + sizeof(VectorImpl);

    char *mem = new (std::nothrow) char[size];
    if (mem == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Couldn't create memory", RESULT_CODE::OUT_OF_MEMORY);
        return nullptr;
    }

    memcpy(mem + sizeof(VectorImpl), pData, sizeof(double) * dim);

    auto res = new (mem) VectorImpl(dim,
            reinterpret_cast<double *>(mem + sizeof(VectorImpl)),
            pLogger);
    return res;
}

IVector * IVector::add(const IVector *pOperand1, const IVector *pOperand2, ILogger *pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Operands is null", RESULT_CODE::WRONG_ARGUMENT);
        return nullptr;
    }

    if (pOperand1->getDim() != pOperand2->getDim()) {
        if (pLogger != nullptr)
            pLogger->log("", RESULT_CODE::WRONG_ARGUMENT);
        return nullptr;
    }

    auto res = createVector(pOperand1->getDim(),
            reinterpret_cast<VectorImpl const *>(pOperand1)->getCoords(),
            pLogger);

    if (res == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Fail create Vector", RESULT_CODE::OUT_OF_MEMORY);
        return nullptr;
    }

    for (size_t i = 0; i < res->getDim(); ++i) {
        if (res->setCoord(i, pOperand1->getCoord(i) + pOperand2->getCoord(i)) != RESULT_CODE::SUCCESS) {
            if (pLogger != nullptr)
                pLogger->log("Fail add", RESULT_CODE::CALCULATION_ERROR);
            delete res;
            return nullptr;
        }
    }
    return res;
}




IVector * IVector::sub(const IVector *pOperand1, const IVector *pOperand2, ILogger *pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Operands is null", RESULT_CODE::WRONG_ARGUMENT);
        return nullptr;
    }

    if (pOperand1->getDim() != pOperand2->getDim()) {
        if (pLogger != nullptr)
            pLogger->log("", RESULT_CODE::WRONG_ARGUMENT);
        return nullptr;
    }

    auto res = createVector(pOperand1->getDim(),
                            reinterpret_cast<VectorImpl const *>(pOperand1)->getCoords(),
                            pLogger);

    if (res == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Fail create Vector", RESULT_CODE::OUT_OF_MEMORY);
        return nullptr;
    }

    for (size_t i = 0; i < res->getDim(); ++i) {
        if (res->setCoord(i, pOperand1->getCoord(i) - pOperand2->getCoord(i)) != RESULT_CODE::SUCCESS) {
            if (pLogger != nullptr)
                pLogger->log("Fail add", RESULT_CODE::CALCULATION_ERROR);
            delete res;
            return nullptr;
        }
    }
    return res;
}

IVector * IVector::mul(const IVector *pOperand1, double scaleParam, ILogger *pLogger)  {
    if (pOperand1 == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Operands is null", RESULT_CODE::WRONG_ARGUMENT);
        return nullptr;
    }

    auto res = createVector(pOperand1->getDim(),
                            reinterpret_cast<VectorImpl const *>(pOperand1)->getCoords(),
                            pLogger);

    if (res == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Fail create Vector", RESULT_CODE::OUT_OF_MEMORY);
        return nullptr;
    }

    for (size_t i = 0; i < res->getDim(); ++i) {
        if (res->setCoord(i, pOperand1->getCoord(i) * scaleParam) != RESULT_CODE::SUCCESS) {
            if (pLogger != nullptr)
                pLogger->log("Fail add", RESULT_CODE::CALCULATION_ERROR);
            delete res;
            return nullptr;
        }
    }
    return res;
}

double IVector::mul(const IVector *pOperand1, const IVector *pOperand2, ILogger *pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Operands is null", RESULT_CODE::WRONG_ARGUMENT);
        return std::numeric_limits<double>::quiet_NaN();
    }

    if (pOperand1->getDim() != pOperand2->getDim()) {
        if (pLogger != nullptr)
            pLogger->log("", RESULT_CODE::WRONG_ARGUMENT);
        return std::numeric_limits<double>::quiet_NaN();
    }

    double res = 0;

    for (size_t i = 0; i < pOperand1->getDim(); ++i) {
        res += pOperand1->getCoord(i) * pOperand2->getCoord(i);
    }
    return res;
}

RESULT_CODE IVector::equals(const IVector *pOperand1, const IVector *pOperand2, NORM norm, double tolerance, bool *result, ILogger *pLogger) {
    if (pOperand1 == nullptr || pOperand2 == nullptr) {
        if (pLogger != nullptr)
            pLogger->log("Operands is null", RESULT_CODE::WRONG_ARGUMENT);
        return RESULT_CODE::NAN_VALUE;
    }

    if (pOperand1->getDim() != pOperand2->getDim()) {
        if (pLogger != nullptr)
            pLogger->log("", RESULT_CODE::WRONG_ARGUMENT);
        return RESULT_CODE::WRONG_DIM;
    }

    auto diff = sub(pOperand1, pOperand2, pLogger);

    if (diff == nullptr) {
        return RESULT_CODE::BAD_REFERENCE;
    }

    *result = diff->norm(norm) < tolerance;
    return RESULT_CODE::SUCCESS;
}


