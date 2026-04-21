/**
 * @file Jsonable_test.cxx  Unit Tests for Jsonable dal class
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2025.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "conffwk/Configuration.hpp"

#include "confmodel/Jsonable.hpp"
#include "confmodel/JsonableTest.hpp"

#define BOOST_TEST_MODULE Jsonable_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(Jsonable_test)

using namespace dunedaq;

constexpr uint32_t u32{0xabcd1234};
constexpr uint32_t u32_2{0xdeadbeef};
constexpr int32_t s32{-42};
constexpr float float1{9.5};
constexpr std::string str{"9.5"};

constexpr int nattributes{5};
constexpr int nrelationships{1};

class Fixture {
public:
  Fixture() : confdb("oksconflibs") {
    const std::string oksfile{"/tmp/jsonabletest.data.xml"};
    const std::list<std::string> includes{
      "schema/confmodel/jsonable_test.schema.xml"
    };
    confdb.create(oksfile, includes);

    conffwk::ConfigObject config_obj1;
    confdb.create(oksfile, "JsonableTest", "test1", config_obj1);

    conffwk::ConfigObject config_obj2;
    confdb.create(oksfile, "JsonableTest", "test2", config_obj2);

    config_obj1.set_by_val<unsigned int>("u32val", u32);
    config_obj1.set_by_val<int>("s32val", s32);
    config_obj1.set_by_val<float>("fval", float1);
    config_obj1.set_by_val<std::string>("strval", str);

    config_obj1.set_obj("other", &config_obj2);

    config_obj2.set_by_val<unsigned int>("u32val", u32_2);
    config_obj2.set_by_val<int>("s32val", -2);

  }
  conffwk::Configuration confdb;

};

BOOST_AUTO_TEST_CASE(recursive){
  Fixture f;
  auto dal1 = f.confdb.get<confmodel::JsonableTest>("test1");
  BOOST_CHECK(dal1 != nullptr);

  nlohmann::json json1 = dal1->to_json(false, false);

  BOOST_CHECK(json1.size() == 1);
  BOOST_CHECK(json1["test1"].size() == nattributes+nrelationships);

  BOOST_CHECK(json1["test1"]["u32val"] == u32);
  BOOST_CHECK(json1["test1"]["s32val"] == s32);
  BOOST_CHECK(json1["test1"]["fval"] == float1);
  BOOST_CHECK(json1["test1"]["strval"] == str);

  const std::vector<bool> bvec = json1["test1"]["vec"];
  BOOST_CHECK(!bvec.empty());

  BOOST_CHECK(json1["test1"]["other"].is_object());
  BOOST_CHECK(json1["test1"]["other"]["test2"]["u32val"] == u32_2);

  auto dal2 = dal1->get_other();
  nlohmann::json json2 = dal2->to_json();
  BOOST_CHECK(json2["test2"]["u32val"] == u32_2);
}

BOOST_AUTO_TEST_CASE(single){
  Fixture f;
  auto dal1 = f.confdb.get<confmodel::JsonableTest>("test1");
  BOOST_CHECK(dal1 != nullptr);

  nlohmann::json json1 = dal1->to_json(true, false);
  BOOST_CHECK(json1.size() == 1);
  // test1 does not include relationship
  BOOST_CHECK(json1["test1"].size() == nattributes);
  BOOST_CHECK(json1["test1"]["other"].is_null());

  // Does still contain attributes
  BOOST_CHECK(json1["test1"]["u32val"] == u32);
}

BOOST_AUTO_TEST_CASE(no_name){
  Fixture f;
  auto dal1 = f.confdb.get<confmodel::JsonableTest>("test1");
  BOOST_CHECK(dal1 != nullptr);

  nlohmann::json json1 = dal1->to_json(false, true);

  // No "test1" entry container all attributes/relationships in top level
  BOOST_CHECK(json1.size() == nattributes+nrelationships);

  BOOST_CHECK(json1["u32val"] == u32);

  BOOST_CHECK(json1["other"].is_object());
  BOOST_CHECK(json1["other"]["u32val"] == u32_2);
}
BOOST_AUTO_TEST_SUITE_END()
