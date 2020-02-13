#include <Vertica.h>
#include "../../../include/datasketches/theta/theta_common.hpp"


uint8_t readLogK(ServerInterface &serverInterface) {
    vint logK;
    ParamReader paramReader = serverInterface.getParamReader();

    if (paramReader.containsParameter(DATASKETCHES_LOG_NOMINAL_VALUE_PARAMETER_NAME)) {
        logK = paramReader.getIntRef(DATASKETCHES_LOG_NOMINAL_VALUE_PARAMETER_NAME);
        if (logK < DATASKETCHES_LOG_NOMINAL_VALUE_MIN || logK > DATASKETCHES_LOG_NOMINAL_VALUE_MAX) {
            vt_report_error(2,
                            "Provided value of the %s parameter is not supported. The value should be between %d and %d, inclusive",
                            DATASKETCHES_LOG_NOMINAL_VALUE_PARAMETER_NAME, DATASKETCHES_LOG_NOMINAL_VALUE_MIN,
                            DATASKETCHES_LOG_NOMINAL_VALUE_MAX);
        }
    } else {
        LogDebugUDxWarn(serverInterface, "Parameter %s was not provided. Defaulting to %d",
                        DATASKETCHES_LOG_NOMINAL_VALUE_PARAMETER_NAME, DATASKETCHES_LOG_NOMINAL_VALUE_DEFAULT);
        logK = DATASKETCHES_LOG_NOMINAL_VALUE_DEFAULT;
    }
    return logK;
}

uint64_t readSeed(ServerInterface &serverInterface) {
    vint seed;
    ParamReader paramReader = serverInterface.getParamReader();

    if (paramReader.containsParameter(DATASKETCHES_SEED_PARAMETER_NAME)) {
        seed = paramReader.getIntRef(DATASKETCHES_SEED_PARAMETER_NAME);
    } else {
        LogDebugUDxWarn(serverInterface, "Parameter %s was not provided. Defaulting to %d",
                        DATASKETCHES_SEED_PARAMETER_NAME, DATASKETCHES_SEED_DEFAULT);
        seed = DATASKETCHES_SEED_DEFAULT;
    }
    return seed;
}


uint32_t quickSelectSketchMinSize(uint8_t logK) {
    return 24 + (1 << logK) * 8;
}

uint32_t quickSelectSketchMaxSize(uint8_t logK) {
    return 24 + (1 << logK) * 15;
}