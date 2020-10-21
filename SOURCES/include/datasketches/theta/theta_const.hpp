#ifndef VERTICA_UDFS_THETA_CONST_H
#define VERTICA_UDFS_THETA_CONST_H

#define DATASKETCHES_LOG_NOMINAL_VALUE_PARAMETER_NAME "logK"
// Vertica supports maximum 32000000 bytes in a long binary field, hence the limit.
// https://datasketches.apache.org/docs/Theta/ThetaErrorTable.html
#define DATASKETCHES_LOG_NOMINAL_VALUE_DEFAULT 24
#define DATASKETCHES_LOG_NOMINAL_VALUE_MIN 4
#define DATASKETCHES_LOG_NOMINAL_VALUE_MAX
#define DATASKETCHES_SEED_PARAMETER_NAME "seed"
#define DATASKETCHES_SEED_DEFAULT 9001


#endif //VERTICA_UDFS_THETA_CONST_H
