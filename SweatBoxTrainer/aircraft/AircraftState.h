#pragma once

#include <cstdint>

// Enum for tracking which properties have changed
enum class AircraftDirtyFlags : uint32_t {
	NONE           = 0,
	ALTITUDE       = 1 << 0,
	HEADING        = 1 << 1,
	VSPEED         = 1 << 2,
	LATITUDE       = 1 << 3,
	LONGITUDE      = 1 << 4,
	SPEED          = 1 << 5,
	TRACK          = 1 << 6,
	DATA           = 1 << 7,
	PITCH          = 1 << 8,
	ROLL           = 1 << 9,
	MODE           = 1 << 10,
	// Combine flags for convenience
	POSITION       = LATITUDE | LONGITUDE,
	ALL            = ~0u // Represents all flags
};

inline AircraftDirtyFlags operator|(AircraftDirtyFlags a, AircraftDirtyFlags b) {
	return static_cast<AircraftDirtyFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

// Holds the physical state of an aircraft. This is mostly a data container.
struct AircraftState {
public:
    double latitude = 0;
    double longitude = 0;
    double altitude = 0;
    double speed = 0;
    double verticalSpeed = 1000;
    double heading = 0;
    double pitch = 0;
    double roll = 0;

    // Dirty flag management
    void MarkDirty(AircraftDirtyFlags flags) { m_dirtyFlags |= static_cast<uint32_t>(flags); }
    bool IsDirty(AircraftDirtyFlags flags) const { return (m_dirtyFlags & static_cast<uint32_t>(flags)) != 0; }
    void ClearDirtyFlags() { m_dirtyFlags = static_cast<uint32_t>(AircraftDirtyFlags::NONE); }
    void MarkAllDirty() { m_dirtyFlags = static_cast<uint32_t>(AircraftDirtyFlags::ALL); }

private:
    uint32_t m_dirtyFlags = static_cast<uint32_t>(AircraftDirtyFlags::ALL); // Start dirty for initial display
};