#include <Vertica.h>
#include <theta_sketch.hpp>
#include <theta_intersection.hpp>
#include "../../../include/datasketches/theta/theta_common.hpp"

using namespace Vertica;

class ThetaSketchScalarIntersection : public ScalarFunction {
public:
    void processBlock(ServerInterface &srvInterface,
                      BlockReader &argReader,
                      BlockWriter &resWriter) {
        try {
            auto seed = readSeed(srvInterface);
            const SizedColumnTypes &inTypes = argReader.getTypeMetaData();
            std::vector<size_t> argCols; // Argument column indexes.
            inTypes.getArgumentColumns(argCols);

            // While we have inputs to process
            do {
                auto intersection = theta_intersection_custom(seed);
                for (uint i = 0; i < argCols.size(); i++) {
                    auto sketch = compact_theta_sketch_custom::deserialize(argReader.getStringRef(i).data(),
                                                                    argReader.getStringRef(i).length());
                    intersection.update(sketch);
                }
                auto data = intersection.get_result().serialize();
                resWriter.getStringRef().copy((char *) &data[0], data.size());
                resWriter.next();
            } while (argReader.next());
        } catch (std::exception &e) {
            // Standard exception. Quit.
            vt_report_error(0, "Exception while processing block: [%s]", e.what());
        }
    }
};

class ThetaSketchScalarIntersectionFactory : public ThetaSketchScalarFunctionFactory {
    // return an instance of AddAnyInts to perform the actual addition.
    virtual ScalarFunction *createScalarFunction(ServerInterface &interface) {
        return vt_createFuncObject<ThetaSketchScalarIntersection>(interface.allocator);
    }

    virtual void getPrototype(ServerInterface &interface,
                              ColumnTypes &argTypes,
                              ColumnTypes &returnType) {
        argTypes.addAny();
        returnType.addLongVarbinary();
    }
};

RegisterFactory(ThetaSketchScalarIntersectionFactory);
