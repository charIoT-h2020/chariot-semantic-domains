project(ScalarInterface)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../utils)
include_directories(../algorithms)
include_directories(../applications)

set(SOURCES
   ../applications/Analyzer/Virtual/Control/AbstractControl.cpp
   ../applications/Analyzer/Virtual/Scalar/AtomicElement.cpp
   ../applications/Analyzer/Scalar/ConcreteOperation.cpp
   ../applications/Analyzer/Scalar/Implementation/Native/FloatElement.cpp
   ../applications/Analyzer/Scalar/Implementation/Native/IntegerElement.cpp
   ../applications/Analyzer/Scalar/Implementation/General/MultiBitElement.cpp
   ../applications/Analyzer/Scalar/Implementation/General/MultiFloatElement.cpp
   ../applications/Analyzer/Scalar/Constant/ConstantElement.cpp
   ../applications/Analyzer/Scalar/Exact/ExactElement.cpp
   ../applications/Analyzer/Scalar/Approximate/VirtualElement.cpp
   ../applications/Analyzer/Scalar/Approximate/Native/ConstantIntegerElement.cpp
   ../applications/Analyzer/Scalar/Approximate/Native/ConstantFloatElement.cpp
   ../applications/Analyzer/Scalar/Approximate/General/ConstantBit.cpp
   ../applications/Analyzer/Scalar/Approximate/General/ConstantMultiBit.cpp
   ../applications/Analyzer/Scalar/Approximate/General/ConstantMultiBitShare.cpp
   ../applications/Analyzer/Scalar/Approximate/General/ConstantMultiFloat.cpp
   ../applications/Analyzer/Scalar/Approximate/Disjunction.cpp
   ../applications/Analyzer/Scalar/Approximate/FormalOperation.cpp
   ../applications/Analyzer/Scalar/Approximate/IntervalInteger.cpp
   ../applications/Analyzer/Scalar/Approximate/Guard.cpp
   ../applications/Analyzer/Scalar/Approximate/Top.cpp
   ../applications/Analyzer/Scalar/Approximate/PrecisionLattice.cpp
   ../applications/Analyzer/Scalar/AbstractDomain.cpp
   )

# add_library(Scalar ${SOURCES})
add_library(ScalarInterface SHARED ${SOURCES})
target_link_libraries(ScalarInterface utils numerics)

