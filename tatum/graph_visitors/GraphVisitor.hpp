#pragma once
#include "timing_graph_fwd.hpp"
#include "timing_constraints_fwd.hpp"

namespace tatum {

class GraphVisitor {
    public:
        bool do_arrival_pre_traverse_node(const TimingGraph& /*tg*/, const TimingConstraints& /*tc*/, const NodeId /*node_id*/) {}
        void do_arrival_traverse_node(const TimingGraph& /*tg*/, const TimingConstraints& /*tc*/, const NodeId /*node_id*/) {}

        template<class DelayCalc>
        void do_required_pre_traverse_node(const TimingGraph& /*tg*/, const DelayCalc& /*dc*/, NodeId /*node_id*/) {}

        template<class DelayCalc>
        void do_required_traverse_node(const TimingGraph& /*tg*/, const DelayCalc& /*dc*/, NodeId /*node_id*/) {}

        void reset() {}
};

} //namepsace
