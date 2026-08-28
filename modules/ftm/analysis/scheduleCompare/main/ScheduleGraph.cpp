
#include "ScheduleGraph.h"

boost::dynamic_properties setDynamicProperties(ScheduleGraph& g, configuration& config) {
  boost::dynamic_properties dp = boost::dynamic_properties(boost::ignore_other_properties);
  if (config.check) {
    dp = boost::dynamic_properties();
    dp.property("cpu", boost::get(&ScheduleVertex::cpu, g));
    dp.property("qty", boost::get(&ScheduleVertex::qty, g));
    dp.property("vabs", boost::get(&ScheduleVertex::vabs, g));
    dp.property("flags", boost::get(&ScheduleVertex::flags, g));
  }
  // attributes of the graph
  // graph [root="Demo",rankdir = TB, nodesep = 0.6, mindist = 1.0, ranksep = 1.0, overlap = false]
  boost::ref_property_map<ScheduleGraph*, std::string> gName(boost::get_property(g, &GraphProperties::name));
  dp.property("name", gName);
  boost::ref_property_map<ScheduleGraph*, std::string> gRoot(boost::get_property(g, &GraphProperties::root));
  dp.property("root", gRoot);
  boost::ref_property_map<ScheduleGraph*, std::string> gRankdir(boost::get_property(g, &GraphProperties::rankdir));
  dp.property("rankdir", gRankdir);
  boost::ref_property_map<ScheduleGraph*, std::string> gNodesep(boost::get_property(g, &GraphProperties::nodesep));
  dp.property("nodesep", gNodesep);
  boost::ref_property_map<ScheduleGraph*, std::string> gMindist(boost::get_property(g, &GraphProperties::mindist));
  dp.property("mindist", gMindist);
  boost::ref_property_map<ScheduleGraph*, std::string> gRanksep(boost::get_property(g, &GraphProperties::ranksep));
  dp.property("ranksep", gRanksep);
  boost::ref_property_map<ScheduleGraph*, std::string> gOverlap(boost::get_property(g, &GraphProperties::overlap));
  dp.property("overlap", gOverlap);
  if (config.extraProperties) {
    boost::ref_property_map<ScheduleGraph*, std::string> gBb(boost::get_property(g, &GraphProperties::bb));
    boost::ref_property_map<ScheduleGraph*, std::string> g_draw_(boost::get_property(g, &GraphProperties::_draw_));
    boost::ref_property_map<ScheduleGraph*, std::string> gXdotversion(boost::get_property(g, &GraphProperties::xdotversion));
    dp.property("bb", gBb);
    dp.property("_draw_", g_draw_);
    dp.property("xdotversion", gXdotversion);
  }
  // attributes of vertices
  dp.property("type", boost::get(&ScheduleVertex::type, g));
  dp.property("name", boost::get(&ScheduleVertex::name, g));
  dp.property("label", boost::get(&ScheduleVertex::label, g));
  if (config.extraProperties) {
    dp.property("pos", boost::get(&ScheduleVertex::pos, g));
    dp.property("_draw_", boost::get(&ScheduleVertex::_draw_, g));
    dp.property("_ldraw_", boost::get(&ScheduleVertex::_ldraw_, g));
    dp.property("_hdraw_", boost::get(&ScheduleVertex::_hdraw_, g));
    dp.property("height", boost::get(&ScheduleVertex::height, g));
    dp.property("width", boost::get(&ScheduleVertex::width, g));
  }
  dp.property("shape", boost::get(&ScheduleVertex::shape, g));
  dp.property("penwidth", boost::get(&ScheduleVertex::penwidth, g));
  dp.property("fillcolor", boost::get(&ScheduleVertex::fillcolor, g));
  dp.property("color", boost::get(&ScheduleVertex::color, g));
  dp.property("style", boost::get(&ScheduleVertex::style, g));
  dp.property("tperiod", boost::get(&ScheduleVertex::tperiod, g));
  dp.property("qlo", boost::get(&ScheduleVertex::qlo, g));
  dp.property("qhi", boost::get(&ScheduleVertex::qhi, g));
  dp.property("qil", boost::get(&ScheduleVertex::qil, g));
  dp.property("tef", boost::get(&ScheduleVertex::tef, g));
  dp.property("toffs", boost::get(&ScheduleVertex::toffs, g));
  dp.property("par", boost::get(&ScheduleVertex::par, g));
  dp.property("id", boost::get(&ScheduleVertex::id, g));
  dp.property("fid", boost::get(&ScheduleVertex::fid, g));
  dp.property("gid", boost::get(&ScheduleVertex::gid, g));
  dp.property("evtno", boost::get(&ScheduleVertex::evtno, g));
  dp.property("sid", boost::get(&ScheduleVertex::sid, g));
  dp.property("bpid", boost::get(&ScheduleVertex::bpid, g));
  dp.property("beamin", boost::get(&ScheduleVertex::beamin, g));
  dp.property("bpcstart", boost::get(&ScheduleVertex::bpcstart, g));
  dp.property("reqnobeam", boost::get(&ScheduleVertex::reqnobeam, g));
  dp.property("vacc", boost::get(&ScheduleVertex::vacc, g));
  dp.property("res", boost::get(&ScheduleVertex::res, g));
  dp.property("tvalid", boost::get(&ScheduleVertex::tvalid, g));
  dp.property("tabs", boost::get(&ScheduleVertex::tabs, g));
  dp.property("target", boost::get(&ScheduleVertex::target, g));
  dp.property("dst", boost::get(&ScheduleVertex::dst, g));
  dp.property("reps", boost::get(&ScheduleVertex::reps, g));
  dp.property("prio", boost::get(&ScheduleVertex::prio, g));
  dp.property("twait", boost::get(&ScheduleVertex::twait, g));
  dp.property("wabs", boost::get(&ScheduleVertex::wabs, g));
  dp.property("clear", boost::get(&ScheduleVertex::clear, g));
  dp.property("ovr", boost::get(&ScheduleVertex::ovr, g));
  dp.property("beamproc", boost::get(&ScheduleVertex::beamproc, g));
  dp.property("pattern", boost::get(&ScheduleVertex::pattern, g));
  dp.property("patentry", boost::get(&ScheduleVertex::patentry, g));
  dp.property("patexit", boost::get(&ScheduleVertex::patexit, g));
  dp.property("bpentry", boost::get(&ScheduleVertex::bpentry, g));
  dp.property("bpexit", boost::get(&ScheduleVertex::bpexit, g));
  dp.property("section", boost::get(&ScheduleVertex::section, g));
  // attributes of edges
  dp.property("type", boost::get(&ScheduleEdge::type, g));
  dp.property("color", boost::get(&ScheduleEdge::color, g));
  dp.property("fieldhead", boost::get(&ScheduleEdge::fieldhead, g));
  dp.property("fieldtail", boost::get(&ScheduleEdge::fieldtail, g));
  dp.property("fieldwidth", boost::get(&ScheduleEdge::fieldwidth, g));
  if (config.extraProperties) {
    dp.property("_draw_", boost::get(&ScheduleEdge::_draw_, g));
    dp.property("_hdraw_", boost::get(&ScheduleEdge::_hdraw_, g));
    dp.property("pos", boost::get(&ScheduleEdge::pos, g));
  }
  return dp;
}

std::string getGraphName(ScheduleGraph& g) { return boost::get_property(g, &GraphProperties::name); }

void setGraphName(ScheduleGraph& g, std::string newName) { boost::set_property(g, &GraphProperties::name, newName); }
