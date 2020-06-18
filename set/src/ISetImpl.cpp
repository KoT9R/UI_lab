//
// Created by Денис Попеску on 14/06/2020.
//
#include <vector>
#include "../interfaces/ISet.h"

namespace {
    class SetImpl : public ISet {
    private:
        std::vector<IVector const *> vectors;
        ILogger * logger;

        RESULT_CODE log(char const* msg, RESULT_CODE err) const {
            if (logger != nullptr) {
                logger->log(msg, err);
            }
            return err;
        }

    protected:
        SetImpl() {
            logger = ILogger::createLogger(this);
        }

    public:
        SetImpl(ILogger *logger) : logger (logger) {};

        ~SetImpl() override {
            clear();
        }

        RESULT_CODE insert(const IVector *pVector, IVector::NORM norm, double tolerance) override {
            if (pVector == nullptr) {
                return log("Set(insert)", RESULT_CODE::BAD_REFERENCE);
            }

            if (!vectors.empty()) {
                if (pVector->getDim() != getDim()) {
                    return log("set(insert)", RESULT_CODE::WRONG_DIM);
                }
            }

            IVector * differenceBetweenItems;

            for (auto vector : vectors) {
                differenceBetweenItems = IVector::sub(pVector, vector, logger);

                if (differenceBetweenItems == nullptr)
                    continue;

                if (differenceBetweenItems->norm(norm) < tolerance) {
                    return log("Set(insert)", RESULT_CODE::MULTIPLE_DEFINITION);
                }
            }

            vectors.push_back(pVector->clone());
            return RESULT_CODE::SUCCESS;
        }

        RESULT_CODE get(IVector *&pVector, size_t index) const override {
            if (index >= vectors.size()) {
                return log("set(get)", RESULT_CODE::OUT_OF_BOUNDS);
            }

            pVector = const_cast<IVector *>(vectors[index]);
            return RESULT_CODE::SUCCESS;
        }

        RESULT_CODE get(IVector *&pVector, const IVector *pSample, IVector::NORM norm, double tolerance) const override {
            if (pSample == nullptr) {
                return log("set(get)", RESULT_CODE::NOT_FOUND);
            }

            IVector *differenceBetweenItems = nullptr;

            for (auto vector: vectors) {
                differenceBetweenItems = IVector::sub(pSample, vector, logger);
                if (differenceBetweenItems == nullptr) {
                    continue;
                }
                if (differenceBetweenItems->norm(norm) < tolerance) {
                    pVector = const_cast<IVector *>(vector);
                    delete differenceBetweenItems;
                    return log("set(get)", RESULT_CODE::SUCCESS);
                }
            }

            delete differenceBetweenItems;
            return log("set(get)", RESULT_CODE::NOT_FOUND);
        }

        size_t getDim() const override {
            if (vectors.empty()) {
                return 0;
            } else {
                return vectors[0]->getDim();
            }
        }

        size_t getSize() const override {
            return vectors.size();
        }

        void clear() override {
            for (auto &vector : vectors) {
                delete vector;
            }
            vectors.clear();
        }

        RESULT_CODE erase(const IVector *pSample, IVector::NORM norm, double tolerance) override {
            if (pSample == nullptr) {
                return log("set(erase)", RESULT_CODE::BAD_REFERENCE);
            }

            size_t indexOfVector = 0;
            IVector *differenceBetweenItems = nullptr;

            for (auto vector : vectors) {
                differenceBetweenItems = IVector::sub(pSample, vector, logger);
                  if (differenceBetweenItems == nullptr) {
                      continue;
                  }
                  if (differenceBetweenItems->norm(norm) < tolerance) {
                      delete differenceBetweenItems;
                      delete *(vectors.begin() + indexOfVector);
                      vectors.erase(vectors.begin() + indexOfVector);
                      return log("set(erase)", RESULT_CODE::SUCCESS);
                  }
                  ++indexOfVector;
            }
            delete differenceBetweenItems;
            return log("set(erase)", RESULT_CODE::NOT_FOUND);
        }

        RESULT_CODE erase(size_t index) override {
            if (index < 0) {
                return log("set(erase)", RESULT_CODE::BAD_REFERENCE);
            }

            if (index >= vectors.size()) {
                return log("set(erase)", RESULT_CODE::NOT_FOUND);
            }

            auto elemToDel = vectors.begin() + index;
            if (*elemToDel != nullptr) {
                delete *elemToDel;
            }
            vectors.erase(elemToDel);
            return RESULT_CODE::SUCCESS;
        }

        ISet * clone() const override {
            auto *newSet = new SetImpl(logger);
            if (newSet == nullptr) {
                log("set(clone)", RESULT_CODE::OUT_OF_MEMORY);
                return nullptr;
            }
            auto size = vectors.size();
            newSet->vectors.resize(size);
            for (size_t i = 0; i < size; ++i) {
                newSet->vectors[i] = vectors[i]->clone();
            }
            return newSet;
        }
    };
}

ISet * ISet::createSet(ILogger *pLogger) {
    ISet *set = new (std::nothrow) SetImpl(pLogger);
    if (set == nullptr) {
        pLogger->log("set(clone)", RESULT_CODE::OUT_OF_MEMORY);
    }
    return set;
}

ISet * ISet::add(const ISet *pOperand1, const ISet *pOperand2, IVector::NORM norm, double tolerance, ILogger *pLogger) {
    if (pOperand1 == nullptr && pOperand2 == nullptr) {
        if (pLogger != nullptr) {
            pLogger->log("set(add)", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }

    if (pOperand1 == nullptr) {
        return pOperand2->clone();
    }

    if (pOperand2 == nullptr) {
        return pOperand1->clone();
    }

    auto ans = pOperand1->clone();
    IVector * bufItem = nullptr;

    for (size_t i = 0; i < pOperand2->getSize(); ++i) {
        pOperand2->get(bufItem, i);
        ans->insert(bufItem, norm, tolerance);
    }

    return ans;
}

ISet * ISet::intersect(const ISet *pOperand1, const ISet *pOperand2, IVector::NORM norm, double tolerance, ILogger *pLogger) {
    if (pOperand1 == nullptr && pOperand2 == nullptr) {
        if (pLogger != nullptr) {
            pLogger->log("set(intersect)", RESULT_CODE::BAD_REFERENCE);
        }
        return nullptr;
    }

    ISet * intersection = ISet::createSet(pLogger);
    
    if (intersection == nullptr) {
        pLogger->log("set(intersection)", RESULT_CODE::OUT_OF_MEMORY);
        return nullptr;
    }
    
    IVector *itemFirst;
    IVector *itemSecond;
    
    for (size_t i = 0; i < pOperand1->getSize(); ++i) {
        pOperand2->get(itemFirst, i);
        if (pOperand1->get(itemSecond, itemFirst, norm, tolerance) == RESULT_CODE::SUCCESS) {
            intersection->insert(itemFirst, norm, tolerance);
        }
    }

    return intersection;
}

