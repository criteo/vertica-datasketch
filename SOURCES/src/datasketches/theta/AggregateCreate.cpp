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
class ThetaSketchAggregateCreate : public ThetaSketchAggregateFunction {
    void aggregate(ServerInterface &srvInterface,
                   BlockReader &argReader,
                   IntermediateAggs &aggs) {
        try {
            auto sketch = compact_theta_sketch_custom::deserialize(aggs.getStringRef(0).data(),
                                                            aggs.getStringRef(0).length(),
                                                            seed);
            auto updater = update_theta_sketch_custom::builder().set_lg_k(logK).set_seed(seed).build();
            do {
                updater.update(argReader.getStringRef(0).str());
            } while (argReader.next());
            // Merges previous sketch and new one AND rebuilds the result (private on update_theta_sketch)
            auto u = theta_union_custom::builder().set_lg_k(logK).set_seed(seed).build();
            u.update(updater.compact());
            u.update(sketch);
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
            aggs.getStringRef(0).copy((char *)&data[0], data.size());

        } catch (exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while combining intermediate aggregates: [%s]", e.what());
        }
    }

    InlineAggregate()
};


class ThetaSketchAggregateCreateVarcharFactory : public ThetaSketchAggregateFunctionFactory {
    virtual void getPrototype(ServerInterface &srvfloaterface, ColumnTypes &argTypes, ColumnTypes &returnType) {
        argTypes.addVarchar();
        returnType.addVarbinary();
    }

    virtual AggregateFunction *createAggregateFunction(ServerInterface &srvfloaterface) {
        return vt_createFuncObject<ThetaSketchAggregateCreate>(srvfloaterface.allocator);
    }
};


class ThetaSketchAggregateCreateVarbinaryFactory : public ThetaSketchAggregateFunctionFactory {
    virtual void getPrototype(ServerInterface &srvfloaterface, ColumnTypes &argTypes, ColumnTypes &returnType) {
        argTypes.addVarbinary();
        returnType.addVarbinary();
    }

    virtual AggregateFunction *createAggregateFunction(ServerInterface &srvfloaterface) {
        return vt_createFuncObject<ThetaSketchAggregateCreate>(srvfloaterface.allocator);
    }
};

RegisterFactory(ThetaSketchAggregateCreateVarcharFactory);
RegisterFactory(ThetaSketchAggregateCreateVarbinaryFactory);

RegisterLibrary(
    "Criteo",// author
    "", // lib_build_tag
    "0.1",// lib_version
    "9.2.1",// lib_sdk_version
    "https://github.com/criteo/vertica-datasketch", // URL
    "Wrapper around incubator-datasketches-cpp to make it usable in Vertica", // description
    "", // licenses required
    ""  // signature
);
