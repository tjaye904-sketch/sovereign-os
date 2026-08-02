#pragma once

#include "identifier.hpp"

namespace sovereign
{

struct RepositoryIdTag final {};
struct RuntimeIdTag final {};
struct NodeIdTag final {};
struct WorkspaceIdTag final {};
struct FacultyIdTag final {};
struct AgentIdTag final {};
struct ServiceIdTag final {};
struct SubscriptionIdTag final {};
struct EventIdTag final {};
struct TaskIdTag final {};
struct CorrelationIdTag final {};
struct TransactionIdTag final {};
struct DecisionIdTag final {};

using RepositoryId =
    Identifier<RepositoryIdTag>;

using RuntimeId =
    Identifier<RuntimeIdTag>;

using NodeId =
    Identifier<NodeIdTag>;

using WorkspaceId =
    Identifier<WorkspaceIdTag>;

using FacultyId =
    Identifier<FacultyIdTag>;

using AgentId =
    Identifier<AgentIdTag>;

using ServiceId =
    Identifier<ServiceIdTag>;

using SubscriptionId =
    Identifier<SubscriptionIdTag>;


using EventId =
    Identifier<EventIdTag>;

using TaskId =
    Identifier<TaskIdTag>;

using CorrelationId =
    Identifier<CorrelationIdTag>;

using TransactionId =
    Identifier<TransactionIdTag>;

using DecisionId =
    Identifier<DecisionIdTag>;

} // namespace sovereign
