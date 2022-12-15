#include "Papyrus.h"

#include <BodyslideReader.h>

using namespace Primary;
using namespace RE;
using namespace RE::BSScript;
using namespace REL;
using namespace SKSE;

namespace {
    constexpr std::string_view PapyrusClass = "BodyslideReader";

    // int32_t* PopulateHitData(Actor* target, char* unk0);

    // The <code>Relocation</code> class references something in the Skyrim engine (e.g. static data, a function, etc.)
    // that can be relocated in memory at runtime. Windows will rebase the memory of the Skyrim process to a random one,
    // so we cannot hard-code the memory address. Relocation will rebase your memory offset to the base memory address
    // of the Skyrim process, so that it adjusts on every run. However, it is recommended not to provide a memory
    // offset, but rather a <code>REL::ID</code> to lookup the offset dynamically with Address Library.
    //
    // The ID here is the ID of the function in Address Library. We use it to not be tied to the specific Skyrim
    // executable version. However, there are still three incompatible databases of addresses, for Skyrim 1.5.x
    // (pre-AE), Skyrim 1.6.x (post-AE), and Skyrim VR. This sample supports all three by allowing you to build a
    // separate DLL for each and changing the ID used with a macro. The SE ID is used for Skyrim VR as well, since the
    // VR address library uses SE ID's but maps them to a VR offset. RELOCATION_ID is a macro in CommonLibSSE NG which
    // can dynamically choose between an SE and AE ID at runtime, depending on which version of Skyrim is in use. This
    // allows for a single DLL that works across both versions of Skyrim.
    // Relocation<decltype(PopulateHitData)>& GetHookedFunction() noexcept {
    //    static Relocation<decltype(PopulateHitData)> value(RELOCATION_ID(42832, 44001), 0x42);
    //    return value;
   // }

    //Relocation<decltype(PopulateHitData)> OriginalPopulateHitData;

    // Start handlers for Papyrus functions.
    //
    // Note that SKSE cannot allow creation of new instanced Papyrus types. Therefore, all Papyrus extensions will be
    // static functions (labeled "global" in Papyrus). For the purpose of binding a function, this is indicated by the
    // first parameter being a <code>RE::StaticFunctionTag*</code>.
    //
    // CommonLibSSE handles conversion of Papyrus types to C++ types. Forms, active magic effects, and aliases can be
    // translated back and forth between Papyrus classes and a poitner to their C++ equivalent, e.g. ObjectReference
    // and RE::TESObjectREFR*. Papyrus arrays and be translated back and forth to std::vector or other array-like
    // classes (e.g. std::list, or even Skyrim classes like RE::BSTArray). Strings can be translated as parameters that
    // are std::string, std::string_view, or Skyrim's RE::BSFixedString (a case-preserving but case-insensitive interned
    // string), and the primitive types are converted to <code>bool</code>, <code>int</code>, and <code>float</code>.

    void ReloadPresets(StaticFunctionTag*) { return BodyslideReader::GetSingleton().ReloadPresets();
    }

    std::vector<float_t> GetPresetSliderHighs(StaticFunctionTag*, std::string name) {
        return BodyslideReader::GetSingleton().GetPresetSliderHighs(name);
    }

    std::vector<float_t> GetPresetSliderLows(StaticFunctionTag*, std::string name) {
        return BodyslideReader::GetSingleton().GetPresetSliderLows(name);
    }

    std::vector<std::string> GetPresetList(StaticFunctionTag*, bool female) {
        return BodyslideReader::GetSingleton().GetPresetList(female);
    }

    std::vector<std::string> GetPresetSliderStrings(StaticFunctionTag*, std::string name) {
        return BodyslideReader::GetSingleton().GetPresetSliderStrings(name);
    }
}


bool Primary::RegisterBodyslideReader(IVirtualMachine* vm) {
    vm->RegisterFunction("ReloadPresets", PapyrusClass, ReloadPresets);
    vm->RegisterFunction("GetPresetSliderHighs", PapyrusClass, GetPresetSliderHighs);
    vm->RegisterFunction("GetPresetSliderLows", PapyrusClass, GetPresetSliderLows);
    vm->RegisterFunction("GetPresetList", PapyrusClass, GetPresetList);
    vm->RegisterFunction("GetPresetSliderStrings", PapyrusClass, GetPresetSliderStrings);

    return true;
}
