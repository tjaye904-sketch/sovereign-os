# Priority fixes 1–5 evidence

1. Honest plan enrichment: PlanningEngine heuristic-only; CognitiveService::BuildPlan calls EnrichPlanWithModel (router).
2. Enrichment smoke: goal on plan object; accepts no_router when models=nullptr.
3. Desktop smoke + IsHeadlessExecutionMode honor SOVEREIGN_*_HEADLESS / SOVEREIGN_TEST_HEADLESS at process start.
4. README: C++ purpose vs Python CU vs desktop operator.
5. IMPLEMENTATION_ROADMAP Phase 2–4 statuses aligned with in-tree v1.
