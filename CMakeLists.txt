cmake_minimum_required(VERSION 3.12)
project(confmodel VERSION 2.8.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(oksdalgen REQUIRED)
find_package(ers REQUIRED)
find_package(logging REQUIRED)
find_package(conffwk REQUIRED)
find_package(nlohmann_json REQUIRED)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################

daq_add_dal_library(dunedaq.schema.xml dummy_excludable_entity.schema.xml
  SOURCES dalMethods.cpp test_circular_dependency.cpp ExcludedEntities.cpp
  LINK_LIBRARIES okssystem::okssystem logging::logging nlohmann_json::nlohmann_json)

daq_add_application(listApps list_apps.cxx
  LINK_LIBRARIES confmodel_dal conffwk::conffwk)

daq_add_application(jsonable_test jsonable_test.cxx TEST
  LINK_LIBRARIES confmodel_dal conffwk::conffwk logging::logging)
##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_python_bindings
daq_add_python_bindings(*.cpp DAL )


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

daq_add_unit_test(ExcludedEntity_test LINK_LIBRARIES confmodel_dal conffwk::conffwk)

##############################################################################

daq_install()

