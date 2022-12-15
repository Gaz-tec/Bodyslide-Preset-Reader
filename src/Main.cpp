#include "Logging.h"
#include "Papyrus.h"
#include "BodyslideReader.h"

using namespace RE::BSScript;
using namespace Primary;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {
    void InitializePapyrus() {
        log::trace("Initializing Papyrus binding...");
        if (GetPapyrusInterface()->Register(Primary::RegisterBodyslideReader)) {
            log::debug("Papyrus functions bound.");
        } else {
            stl::report_and_fail("Failure to register Papyrus bindings.");
        }
    }
}  // namespace

SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();
    const auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);
    Init(skse);

    InitializePapyrus();
    BodyslideReader::GetSingleton().ReloadPresets();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
