/**
 * @file module.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "confmodel/DaqApplication.hpp"
#include "confmodel/HostComponent.hpp"

namespace py = pybind11;

namespace dunedaq::confmodel::python {

extern void
register_dal_methods(py::module&);

PYBIND11_MODULE(_daq_confmodel_py, m)
{

  m.doc() = "C++ implementation of the confmodel modules";
#if 0
  py::class_<dunedaq::confmodel::DaqApplication>(m,"DaqApplication")
    .def(py::init<conffwk::Configuration& , const conffwk::ConfigObject&>())
    .def("get_used_hostresources", &dunedaq::confmodel::DaqApplication::get_used_hostresources);
  py::class_<dunedaq::confmodel::HostComponent>(m,"HostComponent");
#endif
  register_dal_methods(m);
}

} // namespace dunedaq::confmodel::python
