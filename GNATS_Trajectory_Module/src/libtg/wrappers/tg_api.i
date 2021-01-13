%module tg
%{
  #define SWIG_PYTHON_EXTRA_NATIVE_CONTAINERS
  #include "real_t.h"
  #include "PointWGS84.h"
  #include "FlightPlan.h"
  #include "tg_trajectory.h"
  #include "tg_api.h"
  #include "tg_api_wrapper.h"
%}

%include "typemaps.i"
%include "std_map.i"

%include "std_vector.i"
%template(IntVector) std::vector<int>;

%include "std_string.i"
%template(StringVector) std::vector<std::string>;

%include "real_t.h"
%template(RealVector) std::vector<real_t>;

%include "PointWGS84.h"
%template(PointWGS84Vector) std::vector<PointWGS84>;

%include "FlightPlan.h"
%template(IntFlightPlanMap) std::map<int, FlightPlan>;
/*%template(FlightPlanVector) std::vector<FlightPlan>;*/

%include "tg_trajectory.h"
%template(FlightModeVector) std::vector<flight_mode_e>;
%template(TrajectoryVector) std::vector<Trajectory>;

%include "tg_api.h"
%include "tg_api_wrapper.h"
