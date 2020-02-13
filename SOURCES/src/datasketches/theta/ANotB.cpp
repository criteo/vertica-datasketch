
#include <Vertica.h>
#include <theta_sketch.hpp>
#include <theta_a_not_b.hpp>
#include "../../../include/datasketches/theta/theta_common.hpp"

using namespace Vertica;

class ThetaSketchANotB : public ScalarFunction {
public:
    void processBlock(ServerInterface &srvInterface,
                      BlockReader &argReader,
                      BlockWriter &resWriter) {
        try {
            auto seed = readSeed(srvInterface);
            const SizedColumnTypes &inTypes = argReader.getTypeMetaData();
            std::vector<size_t> argCols; // Argument column indexes.
            inTypes.getArgumentColumns(argCols);

            auto aNotB = theta_a_not_b_custom(seed);
            // While we have inputs to process
            do {
                auto a = compact_theta_sketch_custom::deserialize(argReader.getStringRef(0).data(),
                                                                argReader.getStringRef(0).length(),
                                                                seed);
                auto b = compact_theta_sketch_custom::deserialize(argReader.getStringRef(1).data(),
                                                                argReader.getStringRef(1).length(),
                                                                seed);

                auto data = aNotB.compute(a, b).serialize();
                resWriter.getStringRef().copy((char *) &data[0], data.size());
                resWriter.next();
            } while (argReader.next());
        } catch (std::exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while processing block: [%s]", e.what());
        }
    }
};

class ThetaSketchANotBFactory : public ThetaSketchScalarFunctionFactory {
    // return an instance of AddAnyInts to perform the actual addition.
    virtual ScalarFunction *createScalarFunction(ServerInterface &interface) {
        return vt_createFuncObject<ThetaSketchANotB>(interface.allocator);
    }

    virtual void getPrototype(ServerInterface &interface,
                              ColumnTypes &argTypes,
                              ColumnTypes &returnType) {
        argTypes.addVarbinary();
        argTypes.addVarbinary();
        returnType.addVarbinary();
    }
};

RegisterFactory(ThetaSketchANotBFactory);
