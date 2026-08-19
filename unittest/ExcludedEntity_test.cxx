/**
 * @file ExcludedEntity_test.cxx  Unit Tests for Entities exclusion logic
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2025.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "conffwk/Configuration.hpp"
#include "confmodel/ExcludedEntities.hpp"
#include "confmodel/DummyApplication.hpp"
#include "confmodel/DummyD2D.hpp"
#include "confmodel/DummyReceiver.hpp"
#include "confmodel/DummyExcludableEntity.hpp"
#include "confmodel/DummyExcludableEntitySetAND.hpp"
#include "confmodel/DummyExcludableEntitySet.hpp"
#include "confmodel/DummySender.hpp"
#include "confmodel/DummySmartExcludableEntity.hpp"
#include "confmodel/DummyStream.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include "logging/Logging.hpp"

#define BOOST_TEST_MODULE ExcludedEntities_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <list>
#include <string>


BOOST_AUTO_TEST_SUITE(ExcludedEntities_test)

using namespace dunedaq;
using namespace dunedaq::confmodel;


BOOST_AUTO_TEST_CASE(simple_excludable_entity_set){

  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_excludable_entity.schema.xml"};
  confdb.create(oksfile, includes);

  std::vector<const DummyExcludableEntity*> dummy_excludable_entitys;
  std::vector<const conffwk::ConfigObject*> excludable_entity_config_objects;
  for (std::string id : {"dummyRes-0", "dummyRes-1", "dummyRes-2"}) {
    conffwk::ConfigObject conf_obj;
    confdb.create(oksfile, "DummyExcludableEntity", id, conf_obj);
    auto res_dal = confdb.get<DummyExcludableEntity>(conf_obj);
    excludable_entity_config_objects.push_back(&res_dal->config_object());
    dummy_excludable_entitys.push_back(res_dal);
  }

  conffwk::ConfigObject conf_obj;
  confdb.create(oksfile, "DummyExcludableEntitySet", "root", conf_obj);
  conf_obj.set_objs("items", excludable_entity_config_objects);

  auto root = confdb.get<DummyExcludableEntitySet>(conf_obj);

  // Nothing excluded
  ExcludedEntities dr(root,{});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( dr.is_included(dummy_excludable_entitys[1]) );

  // Single excludable_entity excluded
  dr.update(root,{dummy_excludable_entitys[1]});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( dr.is_included(dummy_excludable_entitys[0]) );
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[1]) );

  // All simple excludable_entitys excluded - no affect on ExcludableEntitySet
  dr.update(root, {dummy_excludable_entitys[0], dummy_excludable_entitys[1], dummy_excludable_entitys[2]});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[1]) );

  // ExcludableEntitySet excluded -- should affect contained simple ExcludableEntitys
  dr.update(root,{root});
  BOOST_CHECK( !dr.is_included(root) );
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[0]) );
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[1]) );

}

BOOST_AUTO_TEST_CASE(excludable_entity_set_and){

  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_excludable_entity.schema.xml"};
  confdb.create(oksfile, includes);

  std::vector<const DummyExcludableEntity*> dummy_excludable_entitys;
  std::vector<const conffwk::ConfigObject*> excludable_entity_config_objects;
  for (std::string id : {"dummyRes-0", "dummyRes-1", "dummyRes-2"}) {
    conffwk::ConfigObject conf_obj;
    confdb.create(oksfile, "DummyExcludableEntity", id, conf_obj);
    auto res_dal = confdb.get<DummyExcludableEntity>(conf_obj);
    excludable_entity_config_objects.push_back(&res_dal->config_object());
    dummy_excludable_entitys.push_back(res_dal);
  }

  conffwk::ConfigObject conf_obj;
  confdb.create(oksfile, "DummyExcludableEntitySetAND", "root", conf_obj);
  conf_obj.set_objs("items", excludable_entity_config_objects);

  auto root = confdb.get<DummyExcludableEntitySetAND>(conf_obj);

  // Nothing excluded
  ExcludedEntities dr(root,{});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( dr.is_included(dummy_excludable_entitys[1]) );

  // Single excludable_entity excluded
  dr.update(root,{dummy_excludable_entitys[1]});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( dr.is_included(dummy_excludable_entitys[0]) );
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[1]) );

  // All simple excludable_entitys excluded - also excludes ExcludableEntitySetAND
  dr.update(root, {dummy_excludable_entitys[0], dummy_excludable_entitys[1], dummy_excludable_entitys[2]});
  BOOST_CHECK( !dr.is_included(root) );
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[1]) );

  // ExcludableEntitySet excluded -- should affect contained simple ExcludableEntitys
  dr.update(root,{root});
  BOOST_CHECK( !dr.is_included(root) );
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[0]) );
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[1]) );

}


BOOST_AUTO_TEST_CASE(segment){
  dunedaq::logging::Logging().setup("a","a");
  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_excludable_entity.schema.xml"};
  confdb.create(oksfile, includes);

  std::vector<const DummyApplication*> dummy_apps;
  std::vector<const conffwk::ConfigObject*> app_config_objects;
  for (std::string id : {"dummyApp-0", "dummyApp-1", "dummyApp-2"}) {
    conffwk::ConfigObject conf_obj;
    confdb.create(oksfile, "DummyApplication", id, conf_obj);
    auto res_dal = confdb.get<DummyApplication>(conf_obj);
    app_config_objects.push_back(&res_dal->config_object());
    dummy_apps.push_back(res_dal);
  }

  conffwk::ConfigObject segment_conf_obj;
  confdb.create(oksfile, "Segment", "root", segment_conf_obj);
  segment_conf_obj.set_objs("applications", app_config_objects);

  auto root = confdb.get<Segment>(segment_conf_obj);

  // Nothing excluded
  ExcludedEntities dr(root,{});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( dr.is_included(dummy_apps[0]) );

  // Single excludable_entity excluded
  dr.update(root,{dummy_apps[0]});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( !dr.is_included(dummy_apps[0]) );
  BOOST_CHECK( dr.is_included(dummy_apps[1]) );

  // All simple excludable_entitys excluded - also excludes Segment (ExcludableEntitySetAND)
  dr.update(root, {dummy_apps[0], dummy_apps[1], dummy_apps[2]});
  BOOST_CHECK( !dr.is_included(root) );
  BOOST_CHECK( !dr.is_included(dummy_apps[1]) );
}


BOOST_AUTO_TEST_CASE(detector_to_daq){

  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_excludable_entity.schema.xml"};
  confdb.create(oksfile, includes);

  conffwk::ConfigObject conf_obj;

  std::vector<const ExcludableEntity*> sender0_streams;
  std::vector<const conffwk::ConfigObject*> stream_config_objects;
  for (std::string id : {"dummyStream-0", "dummyStream-1", "dummyStream-2"}) {
    confdb.create(oksfile, "DummyStream", id, conf_obj);
    auto res_dal = confdb.get<DummyStream>(conf_obj);
    stream_config_objects.push_back(&res_dal->config_object());
    sender0_streams.push_back(res_dal);
  }


  std::vector<const conffwk::ConfigObject*> sender_conf_objs;
  confdb.create(oksfile, "DummySender", "sender-0", conf_obj);
  conf_obj.set_objs("streams", stream_config_objects);
  auto sender0_dal = confdb.get<DummySender>(conf_obj);
  sender_conf_objs.push_back(&sender0_dal->config_object());


  std::vector<const ExcludableEntity*> sender1_streams;
  stream_config_objects.clear();
  for (std::string id : {"dummyStream-3", "dummyStream-4", "dummyStream-5"}) {
    confdb.create(oksfile, "DummyStream", id, conf_obj);
    auto res_dal = confdb.get<DummyStream>(conf_obj);
    stream_config_objects.push_back(&res_dal->config_object());
    sender1_streams.push_back(res_dal);
  }

  confdb.create(oksfile, "DummySender", "sender-1", conf_obj);
  conf_obj.set_objs("streams", stream_config_objects);
  auto sender1_dal = confdb.get<DummySender>(conf_obj);
  sender_conf_objs.push_back(&sender1_dal->config_object());

  confdb.create(oksfile, "DummyReceiver", "receiver-0", conf_obj);
  auto receiver = confdb.get<DummyReceiver>(conf_obj);
  auto receiver_dal = confdb.get<DummyReceiver>(conf_obj);
  auto receiver_conf_obj = receiver_dal->config_object();

  confdb.create(oksfile, "DummyD2D", "d2d-0", conf_obj);
  conf_obj.set_objs("dummy_senders", sender_conf_objs);
  conf_obj.set_obj("dummy_receiver", &receiver_conf_obj);
  auto d2d_dal = confdb.get<DummyD2D>(conf_obj);
  auto d2d_conf_obj = d2d_dal->config_object();
  confdb.create(oksfile, "DummyExcludableEntitySet", "root", conf_obj);
  conf_obj.set_objs("items", {&d2d_conf_obj});

  auto root = confdb.get<DummyExcludableEntitySet>(conf_obj);

  // Nothing excluded
  ExcludedEntities dr(root,{});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( dr.is_included(receiver_dal) );
  BOOST_CHECK( dr.is_included(d2d_dal) );

  // receiver excluded - excludes d2d
  dr.update(root,{receiver});
  BOOST_CHECK( dr.is_included(root) );
  BOOST_CHECK( !dr.is_included(receiver_dal) );
  BOOST_CHECK( !dr.is_included(d2d_dal) );

  
  std::vector<const ExcludableEntity*> exclude = sender0_streams;
  // All streams of sender0 excluded - excludes sender0
  dr.update(root, exclude);
  BOOST_CHECK( dr.is_included(d2d_dal) );
  BOOST_CHECK( dr.is_included(sender1_dal) );
  BOOST_CHECK( !dr.is_included(sender0_dal) );
  BOOST_CHECK( dr.is_included(receiver_dal) );

  // All streams of both senders excluded - excludes d2d and in turn receiver
  exclude.insert(exclude.end(), sender1_streams.begin(), sender1_streams.end());
  dr.update(root, exclude);
  BOOST_CHECK( !dr.is_included(d2d_dal) );
  BOOST_CHECK( !dr.is_included(receiver_dal) );

  // Sender0 and all streams of sender1 excluded - also excludes d2d
  exclude.clear();
  exclude.push_back(sender0_dal);
  exclude.insert(exclude.end(), sender1_streams.begin(), sender1_streams.end());
  dr.update(root, exclude);
  BOOST_CHECK( !dr.is_included(d2d_dal) );
  BOOST_CHECK( !dr.is_included(receiver_dal) );

  // Both senders excluded - same as above
  exclude.clear();
  exclude.push_back(sender0_dal);
  exclude.push_back(sender1_dal);
  dr.update(root, exclude);
  BOOST_CHECK( !dr.is_included(d2d_dal) );
  BOOST_CHECK( !dr.is_included(receiver_dal) );

}

BOOST_AUTO_TEST_CASE(smart_excludable_entity){

  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_excludable_entity.schema.xml"};
  confdb.create(oksfile, includes);

  std::vector<const DummySmartExcludableEntity*> dummy_excludable_entitys;
  std::vector<const conffwk::ConfigObject*> excludable_entity_config_objects;
  for (std::string id : {"deadun", "dummyRes-1", "dummyRes-2"}) {
    conffwk::ConfigObject conf_obj;
    confdb.create(oksfile, "DummySmartExcludableEntity", id, conf_obj);
    auto res_dal = confdb.get<DummySmartExcludableEntity>(conf_obj);
    excludable_entity_config_objects.push_back(&res_dal->config_object());
    dummy_excludable_entitys.push_back(res_dal);
  }

  conffwk::ConfigObject conf_obj;
  confdb.create(oksfile, "DummyExcludableEntitySetAND", "root", conf_obj);
  conf_obj.set_objs("items", excludable_entity_config_objects);

  auto root = confdb.get<DummyExcludableEntitySetAND>(conf_obj);

  // Nothing explicitly excluded, one DummySmartExcludableEntity excluded due to UID
  ExcludedEntities dr(root,{});
  BOOST_CHECK( dr.is_included(root));
  BOOST_CHECK( !dr.is_included(dummy_excludable_entitys[0]) );
  BOOST_CHECK( dr.is_included(dummy_excludable_entitys[1]) );

}

BOOST_AUTO_TEST_SUITE_END()
