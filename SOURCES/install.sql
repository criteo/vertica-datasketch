\set ON_ERROR_STOP on
CREATE OR REPLACE LIBRARY DataSketches AS '/home/dbadmin/build/libvertica-datasketches.so';

-- SELECT theta_sketch_get_estimate(theta_sketch) FROM ...
CREATE OR REPLACE FUNCTION theta_sketch_get_estimate AS
    LANGUAGE 'C++'
    NAME 'ThetaSketchGetEstimateFactory' LIBRARY DataSketches;
GRANT EXECUTE ON FUNCTION theta_sketch_get_estimate(LONG VARBINARY) TO PUBLIC;

-- SELECT theta_sketch_union(theta_sketch1, theta_sketch2, ...) FROM ...
CREATE OR REPLACE FUNCTION theta_sketch_union AS
    LANGUAGE 'C++'
    NAME 'ThetaSketchScalarUnionFactory' LIBRARY DataSketches;
GRANT EXECUTE ON FUNCTION theta_sketch_union(LONG VARBINARY) TO PUBLIC;

-- SELECT key, theta_sketch_union_agg(theta_sketch) FROM ... GROUP BY key
CREATE OR REPLACE AGGREGATE FUNCTION theta_sketch_union_agg AS
    LANGUAGE 'C++'
    NAME 'ThetaSketchAggregateUnionFactory' LIBRARY DataSketches;
GRANT EXECUTE ON AGGREGATE FUNCTION theta_sketch_union_agg(LONG VARBINARY) TO PUBLIC;

-- SELECT key, theta_sketch_create(binary) FROM ... GROUP BY key
CREATE OR REPLACE AGGREGATE FUNCTION theta_sketch_create AS
    LANGUAGE 'C++'
    NAME 'ThetaSketchAggregateCreateVarcharFactory' LIBRARY DataSketches;
GRANT EXECUTE ON AGGREGATE FUNCTION theta_sketch_create(VARCHAR) TO PUBLIC;

-- SELECT key, theta_sketch_create(chars) FROM ... GROUP BY key
CREATE OR REPLACE AGGREGATE FUNCTION theta_sketch_create AS
    LANGUAGE 'C++'
    NAME 'ThetaSketchAggregateCreateLongVarbinaryFactory' LIBRARY DataSketches;
GRANT EXECUTE ON AGGREGATE FUNCTION theta_sketch_create(LONG VARBINARY) TO PUBLIC;

-- SELECT theta_sketch_intersection(theta_sketch1, theta_sketch2, ...) FROM ...
CREATE OR REPLACE FUNCTION theta_sketch_intersection AS
    LANGUAGE 'C++'
    NAME 'ThetaSketchScalarIntersectionFactory' LIBRARY DataSketches;
GRANT EXECUTE ON FUNCTION theta_sketch_intersection(LONG VARBINARY) TO PUBLIC;

-- SELECT key, theta_sketch_intersection_agg(theta_sketch) FROM ... GROUP BY key
CREATE OR REPLACE AGGREGATE FUNCTION theta_sketch_intersection_agg AS
    LANGUAGE 'C++'
    NAME 'ThetaSketchAggregateIntersectionFactory' LIBRARY DataSketches;
GRANT EXECUTE ON AGGREGATE FUNCTION theta_sketch_intersection_agg(LONG VARBINARY) TO PUBLIC;

-- SELECT theta_sketch_a_not_b(theta_sketch_a, theta_sketch_b) FROM ...
CREATE OR REPLACE FUNCTION theta_sketch_a_not_b AS
    LANGUAGE 'C++'
    NAME 'ThetaSketchANotBFactory' LIBRARY DataSketches;
GRANT EXECUTE ON FUNCTION theta_sketch_a_not_b(LONG VARBINARY, LONG VARBINARY) TO PUBLIC;
