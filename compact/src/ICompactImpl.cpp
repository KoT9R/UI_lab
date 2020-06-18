//
// Created by Денис Попеску on 14/06/2020.
//
#include <cmath>
#include <new>
#include <vector>
#include <utility>
#include "../include/ICompact.h"
#include "../include/IVector.h"

using pair   = std::pair<size_t, size_t>;
using vector = std::vector<pair>;
using order  = std::vector<size_t>;

namespace {
#define TOLERANCE 10E-6

    bool Less(IVector const *lesser, IVector const *larger) {
        if (lesser->getDim() != larger->getDim()) {
            return false;
        }

        for (size_t i = 0; i < lesser->getDim(); ++i) {
            if (lesser->getCoord(i) > larger->getCoord(i)) {
                return false;
            }
        }
        return true;
    }

    bool CheckData(ICompact const * const first, ICompact const * const second) {
        if (first == nullptr || second == nullptr) {
            return false;
        }
        return first->getDim() == second->getDim();
    }

    bool CheckData(IVector const * const first, IVector const * const second) {
        if (first == nullptr || second == nullptr) {
            return false;
        }
        return first->getDim() == second->getDim();
    }

    IVector const * Min(IVector const *first, IVector const *second ) {
        if (Less(first, second)) {
            return first;
        }
        return second;
    }

    IVector const * Max(IVector const *first, IVector const *second ) {
        return Min(second, first);
    }

    class CompactImpl : public ICompact {

    private:
        ILogger *logger;
        IVector *left;
        IVector *right;
        size_t dim;

        RESULT_CODE log (char const * msg, RESULT_CODE code) const {
            if (logger != nullptr) {
                logger->log(msg, code);
            }
            return code;
        }

    public:

        CompactImpl( IVector *left, IVector *right, ILogger *logger ):
                left(left->clone()),
                right(right->clone()),
                dim(left->getDim()),
                logger(logger) {}

        IVector * getBegin() const override {
            return left;
        }

        IVector * getEnd() const override {
            return right;
        }

        RESULT_CODE isContains(const IVector *const vec, bool &result) const override {
            if (vec == nullptr) {
                return log("compact(is contains)",RESULT_CODE::BAD_REFERENCE);
            }

            if (vec->getDim() != getDim()) {
                return log("compact(is contains)",RESULT_CODE::WRONG_DIM);
            }

            result = Less(left, vec) && Less(vec, right);
            return RESULT_CODE::SUCCESS;
        }

        RESULT_CODE isSubSet(const ICompact *const other, bool &result) const override {
            if (!CheckData(this, other)) {
                log("compact(is subset)", RESULT_CODE::BAD_REFERENCE);
            }

            bool otherIsContains = false;
            auto resultCode = isContains(other->getBegin(), otherIsContains);

            if (resultCode != RESULT_CODE::SUCCESS) {
                log("compact(is subset)", RESULT_CODE::BAD_REFERENCE);
            }

            if (otherIsContains) {
                resultCode = isContains(other->getEnd(), otherIsContains);
                if (resultCode != RESULT_CODE::SUCCESS) {
                    log("compact(is subset)", RESULT_CODE::BAD_REFERENCE);
                }
            }

            result = otherIsContains;
            return log("compact(is subset)", RESULT_CODE::SUCCESS);
        }

        RESULT_CODE isIntersects(const ICompact *const other, bool &result) const override {
            if (!CheckData(this, other)) {
                log("compact(is intersects)", RESULT_CODE::BAD_REFERENCE);
            }

            auto l = Max(left, other->getBegin());
            auto r = Min(right, other->getEnd());

            result = Less(l, r);
            return log("compact(is intersects)", RESULT_CODE::SUCCESS);
        }

        size_t getDim() const override {
            return dim;
        }

        ICompact * clone() const override {
            auto newCompact = new (std::nothrow) CompactImpl(left, right, logger);
            if (newCompact == nullptr) {
                log("compact(clone)", RESULT_CODE::OUT_OF_MEMORY);
            }
            return newCompact;
        }

        iterator * begin(const IVector *const step = nullptr) override {
            auto iter = new (std::nothrow) iterator(this, step, logger);
            if (iter == nullptr) {
                log("iterator(begin)", RESULT_CODE::OUT_OF_MEMORY);
            }
            return iter;
        }

        iterator * end(const IVector *const step = nullptr) override {
            auto iter = new (std::nothrow) iterator(this, step, logger, true);
            if (iter == nullptr) {
                log("iterator(end)", RESULT_CODE::OUT_OF_MEMORY);
            }
            return iter;
        }

        ~CompactImpl() override  {
            delete left;
            delete right;
        }

    class iterator : public ICompact::iterator {
    private:
            ICompact const *compact;
            IVector const *step;
            IVector *current;
            IVector *dir;
            ILogger *logger;
            order indexes;
            bool isBegin;

        RESULT_CODE log (char const * msg, RESULT_CODE code) const {
            if (logger != nullptr) {
                logger->log(msg, code);
            }
            return code;
        }

            void Order() {
                auto dim = dir->getDim();
                vector vectorWithIndx (dim);
                for (size_t i = 0; i < dim; ++i) {
                    pair pos (static_cast<size_t>(std::round(dir->getCoord(i))), i);
                    vectorWithIndx[i] = pos;
                }

                std::sort(vectorWithIndx.begin(), vectorWithIndx.end(),
                          [](pair &first, pair &second) {
                    return first.first < second.first;
                } );

                for (size_t i = 0; i < dim; ++i) {
                    indexes[i] = vectorWithIndx[i].second;
                }
            }

            bool ValidationDirection(IVector const * newDir, size_t indx) {
            auto newDim = newDir->getDim();
            auto coord  = dir->getCoord(indx);
            if (coord < 0 || coord > newDim - 1) {
                return false;
            }
            for (size_t i = 0; i < newDim; ++i) {
                if (i == indx) {
                    continue;
                }
                if (std::abs(newDir->getCoord(indx) - coord) < TOLERANCE) {
                    return false;
                }
            }
                return true;
        }

    public:
            iterator(ICompact const *compact, IVector const *step, ILogger *logger, bool isBegin = true) :
                    compact(compact->clone()),
                    logger(logger),
                    step(step->clone()),
                    current(isBegin ? compact->getBegin()->clone() : compact->getEnd()->clone()),
                    isBegin(isBegin) {
                auto dim = compact->getDim();
                indexes.resize(dim);
                auto * data = new double[dim];
                for (size_t i = 0; i < dim; ++i) {
                    data[i] = i;
                    indexes[i] = i;
                }
                dir = IVector::createVector(dim, data, logger);
                delete []data;
            }

            RESULT_CODE doStep() override {
                size_t curAxis = 0;

                auto begin = compact->getBegin();
                auto end   = compact->getEnd();
                auto dim   = compact->getDim();

                auto *currentClone = current->clone();

                auto curValue = 0;
                auto endValue = 0;
                auto beginVal = 0;
                size_t i = 0;

                for (i = 0; i < indexes.size(); ++i) {
                    if (isBegin) {
                        curValue = currentClone->getCoord(indexes[i]);
                        endValue = end->getCoord(indexes[i]);
                        if (std::abs(curValue - endValue) < TOLERANCE) {
                            continue;
                        } else {
                            curAxis = i;
                            break;
                        }
                    } else {
                        curValue = currentClone->getCoord(indexes[i]);
                        beginVal = begin->getCoord(indexes[i]);
                        if (std::abs(curValue - beginVal) < TOLERANCE) {
                            continue;
                        } else {
                            curAxis = i;
                            break;
                        }
                    }
                }

                if (i == indexes.size()) {
                    delete currentClone;
                    return RESULT_CODE::OUT_OF_BOUNDS;
                }

                curValue = currentClone->getCoord(indexes[curAxis]);

                if (isBegin) {
                    currentClone->setCoord(indexes[curAxis], curValue + step->getCoord(indexes[curAxis]));
                } else {
                    currentClone->setCoord(indexes[curAxis], curValue - step->getCoord(indexes[curAxis]));
                }

                bool contains;
                auto vecIsContains = compact->isContains(currentClone, contains);

                if (vecIsContains != RESULT_CODE::SUCCESS) {
                    delete currentClone;
                    return RESULT_CODE::OUT_OF_BOUNDS;
                } else {
                    for (i = 0; i < dim; ++i) {
                        current->setCoord(i, currentClone->getCoord(i));
                    }
                }

                return RESULT_CODE::SUCCESS;
            }

            IVector * getPoint() const override {
                return current;
            }

            RESULT_CODE setDirection(const IVector *const newDir) override {
                if (newDir->getDim() != compact->getDim()) {
                    return log("iterator(set direction)", RESULT_CODE::WRONG_DIM);
                }

                auto dim = newDir->getDim();
                for (size_t i = 0; i < dim; ++i) {
                    if (!ValidationDirection(newDir, i)) {
                        return log("iterator(set direction)", RESULT_CODE::WRONG_ARGUMENT);
                    }
                }

                for (size_t i = 0; i < compact->getDim(); ++i) {
                    auto coord = newDir->getCoord(i);
                    if (std::abs(coord - std::round(coord)) > TOLERANCE) {
                        return log("iterator(set direction)", RESULT_CODE::WRONG_ARGUMENT);
                    }
                    dir->setCoord(i, newDir->getCoord(i));
                }
                Order();
                return RESULT_CODE::SUCCESS;
            }

        ~iterator() override
        {
            delete dir;
            delete step;
            delete current;
            delete compact;
            indexes.clear();
        }

        };
    };
}

RESULT_CODE log (char const * msg, RESULT_CODE code, ILogger * const logger) {
    if (logger != nullptr) {
        logger->log(msg, code);
    }
    return code;
}

ICompact * ICompact::createCompact(const IVector *const begin, const IVector *const end, ILogger *logger) {
    if (!CheckData(begin, end)) {
        log("Create compact", RESULT_CODE::BAD_REFERENCE, logger);
        return nullptr;
    }

    auto begLessEnd = Less(begin, end);
    auto endLessBeg = Less(end, begin);

    if (!begLessEnd && !endLessBeg) {
        log("Create compact", RESULT_CODE::WRONG_ARGUMENT, logger);
    }

    CompactImpl* newCompact = nullptr;

    if (begLessEnd) {
        newCompact = new (std::nothrow) CompactImpl(const_cast<IVector *>(begin),
                                                            const_cast<IVector *>(end),
                                                            logger);

    } else {
        newCompact = new (std::nothrow) CompactImpl(const_cast<IVector *>(end),
                                                         const_cast<IVector *>(begin),
                                                         logger);
    }

    if (newCompact == nullptr) {
        log("Create compact ", RESULT_CODE::OUT_OF_MEMORY, logger);
    }

    return newCompact;
}

ICompact * ICompact::intersection(const ICompact *const left, const ICompact *const right, ILogger *logger) {
    if (!CheckData(left, right)) {
        log("Intersection", RESULT_CODE::BAD_REFERENCE, logger);
        return nullptr;
    }

    bool in;
    auto resCode = left->isIntersects(right, in);

    if (resCode != RESULT_CODE::SUCCESS) {
        log("Compact intersection", RESULT_CODE::BAD_REFERENCE, logger);
        return nullptr;
    }

    if (in) {
        auto newLeft = Max(left->getBegin(), right->getBegin());
        auto newRight = Min(left->getEnd(), right->getEnd());
        auto newCompact = createCompact(newLeft, newRight, logger);
        if (newCompact == nullptr) {
            log("Compact intersection", RESULT_CODE::OUT_OF_MEMORY, logger);
        }
        return newCompact;
    } else {
        log("Compact intersection", RESULT_CODE::WRONG_ARGUMENT, logger);
        return nullptr;
    }
}

bool CompactInCompact(ICompact const * const left,ICompact const * const right) {
    return Less(left->getBegin(), right->getBegin()) &&
            Less(right->getEnd(), left->getEnd());
}

bool CompactsIsConnected(ICompact const * const left,ICompact const * const right) {
    return Less(left->getBegin(), right->getEnd()) || Less(right->getBegin(), left->getEnd());
}

bool CheckParallel(IVector const * const vector, int &indxAxis) {
    int count = 0;
    double norm = vector->norm(IVector::NORM::NORM_INF);
    for (size_t i = 0; i < vector->getDim() && count <= 1; ++i) {
        if (std::abs(vector->getCoord(i) / norm) > TOLERANCE) {
            indxAxis = i;
            ++count;
        }
    }
    return count <= 1;
}

ICompact * ICompact::add(const ICompact *const left, const ICompact *const right, ILogger *logger) {
    if (!CheckData(left, right)) {
        log("add", RESULT_CODE::BAD_REFERENCE, logger);
        return nullptr;
    }

    if (!CompactsIsConnected(left, right)) {
        log("add: not have intersection", RESULT_CODE::BAD_REFERENCE, logger);
    }

    if (CompactInCompact(left, right)) {
        return left->clone();
    }
    if (CompactInCompact(right, left)) {
        return right->clone();
    }

    auto beg = IVector::sub(left->getBegin(), right->getBegin(), logger);

    if (beg == nullptr) {
        log("add: beg", RESULT_CODE::WRONG_ARGUMENT, logger);
    }

    int indxAxisBeg;
    int indxAxisEnd;

    if (CheckParallel(beg, indxAxisBeg)) {
        auto vend = IVector::sub(left->getEnd(), right->getEnd(), logger);
        if (vend == nullptr) {
            log("add: end", RESULT_CODE::WRONG_ARGUMENT, logger);
            return nullptr;
        }

        if (CheckParallel(vend, indxAxisEnd)) {
            if (indxAxisBeg == indxAxisEnd) {
                auto newCompact = createCompact(Min(left->getBegin(), right->getBegin()),
                                                Max(left->getEnd(), right->getEnd()), logger);
                if (newCompact == nullptr) {
                    log("add", RESULT_CODE::OUT_OF_MEMORY, logger);
                }
                return newCompact;
            }
        }
    }
    return nullptr;
}

ICompact * ICompact::makeConvex(const ICompact *const left, const ICompact *const right, ILogger *logger) {
    if (!CheckData(left, right) || logger == nullptr) {
        log("makeConvex", RESULT_CODE::BAD_REFERENCE, logger);
    }

    auto newCompact = createCompact(Min(left->getBegin(), right->getBegin()),
                                    Max(left->getEnd(), right->getEnd()), logger);
    if (newCompact == nullptr) {
        log("add", RESULT_CODE::OUT_OF_MEMORY, logger);
    }
    return newCompact;

}