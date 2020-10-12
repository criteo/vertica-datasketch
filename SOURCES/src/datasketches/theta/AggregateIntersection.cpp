#include "Vertica.h"
#include <iostream>
#include <theta_sketch.hpp>
#include <theta_intersection.hpp>
#include "../../../include/datasketches/theta/theta_common.hpp"

using namespace Vertica;
using namespace std;


/**
 * User Defined Aggregate Function concatenate that takes in strings and concatenates
 * them together. Right now, the max length of the resulting string is ten times the
 * maximum length of the input string.
 */
class ThetaSketchAggregateIntersection : public ThetaSketchAggregateFunction {
    virtual void initAggregate(ServerInterface &srvInterface, IntermediateAggs &aggs) {
        try {
            vbool &initialized = aggs.getBoolRef(1);
            initialized = false;

            auto u = theta_union_custom::builder()
                    .set_lg_k(logK)
                    .set_seed(seed)
                    .build();
            auto data = u.get_result().serialize(); // provides compact & rebuild sketch <=> min size
            aggs.getStringRef(0).copy((char *) &data[0], data.size());
        } catch (exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while initializing intermediate aggregates: [%s]", e.what());
        }
    }

    void aggregate(ServerInterface &srvInterface,
                   BlockReader &argReader,
                   IntermediateAggs &aggs) {
        try {
            auto intersection = theta_intersection_custom(seed);
            vbool &initialized = aggs.getBoolRef(1);

            if (initialized) {
                auto aggSketch = compact_theta_sketch_custom::deserialize(aggs.getStringRef(0).data(),
                                                                   aggs.getStringRef(0).length(),
                                                                   seed);
                intersection.update(aggSketch);
            }

            do {
                auto sketch = compact_theta_sketch_custom::deserialize(argReader.getStringRef(0).data(),
                                                                argReader.getStringRef(0).length(),
                                                                seed);
                intersection.update(sketch);
                initialized = true;
            } while (argReader.next());

            auto data = intersection.get_result().serialize();
            aggs.getStringRef(0).copy((char *) &data[0], data.size());
        } catch (exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while processing aggregate: [%s]", e.what());
        }
    }

    virtual void combine(ServerInterface &srvInterface,
                         IntermediateAggs &aggs,
                         MultipleIntermediateAggs &aggsOther) {
        try {
            auto intersection = theta_intersection_custom(seed);
            // Unsure if all aggregations here must have been used at least once or not.
            vbool &initialized = aggs.getBoolRef(1);
            if (initialized) {
                auto initSketch = compact_theta_sketch_custom::deserialize(aggs.getStringRef(0).data(),
                                                                    aggs.getStringRef(0).length(),
                                                                    seed);
                intersection.update(initSketch);
            }

            do {
                vbool otherInitialized = aggsOther.getBoolRef(1);
                if (otherInitialized) {
                    auto sketch = compact_theta_sketch_custom::deserialize(aggsOther.getStringRef(0).data(),
                                                                    aggsOther.getStringRef(0).length(),
                                                                    seed);
                    intersection.update(sketch);
                    initialized = true;
                }
            } while (aggsOther.next());

            if (intersection.has_result()) { // Overwrite empty sketch only if necessary
                auto data = intersection.get_result().serialize();
                aggs.getStringRef(0).copy((char *) &data[0], data.size());
            }
        } catch (exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while combining intermediate aggregates: [%s]", e.what());
        }
    }

    InlineAggregate()
};


class ThetaSketchAggregateIntersectionFactory : public ThetaSketchAggregateFunctionFactory {
    virtual void getIntermediateTypes(ServerInterface &srvInterface,
                                      const SizedColumnTypes &inputTypes,
                                      SizedColumnTypes &intermediateTypeMetaData) {
        uint8_t logK = readLogK(srvInterface);
        intermediateTypeMetaData.addLongVarbinary(quickSelectSketchMinSize(logK));
        intermediateTypeMetaData.addBool();
    }

    virtual void getPrototype(ServerInterface &srvfloaterface, ColumnTypes &argTypes, ColumnTypes &returnType) {
        argTypes.addLongVarbinary();
        returnType.addLongVarbinary();
    }

    virtual AggregateFunction *createAggregateFunction(ServerInterface &srvfloaterface) {
        return vt_createFuncObject<ThetaSketchAggregateIntersection>(srvfloaterface.allocator);
    }
};

RegisterFactory(ThetaSketchAggregateIntersectionFactory);

