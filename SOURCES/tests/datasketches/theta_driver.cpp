#include <iostream>
#include "datasketches/theta/theta_const.hpp"
#include "datasketches/theta/theta_def.hpp"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


using namespace std;
using namespace datasketches;

/**
 * A simple driver program to show how ThetaSketch can be used
 */



void handler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}


int main(int argc, char **argv) {
    signal(SIGSEGV, handler);

    std::cout << "ALLOC USED " << custom_alloc_state::size_used << std::endl;

    try {
        auto sketchA = update_theta_sketch_custom::builder()
                .set_lg_k(DATASKETCHES_LOG_NOMINAL_VALUE_MAX)
                .set_seed(DATASKETCHES_SEED_DEFAULT)
                .build();

        std::cout << sketchA.is_empty() << std::endl;

        sketchA.update(1);
        sketchA.update(2);
        sketchA.update(3);


        auto sketchB = update_theta_sketch_custom::builder()
                .set_lg_k(DATASKETCHES_LOG_NOMINAL_VALUE_MAX)
                .set_seed(DATASKETCHES_SEED_DEFAULT)
                .build();

        sketchB.update(5);
        sketchB.update(6);
        sketchB.update(4);


        auto intersectionA = theta_intersection_custom(DATASKETCHES_SEED_DEFAULT);

        intersectionA.update(sketchA);
        intersectionA.update(sketchB);

        std::cout << intersectionA.get_result().is_empty() << std::endl;

        auto unionA = theta_union_custom::builder()
                .set_lg_k(DATASKETCHES_LOG_NOMINAL_VALUE_MAX)
                .set_seed(DATASKETCHES_SEED_DEFAULT)
                .build();

        intersectionA.update(sketchA);
        intersectionA.update(sketchB);

        unionA.update(sketchA);
        unionA.update(sketchB);


        std::cout << "Intersection: " << intersectionA.get_result().get_estimate() << std::endl;
        std::cout << "Union: " << unionA.get_result().get_estimate() << std::endl;
    } catch (const std::exception &exc) {
        std::cerr << exc.what();
    }


    std::cout << "ALLOC USED " << custom_alloc_state::size_used << std::endl;
}
