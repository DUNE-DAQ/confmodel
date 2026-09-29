"""
C++ implementation of the confmodel modules
"""
from __future__ import annotations
import conffwk._daq_conffwk_py
__all__: list[str] = ['ObjectLocator', 'd2d_receiver', 'd2d_senders', 'd2d_streams', 'daq_application_construct_commandline_parameters', 'daqapp_get_used_host_components', 'entity_excluded', 'entity_get_parents', 'exclude_entity', 'exclude_entity_set_contains', 'include_entity', 'rc_application_construct_commandline_parameters', 'session_get_all_applications', 'session_get_included_applications']
class ObjectLocator:
    def __init__(self, arg0: str, arg1: str) -> None:
        ...
    @property
    def class_name(self) -> str:
        ...
    @property
    def id(self) -> str:
        ...
def d2d_receiver(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str) -> str:
    """
    Get receiver associated with DetectorToDaqConnection
    """
def d2d_senders(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str) -> list[str]:
    """
    Get senders associated with DetectorToDaqConnection
    """
def d2d_streams(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str) -> list[str]:
    """
    Get streams associated with DetectorToDaqConnection
    """
def daq_application_construct_commandline_parameters(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str, arg2: str) -> list[str]:
    """
    Get a version of the command line agruments parsed
    """
def daqapp_get_used_host_components(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str) -> list[str]:
    """
    Get list of HostExcludableEntitys used by DAQApplication
    """
def entity_excluded(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str, arg2: str) -> bool:
    """
    Determine if a ExcludableEntity-derived object (e.g. a Segment) has been excluded
    """
def entity_get_parents(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str, arg2: str) -> list[list[ObjectLocator]]:
    """
    Get the ExcludableEntity-derived class instances of the parent(s) of the ExcludableEntity-derived object in question
    """
def exclude_entity(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str, arg2: str) -> None:
    """
    Exclude a ExcludableEntity-derived object (e.g. a Segment)
    """
def exclude_entity_set_contains(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str) -> list[str]:
    """
    Get contained ExcludableEntitys from ExcludableEntitySet
    """
def include_entity(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str, arg2: str) -> None:
    """
    Include a ExcludableEntity-derived object (e.g. a Segment)
    """
def rc_application_construct_commandline_parameters(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str, arg2: str) -> list[str]:
    """
    Get a version of the command line agruments parsed
    """
def session_get_all_applications(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str) -> list[ObjectLocator]:
    """
    Get list of ALL applications (regardless of included/excluded state) in the requested session
    """
def session_get_included_applications(arg0: conffwk._daq_conffwk_py._Configuration, arg1: str) -> list[ObjectLocator]:
    """
    Get list of included applications in the requested session
    """
