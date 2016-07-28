#include "db_init.h"

#include "agent.h"
#include "context.h"

namespace cyclus {

DbInit::DbInit(Agent* m) : m_(m), full_prefix_(true) {}

DbInit::DbInit(Agent* m, bool dummy) : m_(m), full_prefix_(false) {}

Datum* DbInit::NewDatum(std::string title) {
  std::string prefix = "AgentState";
  if (full_prefix_) {
    prefix += AgentSpec(m_->spec()).Sanitize();
  }
  Datum* d = m_->context()->NewDatum(prefix + title);
  d->AddVal("AgentId", m_->id());
  d->AddVal("SimTime", m_->context()->time());
  return d;
}

}  // namespace cyclus
