#pragma once
#include "SetupAnalysis.hpp"
#include "HoldAnalysis.hpp"

namespace tatum {

/** \class SetupHoldAnalysis
 *
 * The SetupHoldAnalysis class defines the operations needed by a timing analyzer
 * to perform a combinded setup (max/long path) and hold (min/shortest path) analysis.
 *
 * Performing both analysis simultaneously tends to be more efficient than performing
 * them sperately due to cache locality.
 *
 * \see SetupAnalysis
 * \see HoldAnalysis
 * \see TimingAnalyzer
 */
class SetupHoldAnalysis {
    public:
        SetupHoldAnalysis(size_t num_tags, size_t num_slacks)
            : setup_visitor_(num_tags, num_slacks)
            , hold_visitor_(num_tags, num_slacks) {}

        void do_reset_node(const NodeId node_id) { 
            setup_visitor_.do_reset_node(node_id); 
            hold_visitor_.do_reset_node(node_id); 
        }
        void do_reset_edge(const EdgeId edge_id) { 
            setup_visitor_.do_reset_edge(edge_id); 
            hold_visitor_.do_reset_edge(edge_id); 
        }

        void do_arrival_pre_traverse_node(const TimingGraph& tg, const TimingConstraints& tc, const NodeId node_id) { 
            setup_visitor_.do_arrival_pre_traverse_node(tg, tc, node_id); 
            hold_visitor_.do_arrival_pre_traverse_node(tg, tc, node_id); 
        }

        void do_required_pre_traverse_node(const TimingGraph& tg, const TimingConstraints& tc, const NodeId node_id) { 
            setup_visitor_.do_required_pre_traverse_node(tg, tc, node_id); 
            hold_visitor_.do_required_pre_traverse_node(tg, tc, node_id); 
        }

        template<class DelayCalc>
        void do_arrival_traverse_node(const TimingGraph& tg, const TimingConstraints& tc, const DelayCalc& dc, const NodeId node_id) { 
            setup_visitor_.do_arrival_traverse_node(tg, tc, dc, node_id); 
            hold_visitor_.do_arrival_traverse_node(tg, tc, dc, node_id); 
        }

        template<class DelayCalc>
        void do_required_traverse_node(const TimingGraph& tg, const TimingConstraints& tc, const DelayCalc& dc, const NodeId node_id) { 
            setup_visitor_.do_required_traverse_node(tg, tc, dc, node_id); 
            hold_visitor_.do_required_traverse_node(tg, tc, dc, node_id); 
        }
        
        template<class DelayCalc>
        void do_slack_traverse_edge(const TimingGraph& tg, const DelayCalc& dc, const EdgeId edge) {
            setup_visitor_.do_slack_traverse_edge(tg, dc, edge); 
            hold_visitor_.do_slack_traverse_edge(tg, dc, edge); 
        }

        TimingTags::tag_range setup_tags(const NodeId node_id) const { return setup_visitor_.setup_tags(node_id); }
        TimingTags::tag_range setup_tags(const NodeId node_id, TagType type) const { return setup_visitor_.setup_tags(node_id, type); }
        TimingTags::tag_range setup_slacks(const EdgeId edge_id) const { return setup_visitor_.setup_slacks(edge_id); }

        TimingTags::tag_range hold_tags(const NodeId node_id) const { return hold_visitor_.hold_tags(node_id); }
        TimingTags::tag_range hold_tags(const NodeId node_id, TagType type) const { return hold_visitor_.hold_tags(node_id, type); }
        TimingTags::tag_range hold_slacks(const EdgeId edge_id) const { return hold_visitor_.hold_slacks(edge_id); }


    private:
        SetupAnalysis setup_visitor_;
        HoldAnalysis hold_visitor_;
};

} //namepsace
