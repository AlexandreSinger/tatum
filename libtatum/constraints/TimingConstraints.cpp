#include <iostream>
#include <limits>

#include "tatum_assert.hpp"
#include "TimingConstraints.hpp"

using std::cout;
using std::endl;

namespace tatum {

TimingConstraints::domain_range TimingConstraints::clock_domains() const {
    return tatum::util::make_range(domain_ids_.begin(), domain_ids_.end());
}

const std::string& TimingConstraints::clock_domain_name(const DomainId id) const {
    return domain_names_[id];
}

NodeId TimingConstraints::clock_domain_source_node(const DomainId id) const {
    return domain_sources_[id];
}

DomainId TimingConstraints::node_clock_domain(const NodeId id) const {
    //This is currenlty a linear search through all clock sources and
    //I/O constraints, could be made more efficient but it is only called
    //rarely (i.e. during pre-traversals)

    //Is it a clock source?
    DomainId source_domain = find_node_source_clock_domain(id);
    if(source_domain) return source_domain;

    //Does it have an input constarint?
    for(auto kv : input_constraints()) {
        auto node_id = kv.first;
        auto domain_id = kv.second.domain;

        //TODO: Assumes a single clock per node
        if(node_id == id) return domain_id;
    }

    //Does it have an output constraint?
    for(auto kv : output_constraints()) {
        auto node_id = kv.first;
        auto domain_id = kv.second.domain;

        //TODO: Assumes a single clock per node
        if(node_id == id) return domain_id;
    }

    //None found
    return DomainId::INVALID();
}

bool TimingConstraints::node_is_clock_source(const NodeId id) const {
    //Returns a DomainId which converts to true if valid
    return bool(find_node_source_clock_domain(id));
}

bool TimingConstraints::node_is_constant_generator(const NodeId id) const {
    return constant_generators_.count(id);
}

DomainId TimingConstraints::find_node_source_clock_domain(const NodeId node_id) const {
    //We don't expect many clocks, so the linear search should be fine
    for(auto domain_id : clock_domains()) {
        if(clock_domain_source_node(domain_id) == node_id) {
            return domain_id;
        }
    }

    return DomainId::INVALID();
}

DomainId TimingConstraints::find_clock_domain(const std::string& name) const {
    //Linear search for name
    // We don't expect a large number of domains
    for(DomainId id : clock_domains()) {
        if(clock_domain_name(id) == name) {
            return id;
        }
    }

    //Not found
    return DomainId::INVALID();
}

bool TimingConstraints::should_analyze(const DomainId src_domain, const DomainId sink_domain) const {
    return setup_constraints_.count(DomainPair(src_domain, sink_domain)) 
           || hold_constraints_.count(DomainPair(src_domain, sink_domain));
}

float TimingConstraints::hold_constraint(const DomainId src_domain, const DomainId sink_domain) const {
    auto iter = hold_constraints_.find(DomainPair(src_domain, sink_domain));
    if(iter == hold_constraints_.end()) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    return iter->second;
}
float TimingConstraints::setup_constraint(const DomainId src_domain, const DomainId sink_domain) const {
    auto iter = setup_constraints_.find(DomainPair(src_domain, sink_domain));
    if(iter == setup_constraints_.end()) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    return iter->second;
}

float TimingConstraints::input_constraint(const NodeId node_id, const DomainId domain_id) const {

    auto iter = find_io_constraint(node_id, domain_id, input_constraints_);
    if(iter != input_constraints_.end()) {
        return iter->second.constraint;
    }

    return std::numeric_limits<float>::quiet_NaN();
}

float TimingConstraints::output_constraint(const NodeId node_id, const DomainId domain_id) const {

    auto iter = find_io_constraint(node_id, domain_id, output_constraints_);
    if(iter != output_constraints_.end()) {
        return iter->second.constraint;
    }

    return std::numeric_limits<float>::quiet_NaN();
}

TimingConstraints::constant_generator_range TimingConstraints::constant_generators() const {
    return tatum::util::make_range(constant_generators_.begin(), constant_generators_.end());
}

TimingConstraints::clock_constraint_range TimingConstraints::setup_constraints() const {
    return tatum::util::make_range(setup_constraints_.begin(), setup_constraints_.end());
}

TimingConstraints::clock_constraint_range TimingConstraints::hold_constraints() const {
    return tatum::util::make_range(hold_constraints_.begin(), hold_constraints_.end());
}

TimingConstraints::io_constraint_range TimingConstraints::input_constraints() const {
    return tatum::util::make_range(input_constraints_.begin(), input_constraints_.end());
}

TimingConstraints::io_constraint_range TimingConstraints::output_constraints() const {
    return tatum::util::make_range(output_constraints_.begin(), output_constraints_.end());
}

TimingConstraints::io_constraint_range TimingConstraints::input_constraints(const NodeId id) const {
    auto range = input_constraints_.equal_range(id);
    return tatum::util::make_range(range.first, range.second);
}

TimingConstraints::io_constraint_range TimingConstraints::output_constraints(const NodeId id) const {
    auto range = output_constraints_.equal_range(id);
    return tatum::util::make_range(range.first, range.second);
}

DomainId TimingConstraints::create_clock_domain(const std::string name) { 
    DomainId id = find_clock_domain(name);
    if(!id) {
        //Create it
        id = DomainId(domain_ids_.size()); 
        domain_ids_.push_back(id); 
        
        domain_names_.push_back(name);
        domain_sources_.emplace_back(NodeId::INVALID());

        TATUM_ASSERT(clock_domain_name(id) == name);
        TATUM_ASSERT(find_clock_domain(name) == id);
    }

    return id; 
}

void TimingConstraints::set_setup_constraint(const DomainId src_domain, const DomainId sink_domain, const float constraint) {
    std::cout << "SRC: " << src_domain << " SINK: " << sink_domain << " Constraint: " << constraint << std::endl;
    auto key = DomainPair(src_domain, sink_domain);
    auto iter = setup_constraints_.insert(std::make_pair(key, constraint));
    TATUM_ASSERT_MSG(iter.second, "Attempted to insert duplicate setup clock constraint");
}

void TimingConstraints::set_hold_constraint(const DomainId src_domain, const DomainId sink_domain, const float constraint) {
    std::cout << "SRC: " << src_domain << " SINK: " << sink_domain << " Constraint: " << constraint << std::endl;
    auto key = DomainPair(src_domain, sink_domain);
    auto iter = hold_constraints_.insert(std::make_pair(key, constraint));
    TATUM_ASSERT_MSG(iter.second, "Attempted to insert duplicate hold clock constraint");
}

void TimingConstraints::set_input_constraint(const NodeId node_id, const DomainId domain_id, const float constraint) {
    auto iter = find_io_constraint(node_id, domain_id, input_constraints_);
    if(iter != input_constraints_.end()) {
        //Found, update
        iter->second.constraint = constraint;
    } else {
        //Not found create it
        input_constraints_.insert(std::make_pair(node_id, IoConstraint(domain_id, constraint)));
    }
}

void TimingConstraints::set_output_constraint(const NodeId node_id, const DomainId domain_id, const float constraint) {
    auto iter = find_io_constraint(node_id, domain_id, output_constraints_);
    if(iter != output_constraints_.end()) {
        //Found, update
        iter->second.constraint = constraint;
    } else {
        //Not found create it
        output_constraints_.insert(std::make_pair(node_id, IoConstraint(domain_id, constraint)));
    }
}

void TimingConstraints::set_clock_domain_source(const NodeId node_id, const DomainId domain_id) {
    domain_sources_[domain_id] = node_id;
}

void TimingConstraints::set_constant_generator(const NodeId node_id, bool is_constant_generator) {
    if(is_constant_generator) {
        constant_generators_.insert(node_id);
    } else {
        constant_generators_.erase(node_id);
    }
}

void TimingConstraints::remap_nodes(const tatum::util::linear_map<NodeId,NodeId>& node_map) {
    std::multimap<NodeId,IoConstraint> remapped_input_constraints;
    std::multimap<NodeId,IoConstraint> remapped_output_constraints;
    tatum::util::linear_map<DomainId,NodeId> remapped_domain_sources(domain_sources_.size());

    for(auto kv : input_constraints_) {
        NodeId new_node_id = node_map[kv.first];

        remapped_input_constraints.insert(std::make_pair(new_node_id, kv.second));
    }
    for(auto kv : output_constraints_) {
        NodeId new_node_id = node_map[kv.first];

        remapped_output_constraints.insert(std::make_pair(new_node_id, kv.second));
    }

    for(size_t domain_idx = 0; domain_idx < domain_sources_.size(); ++domain_idx) {
        DomainId domain_id(domain_idx);

        NodeId old_node_id = domain_sources_[domain_id];
        if(old_node_id) {
            remapped_domain_sources[domain_id] = node_map[old_node_id];
        }
    }

    std::swap(input_constraints_, remapped_input_constraints);
    std::swap(output_constraints_, remapped_output_constraints);
    std::swap(domain_sources_, remapped_domain_sources);
}

void TimingConstraints::print() const {
    cout << "Setup Clock Constraints" << endl;
    for(auto kv : setup_constraints_) {
        auto key = kv.first;
        float constraint = kv.second;
        cout << "SRC: " << key.src_domain_id;
        cout << " SINK: " << key.sink_domain_id;
        cout << " Constraint: " << constraint;
        cout << endl;
    }
    cout << "Hold Clock Constraints" << endl;
    for(auto kv : hold_constraints_) {
        auto key = kv.first;
        float constraint = kv.second;
        cout << "SRC: " << key.src_domain_id;
        cout << " SINK: " << key.sink_domain_id;
        cout << " Constraint: " << constraint;
        cout << endl;
    }
    cout << "Input Constraints" << endl;
    for(auto kv : input_constraints_) {
        auto node_id = kv.first;
        auto io_constraint = kv.second;
        cout << "Node: " << node_id;
        cout << " Domain: " << io_constraint.domain;
        cout << " Constraint: " << io_constraint.constraint;
        cout << endl;
    }
    cout << "Output Constraints" << endl;
    for(auto kv : output_constraints_) {
        auto node_id = kv.first;
        auto io_constraint = kv.second;
        cout << "Node: " << node_id;
        cout << " Domain: " << io_constraint.domain;
        cout << " Constraint: " << io_constraint.constraint;
        cout << endl;
    }
}

TimingConstraints::io_constraint_iterator TimingConstraints::find_io_constraint(const NodeId node_id, const DomainId domain_id, const std::multimap<NodeId,IoConstraint>& io_constraints) const {
    auto range = io_constraints.equal_range(node_id);
    for(auto iter = range.first; iter != range.second; ++iter) {
        if(iter->second.domain == domain_id) return iter;
    }

    //Not found
    return io_constraints.end();
}

TimingConstraints::mutable_io_constraint_iterator TimingConstraints::find_io_constraint(const NodeId node_id, const DomainId domain_id, std::multimap<NodeId,IoConstraint>& io_constraints) {
    auto range = io_constraints.equal_range(node_id);
    for(auto iter = range.first; iter != range.second; ++iter) {
        if(iter->second.domain == domain_id) return iter;
    }

    //Not found
    return io_constraints.end();
}

} //namepsace