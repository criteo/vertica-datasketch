#include <Vertica.h>
#include <theta_sketch.hpp>
#include <theta_union.hpp>
#include "../../../include/datasketches/theta/theta_common.hpp"

using namespace Vertica;

class ThetaSketchScalarUnion : public ScalarFunction {
public:
    void processBlock(ServerInterface &srvInterface,
                      BlockReader &argReader,
                      BlockWriter &resWriter) {
        try {
            auto logK = readLogK(srvInterface);
            auto seed = readSeed(srvInterface);
            const SizedColumnTypes &inTypes = argReader.getTypeMetaData();
            std::vector<size_t> argCols; // Argument column indexes.
            inTypes.getArgumentColumns(argCols);

            // While we have inputs to process
            do {
                auto u = theta_union_custom::builder()
                        .set_lg_k(logK)
                        .set_seed(seed)
                        .build();
                for (uint i = 0; i < argCols.size(); i++) {
                    auto sketch = compact_theta_sketch_custom::deserialize(argReader.getStringRef(i).data(),
                                                                    argReader.getStringRef(i).length());
                    u.update(sketch);
                }
                auto data = u.get_result().serialize();
                resWriter.getStringRef().copy((char *) &data[0], data.size());
                resWriter.next();
            } while (argReader.next());
        } catch (std::exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while processing block: [%s]", e.what());
        }
    }
};

class ThetaSketchScalarUnionFactory : public ThetaSketchScalarFunctionFactory {
    // return an instance of AddAnyInts to perform the actual addition.
    virtual ScalarFunction *createScalarFunction(ServerInterface &interface) {
        return vt_createFuncObject<ThetaSketchScalarUnion>(interface.allocator);
    }

    virtual void getPrototype(ServerInterface &interface,
                              ColumnTypes &argTypes,
                              ColumnTypes &returnType) {
        argTypes.addAny();
        returnType.addLongVarbinary();
    }
};

RegisterFactory(ThetaSketchScalarUnionFactory);
