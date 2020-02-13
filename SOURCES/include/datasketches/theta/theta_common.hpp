#ifndef COM_CRITEO_MOAB_DATASKETCHES_VERTICA_H
#define COM_CRITEO_MOAB_DATASKETCHES_VERTICA_H

#include <Vertica.h>
#include <cstdint>
#include "theta_const.hpp"
#include "theta_def.hpp"

using namespace Vertica;
using namespace std;

uint8_t readLogK(ServerInterface &serverInterface);

uint64_t readSeed(ServerInterface &serverInterface);

uint32_t quickSelectSketchMinSize(uint8_t logK);

uint32_t quickSelectSketchMaxSize(uint8_t logK);


class ThetaSketchScalarFunctionFactory : public ScalarFunctionFactory {
    virtual void getReturnType(ServerInterface &srvfloaterface,
                               const SizedColumnTypes &inputTypes,
                               SizedColumnTypes &outputTypes) {
        uint8_t logK = readLogK(srvfloaterface);
        outputTypes.addVarbinary(quickSelectSketchMinSize(logK));
    }

    virtual void getParameterType(ServerInterface &srvInterface,
                                  SizedColumnTypes &parameterTypes) {
        // Unfortunately it cannot be forced in the intersection...
        // So we have to trust the user to give the maximum logK used of the input sketches.
        SizedColumnTypes::Properties logNominalProps;
        logNominalProps.required = false;
        logNominalProps.canBeNull = false;
        logNominalProps.comment = "Log Nominal value.";
        parameterTypes.addInt(DATASKETCHES_LOG_NOMINAL_VALUE_PARAMETER_NAME, logNominalProps);

        SizedColumnTypes::Properties seedProps;
        seedProps.required = false;
        seedProps.canBeNull = false;
        seedProps.comment = "Seed value";
        parameterTypes.addInt(DATASKETCHES_SEED_PARAMETER_NAME, seedProps);
    }
};

class ThetaSketchAggregateFunctionFactory : public AggregateFunctionFactory {
    virtual void getIntermediateTypes(ServerInterface &srvInterface,
                                      const SizedColumnTypes &inputTypes,
                                      SizedColumnTypes &intermediateTypeMetaData) {
        uint8_t logK = readLogK(srvInterface);
        intermediateTypeMetaData.addVarbinary(quickSelectSketchMinSize(logK));
    }

    virtual void getReturnType(ServerInterface &srvfloaterface,
                               const SizedColumnTypes &inputTypes,
                               SizedColumnTypes &outputTypes) {
        uint8_t logK = readLogK(srvfloaterface);
        outputTypes.addVarbinary(quickSelectSketchMinSize(logK));
    }

    virtual void getParameterType(ServerInterface &srvInterface,
                                  SizedColumnTypes &parameterTypes) {
        // Unfortunately it cannot be forced in the intersection...
        // So we have to trust the user to give the maximum logK used of the input sketches.
        SizedColumnTypes::Properties logNominalProps;
        logNominalProps.required = false;
        logNominalProps.canBeNull = false;
        logNominalProps.comment = "Log Nominal value.";
        parameterTypes.addInt(DATASKETCHES_LOG_NOMINAL_VALUE_PARAMETER_NAME, logNominalProps);

        SizedColumnTypes::Properties seedProps;
        seedProps.required = false;
        seedProps.canBeNull = false;
        seedProps.comment = "Seed value";
        parameterTypes.addInt(DATASKETCHES_SEED_PARAMETER_NAME, seedProps);
    }
};


class ThetaSketchAggregateFunction : public AggregateFunction {
protected:
    uint8_t logK;
    uint64_t seed;

public:
    virtual void setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) {
        this->logK = readLogK(srvInterface);
        this->seed = readSeed(srvInterface);
    }

    virtual void initAggregate(ServerInterface &srvInterface, IntermediateAggs &aggs) {
        try {
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

    virtual void terminate(ServerInterface &srvInterface,
                           BlockWriter &resWriter,
                           IntermediateAggs &aggs) override {
        try {
            const VString &concat = aggs.getStringRef(0);
            VString &result = resWriter.getStringRef();
            result.copy(&concat);
        } catch (exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while computing aggregate output: [%s]", e.what());
        }
    }
};

#endif //COM_CRITEO_MOAB_DATASKETCHES_VERTICA_H
