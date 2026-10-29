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

SKSEPluginInfo(
    .Version = REL::Version{ 1, 0, 0, 0 },
    .Name = "GazBodyslideReader"sv,
    .Author = "Gaz-tec"sv,
    .SupportEmail = ""sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
    .MinimumSKSEVersion = REL::Version{ 0, 0, 0, 0 }
)

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
