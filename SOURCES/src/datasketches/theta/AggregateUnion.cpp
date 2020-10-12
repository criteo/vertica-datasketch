#include "Vertica.h"
#include <iostream>
#include <theta_sketch.hpp>
#include <theta_union.hpp>
#include "../../../include/datasketches/theta/theta_common.hpp"

using namespace Vertica;
using namespace std;


/**
 * User Defined Aggregate Function concatenate that takes in strings and concatenates
 * them together. Right now, the max length of the resulting string is ten times the
 * maximum length of the input string.
 */
class ThetaSketchAggregateUnion : public ThetaSketchAggregateFunction {
    void aggregate(ServerInterface &srvInterface,
                   BlockReader &argReader,
                   IntermediateAggs &aggs) {
        try {
            auto u = theta_union_custom::builder()
                    .set_lg_k(logK)
                    .set_seed(seed)
                    .build();
            auto sketch = compact_theta_sketch_custom::deserialize(aggs.getStringRef(0).data(),
                                                            aggs.getStringRef(0).length(),
                                                            seed);
            u.update(sketch);
            do {
                sketch = compact_theta_sketch_custom::deserialize(argReader.getStringRef(0).data(),
                                                           argReader.getStringRef(0).length(),
                                                           seed);
                u.update(sketch);
            } while (argReader.next());
            auto data = u.get_result().serialize();
            aggs.getStringRef(0).copy((char *) &data[0], data.size());

        } catch (exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while processing aggregate: [%s]", e.what());
        }
    }

    virtual void combine(ServerInterface &srvInterface,
                         IntermediateAggs &aggs,
                         MultipleIntermediateAggs &aggsOther) override {
        try {
            auto u = theta_union_custom::builder()
                    .set_lg_k(logK)
                    .set_seed(seed)
                    .build();

            auto sketch = compact_theta_sketch_custom::deserialize(aggs.getStringRef(0).data(),
                                                            aggs.getStringRef(0).length(),
                                                            seed);
            u.update(sketch);

            do {
                sketch = compact_theta_sketch_custom::deserialize(aggsOther.getStringRef(0).data(),
                                                           aggsOther.getStringRef(0).length(),
                                                           seed);
                u.update(sketch);
            } while (aggsOther.next());

            auto data = u.get_result().serialize();
            aggs.getStringRef(0).copy((char *) &data[0], data.size());

        } catch (exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while combining intermediate aggregates: [%s]", e.what());
        }
    }

    InlineAggregate()
};


class ThetaSketchAggregateUnionFactory : public ThetaSketchAggregateFunctionFactory {
    virtual void getPrototype(ServerInterface &srvfloaterface, ColumnTypes &argTypes, ColumnTypes &returnType) {
        argTypes.addLongVarbinary();
        returnType.addLongVarbinary();
    }

    virtual AggregateFunction *createAggregateFunction(ServerInterface &srvfloaterface) {
        return vt_createFuncObject<ThetaSketchAggregateUnion>(srvfloaterface.allocator);
    }
};

RegisterFactory(ThetaSketchAggregateUnionFactory);
