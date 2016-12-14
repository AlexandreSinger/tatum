#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>

#include "echo_writer.hpp"

#include "tatum_assert.hpp"
#include "TimingGraph.hpp"

#include "TimingConstraints.hpp"
#include "TimingTags.hpp"
#include "timing_analyzers.hpp"

namespace tatum {

void write_tags(std::ostream& os, const std::string& type, const TimingTags::tag_range tags, const NodeId node_id);
void write_slacks(std::ostream& os, const std::string& type, const TimingTags::tag_range tags, const EdgeId edge);

void write_timing_graph(std::ostream& os, const TimingGraph& tg) {
    os << "timing_graph:" << "\n";

    //We manually iterate to write the nodes in ascending order
    for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
        NodeId node_id(node_idx);

        os << " node: " << size_t(node_id) << "\n";

        os << "  type: " << tg.node_type(node_id);
        os << "\n";

        os << "  in_edges: ";
        auto in_edges = tg.node_in_edges(node_id);
        std::vector<EdgeId> edges(in_edges.begin(), in_edges.end());
        std::sort(edges.begin(), edges.end()); //sort the edges for consitent output
        for(EdgeId edge_id : edges) {
            os << size_t(edge_id) << " ";
        }
        os << "\n";

        os << "  out_edges: ";  
        auto out_edges = tg.node_out_edges(node_id);
        edges =  std::vector<EdgeId>(out_edges.begin(), out_edges.end());
        std::sort(edges.begin(), edges.end()); //sort the edges for consitent output
        for(EdgeId edge_id : edges) {
            os << size_t(edge_id) << " ";
        }
        os << "\n";

    }

    //We manually iterate to write the edges in ascending order
    for(size_t edge_idx = 0; edge_idx < tg.edges().size(); ++edge_idx) {
        EdgeId edge_id(edge_idx);

        os << " edge: " << size_t(edge_id) << "\n";
        os << "  src_node: " << size_t(tg.edge_src_node(edge_id)) << "\n";
        os << "  sink_node: " << size_t(tg.edge_sink_node(edge_id)) << "\n";
        os << "  disabled: ";
        if(tg.edge_disabled(edge_id)) {
            os << "true";
        } else {
            os << "false";
        }
        os << "\n";
    }
    os << "\n";
}

void write_timing_constraints(std::ostream& os, const TimingConstraints& tc) {
    os << "timing_constraints:\n";    

    for(auto domain_id : tc.clock_domains()) {
        os << " type: CLOCK domain: " << size_t(domain_id) << " name: \"" << tc.clock_domain_name(domain_id) << "\"\n";
    }

    for(auto domain_id : tc.clock_domains()) {
        NodeId source_node_id = tc.clock_domain_source_node(domain_id);
        if(source_node_id) {
            os << " type: CLOCK_SOURCE node: " << size_t(source_node_id) << " domain: " << size_t(domain_id) << "\n";
        }
    }

    for(auto node_id : tc.constant_generators()) {
        os << " type: CONSTANT_GENERATOR node: " << size_t(node_id) << "\n";
    }

    for(auto kv : tc.input_constraints()) {
        auto node_id = kv.first;
        auto domain_id = kv.second.domain;
        auto constraint = kv.second.constraint;
        if(!isnan(constraint)) {
            os << " type: INPUT_CONSTRAINT node: " << size_t(node_id) << " domain: " << size_t(domain_id) << " constraint: " << constraint << "\n";
        }
    }

    for(auto kv : tc.output_constraints()) {
        auto node_id = kv.first;
        auto domain_id = kv.second.domain;
        auto constraint = kv.second.constraint;
        if(!isnan(constraint)) {
            os << " type: OUTPUT_CONSTRAINT node: " << size_t(node_id) << " domain: " << size_t(domain_id) << " constraint: " << constraint << "\n";
        }
    }

    for(auto kv : tc.setup_constraints()) {
        auto key = kv.first;
        auto constraint = kv.second;
        if(!isnan(constraint)) {
            os << " type: SETUP_CONSTRAINT";
            os << " launch_domain: " << size_t(key.src_domain_id);
            os << " capture_domain: " << size_t(key.sink_domain_id);
            os << " constraint: " << constraint;
            os << "\n";
        }
    }

    for(auto kv : tc.hold_constraints()) {
        auto key = kv.first;
        auto constraint = kv.second;
        if(!isnan(constraint)) {
            os << " type: HOLD_CONSTRAINT";
            os << " launch_domain: " << size_t(key.src_domain_id);
            os << " capture_domain: " << size_t(key.sink_domain_id);
            os << " constraint: " << constraint;
            os << "\n";
        }
    }
    os << "\n";
}

void write_analysis_result(std::ostream& os, const TimingGraph& tg, const std::shared_ptr<TimingAnalyzer> analyzer) {
    os << "analysis_result:\n";

    auto setup_analyzer = std::dynamic_pointer_cast<SetupTimingAnalyzer>(analyzer);
    if(setup_analyzer) {
        for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
            NodeId node_id(node_idx);
            write_tags(os, "SETUP_DATA_ARRIVAL", setup_analyzer->setup_tags(node_id, TagType::DATA_ARRIVAL), node_id);
        }
        for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
            NodeId node_id(node_idx);
            write_tags(os, "SETUP_DATA_REQUIRED", setup_analyzer->setup_tags(node_id, TagType::DATA_REQUIRED), node_id);
        }
        for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
            NodeId node_id(node_idx);
            write_tags(os, "SETUP_LAUNCH_CLOCK", setup_analyzer->setup_tags(node_id, TagType::CLOCK_LAUNCH), node_id);
        }
        for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
            NodeId node_id(node_idx);
            write_tags(os, "SETUP_CAPTURE_CLOCK", setup_analyzer->setup_tags(node_id, TagType::CLOCK_CAPTURE), node_id);
        }
        for(size_t edge_idx = 0; edge_idx < tg.edges().size(); ++edge_idx) {
            EdgeId edge_id(edge_idx);
            write_slacks(os, "SETUP_SLACK", setup_analyzer->setup_slacks(edge_id), edge_id);
        }
    }
    auto hold_analyzer = std::dynamic_pointer_cast<HoldTimingAnalyzer>(analyzer);
    if(hold_analyzer) {
        for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
            NodeId node_id(node_idx);
            write_tags(os, "HOLD_DATA_ARRIVAL", hold_analyzer->hold_tags(node_id, TagType::DATA_ARRIVAL), node_id);
        }
        for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
            NodeId node_id(node_idx);
            write_tags(os, "HOLD_DATA_REQUIRED", hold_analyzer->hold_tags(node_id, TagType::DATA_REQUIRED), node_id);
        }
        for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
            NodeId node_id(node_idx);
            write_tags(os, "HOLD_LAUNCH_CLOCK", hold_analyzer->hold_tags(node_id, TagType::CLOCK_LAUNCH), node_id);
        }
        for(size_t node_idx = 0; node_idx < tg.nodes().size(); ++node_idx) {
            NodeId node_id(node_idx);
            write_tags(os, "HOLD_CAPTURE_CLOCK", hold_analyzer->hold_tags(node_id, TagType::CLOCK_CAPTURE), node_id);
        }
        for(size_t edge_idx = 0; edge_idx < tg.edges().size(); ++edge_idx) {
            EdgeId edge_id(edge_idx);
            write_slacks(os, "HOLD_SLACK", hold_analyzer->hold_slacks(edge_id), edge_id);
        }
    }
    os << "\n";
}

void write_tags(std::ostream& os, const std::string& type, const TimingTags::tag_range tags, const NodeId node_id) {
    for(const auto& tag : tags) {
        TATUM_ASSERT(tag.type() != TagType::SLACK);

        float time = tag.time().value();

        if(!isnan(time)) {
            os << " type: " << type;
            os << " node: " << size_t(node_id);
            os << " launch_domain: ";
            if(tag.launch_clock_domain()) {
                os << size_t(tag.launch_clock_domain());
            } else {
                os << "-1";
            }
            os << " capture_domain: ";
            if(tag.capture_clock_domain()) {
                os << size_t(tag.capture_clock_domain());
            } else {
                os << "-1";
            }
            os << " time: " << time;
            os << "\n";
        }
    }
}

void write_slacks(std::ostream& os, const std::string& type, const TimingTags::tag_range tags, const EdgeId edge) {
    for(const auto& tag : tags) {
        TATUM_ASSERT(tag.type() == TagType::SLACK);

        float time = tag.time().value();

        if(!isnan(time)) {
            os << " type: " << type;
            os << " edge: " << size_t(edge);
            os << " launch_domain: ";
            if(tag.launch_clock_domain()) {
                os << size_t(tag.launch_clock_domain());
            } else {
                os << "-1";
            }
            os << " capture_domain: ";
            if(tag.capture_clock_domain()) {
                os << size_t(tag.capture_clock_domain());
            } else {
                os << "-1";
            }
            os << " slack: " << time;
            os << "\n";
        }
    }
}

}
