#include "tatum_assert.hpp"

namespace tatum {

inline std::ostream& operator<<(std::ostream& os, TagType type) {
    if(type == TagType::DATA) os << "DATA";
    else if(type == TagType::CLOCK_LAUNCH) os << "CLOCK_LAUNCH";
    else if(type == TagType::CLOCK_CAPTURE) os << "CLOCK_CAPTURE";
    else TATUM_ASSERT_MSG(false, "Unrecognized TagType");
    return os;
}

/*
 * TimingTag implementation
 */

inline TimingTag::TimingTag()
    : arr_time_(NAN)
    , req_time_(NAN)
    , clock_domain_(DomainId::INVALID())
    , launch_node_(NodeId::INVALID())
    {}

inline TimingTag::TimingTag(const Time& arr_time_val, const Time& req_time_val, const DomainId domain, const NodeId node, const TagType type)
    : arr_time_(arr_time_val)
    , req_time_(req_time_val)
    , clock_domain_(domain)
    , launch_node_(node)
    , type_(type)
    {}

inline TimingTag::TimingTag(const Time& arr_time_val, const Time& req_time_val, const TimingTag& base_tag)
    : arr_time_(arr_time_val)
    , req_time_(req_time_val)
    , clock_domain_(base_tag.clock_domain())
    , launch_node_(base_tag.launch_node())
    , type_(base_tag.type())
    {}


inline void TimingTag::update_arr(const Time& new_arr_time, const TimingTag& base_tag) {
    //NOTE: leave next alone, since we want to keep the linked list intact
    TATUM_ASSERT(clock_domain() == base_tag.clock_domain()); //Domain must be the same
    set_arr_time(new_arr_time);
    set_launch_node(base_tag.launch_node());
}

inline void TimingTag::update_req(const Time& new_req_time, const TimingTag& base_tag) {
    //NOTE: leave next alone, since we want to keep the linked list intact
    //      leave launch_node alone, since it is set by arrival only
    TATUM_ASSERT(clock_domain() == base_tag.clock_domain()); //Domain must be the same
    set_req_time(new_req_time);
}


inline void TimingTag::max_arr(const Time& new_arr_time, const TimingTag& base_tag) {
    //Need to min with existing value
    if(!arr_time().valid() || new_arr_time.value() > arr_time().value()) {
        //New value is smaller, or no previous valid value existed
        //Update min
        update_arr(new_arr_time, base_tag);
    }
}

inline void TimingTag::min_req(const Time& new_req_time, const TimingTag& base_tag) {
    //Need to min with existing value
    if(!req_time().valid() || new_req_time.value() < req_time().value()) {
        //New value is smaller, or no previous valid value existed
        //Update min
        update_req(new_req_time, base_tag);
    }
}

inline void TimingTag::min_arr(const Time& new_arr_time, const TimingTag& base_tag) {
    //Need to min with existing value
    if(!arr_time().valid() || new_arr_time.value() < arr_time().value()) {
        //New value is smaller, or no previous valid value existed
        //Update min
        update_arr(new_arr_time, base_tag);
    }
}

inline void TimingTag::max_req(const Time& new_req_time, const TimingTag& base_tag) {
    //Need to min with existing value
    if(!req_time().valid() || new_req_time.value() > req_time().value()) {
        //New value is smaller, or no previous valid value existed
        //Update min
        update_req(new_req_time, base_tag);
    }
}

} //namepsace
