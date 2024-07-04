#include "visitorvalidation.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "global.h"
#include "event.h"



//FIXME this should be two pass - single check first, then sequence checks

void VisitorValidation::visit(const Block& el) const {
}

void VisitorValidation::visit(const TimingMsg& el) const  {
  eventSequenceCheck();
}

void VisitorValidation::visit(const Flow& el) const  {
  eventSequenceCheck();
  //targetcheck
  //destcheck
}

void VisitorValidation::visit(const Switch& el) const  {
  eventSequenceCheck();
  //targetcheck
  //destcheck
}

void VisitorValidation::visit(const Origin& el) const  {
  eventSequenceCheck();
  //targetcheck
  //destcheck
}

void VisitorValidation::visit(const StartThread& el) const  {
  eventSequenceCheck();
  //targetcheck
  //destcheck
}


void VisitorValidation::visit(const Flush& el) const {
  eventSequenceCheck();
  //targetcheck

}

void VisitorValidation::visit(const Noop& el) const {
  eventSequenceCheck();
  //targetcheck

}

void VisitorValidation::visit(const Wait& el) const {
  eventSequenceCheck();
  //targetcheck

}

void VisitorValidation::visit(const CmdQMeta& el) const {
  metaSequenceCheck();
}

void VisitorValidation::visit(const CmdQBuffer& el) const {
  //meta check unnecessary, not allowed to have children
}

void VisitorValidation::visit(const DestList& el) const {
  //meta check unnecessary, not allowed to have children
}

void VisitorValidation::visit(const Global& el) const {
  //meta check unnecessary, not allowed to have children
}