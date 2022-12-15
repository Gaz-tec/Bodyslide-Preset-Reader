#pragma once

#include <RE/Skyrim.h>
#include <rapidxml.hpp>

namespace Primary {
#pragma warning(push)
#pragma warning(disable : 4251)

    // raw slider struct
    struct slider {
        float max;
        float min;
        std::string name = "";
    };

    // raw body struct
    struct bodypreset {
        std::vector<slider> sliderlist;
        std::string name = "";
    };

    class PresetContainer {
    public:
        // Master lists of presets from Bodyslide folder. Thanks, AutoBodyAE!
        std::vector<bodypreset> femaleMasterSet;
        std::vector<bodypreset> maleMasterSet;

        PresetContainer() = default;

        // Singleton pattern.
        static PresetContainer* GetInstance();
    };

    namespace Parsing {
        void PrintPreset(bodypreset preset);

        void PrintPresetList(std::vector<bodypreset> setofsets);

        // takes the list as a pointer
        void ParsePreset(std::string filename, std::vector<bodypreset>* femalelist, std::vector<bodypreset>* malelist);

        // finds all the xmls in a target folder, and parses them into an xml vector.
        void ParseAllInFolder(std::string path, std::vector<bodypreset>* femalelist, std::vector<bodypreset>* malelist);

        std::string CheckConfig();

    }  // namespace Parsing

    class __declspec(dllexport) BodyslideReader {
    public:
        /**
         * Get the singleton instance of the <code>BodyslideReader</code>.
         */
        [[nodiscard]] static BodyslideReader& GetSingleton() noexcept;

        std::vector<bodypreset> femaleMasterSet;
        std::vector<bodypreset> maleMasterSet;

        /**
         * Tells the manager that a particular actor should have its hits tracked.
         *
         * @param actor The actor to track.
         * @return <code>true</code> if <code>actor</code> is a valid actor and is not already tracked, otherwise
         *         <code>false</code>.
         */
        void ReloadPresets();

        std::vector<float_t> GetPresetSliderLows(std::string name);

        /**
         * Tells the manager that a particular actor should no longer have its hits tracked.
         *
         * @param actor The actor to no longer track.
         * @return <code>true</code> if <code>actor</code> is a valid actor and was being tracked, otherwise
         *         <code>false</code>.
         */
        std::vector<float_t> GetPresetSliderHighs(std::string name);

        /**
         * Register a hit has occurred on an actor.
         *
         * <p>
         * This will have no effect if the actor is not valid or if the actor is not currently tracked.
         * </p>
         *
         * @param target The actor that was hit.
         */
       //  inline void RegisterHit(RE::Actor* target) noexcept { Increment(target); }

        /**
         * Increment the hit count on an actor.
         *
         * <p>
         * This will have no effect if the actor is not valid or if the actor is not currently tracked.
         * </p>
         *
         * @param target The actor whose hit count should be incremented.
         * @param by The amount by which to increment the hit count.
         */
        std::vector<std::string> GetPresetList(bool female);

        /**
         * Gets the current hit count for an actor.
         *
         * @param target The actor whose hit count should be returned.
         * @return Empty if the actor is not tracked, otherwise the number of hits that have been registered.
         */
        std::vector<std::string> GetPresetSliderStrings(std::string name);

        /**
         * The serialization handler for reverting game state.
         *
         * <p>
         * This is called as the handler for revert. Revert is called by SKSE on a plugin that is registered for
         * serialization handling on a new game or before a save game is loaded. It should be used to revert the state
         * of the plugin back to its default.
         * </p>
         */
        //static void OnRevert(SKSE::SerializationInterface*);

        /**
         * The serialization handler for saving data to the cosave.
         *
         * @param serde The serialization interface used to write data.
         */
        //static void OnGameSaved(SKSE::SerializationInterface* serde);

        /**
         * The serialization handler for loading data from a cosave.
         *
         * @param serde  The serialization interface used to read data.
         */
        //static void OnGameLoaded(SKSE::SerializationInterface* serde);

    private:
        BodyslideReader() = default;

        mutable std::mutex _lock;
        std::unordered_map<RE::Actor*, int32_t> _hitCounts;
        std::unordered_set<RE::Actor*> _trackedActors;
    };
#pragma warning(pop)
}  // namespace Primary
