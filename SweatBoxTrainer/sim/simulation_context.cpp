#include "simulation_context.h"

SimulationContext& SimulationContext::instance() {
    static SimulationContext ctx;
    return ctx;
}