#ifndef CONFMODEL_INCLUDE_CONFMODEL_TEST_CIRCULAR_DEPENDENCY_HPP_
#define CONFMODEL_INCLUDE_CONFMODEL_TEST_CIRCULAR_DEPENDENCY_HPP_




namespace dunedaq::conffwk {
  class DalObject2g;
} // namespace dunedaq::conffwk

namespace dunedaq::confmodel {

    class TestCircularDependency {

      friend class AddTestOnCircularDependency;

      public:

        TestCircularDependency(const char * goal, const dunedaq::conffwk::DalObject * first_object) :
            p_goal(goal), p_index(0)
        {
          p_objects[p_index] = first_object;
          p_index++;
        }


      private:

        /// \throw dunedaq::confmodel::FoundCircularDependency
        void push(const dunedaq::conffwk::DalObject * object);

        void
        pop()
        {
          p_index--;
        }

          /// maximum recursion level
        enum {
	  p_limit = 64
	};

        const char * p_goal;
        unsigned int p_index;
        const dunedaq::conffwk::DalObject * p_objects[p_limit];

    };

    class AddTestOnCircularDependency {

      public:

        AddTestOnCircularDependency(TestCircularDependency& fuse, const dunedaq::conffwk::DalObject * obj) : p_fuse(fuse) { p_fuse.push(obj); }
        ~AddTestOnCircularDependency() { p_fuse.pop(); }


      private:

        TestCircularDependency& p_fuse;
    };
} // namespace dunedaq::confmodel


#endif // CONFMODEL_INCLUDE_CONFMODEL_TEST_CIRCULAR_DEPENDENCY_HPP_
