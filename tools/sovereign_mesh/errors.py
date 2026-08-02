"""Mesh-specific exception types."""


class MeshError(RuntimeError):
    """Base exception for governed mesh failures."""


class PolicyViolation(MeshError):
    """Raised when an operation violates the authority policy."""


class PrimaryCheckoutViolation(PolicyViolation):
    """Raised when an agent operation targets the primary checkout."""


class ApprovalRequired(PolicyViolation):
    """Raised when an operation requires an approved user decision."""


class InterfaceUnavailable(MeshError):
    """Raised when a required external interface is unavailable."""
