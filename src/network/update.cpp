#include "update.h"

// The inherited unsigned HTTP updater is quarantined independently of the HBC
// build flavor. Even a direct API call cannot download or replace executables.
bool downloadUpdate(int) { return false; }
struct update checkUpdate() { return {}; }
