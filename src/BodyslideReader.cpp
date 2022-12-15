#include <SKSE/SKSE.h>
#include <BodyslideReader.h>
#include <rapidxml.hpp>
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <string>
#include <vector>
#include <random>

using namespace rapidxml;
using namespace RE;
using namespace Primary;
using namespace SKSE;

const std::vector<std::string> DefaultSliders = {"Breasts",   "BreastsSmall", "NippleDistance", "NippleSize",
                                                 "ButtCrack", "Butt",         "ButtSmall",      "Legs",
                                                 "Arms",      "ShoulderWidth"};

PresetContainer* PresetContainer::GetInstance() {
    static PresetContainer instance;
    return std::addressof(instance);
}

namespace Presets {

    // Code in this namespace taken from AutoBodyAE and mostly unaltered, thank you!

    // today i learned about LNK2005.
    bodypreset notfound{};
    std::vector<slider> nullsliders;
    std::vector<bodypreset> vecNotFound;
    

    bodypreset FindPresetByName(std::vector<bodypreset> searchable, std::string name) {
        logger::trace("entered findpresetbyname");
        // logger::debug("the searchable set has {} elements in it.", searchable.size());
        for (bodypreset searchitem : searchable) {
            // logger::debug("the items name is {} and the search target is {}", searchitem.name, name);
            if (searchitem.name == name) {
                logger::trace("{} was found!", searchitem.name);
                return searchitem;
            }
        }
        // logger::info("Preset could not be found!");
        return notfound;
    }

    // everything to do with managing external files.
    namespace Parsing {
        // utility functions for morphs.ini parsing
        // https://stackoverflow.com/questions/12966957/
        std::vector<std::string> explode(std::string const& s, char delim) {
            std::vector<std::string> result;
            std::istringstream iss(s);

            for (std::string token; std::getline(iss, token, delim);) {
                result.push_back(std::move(token));
            }
            return result;
        }

        bool contains(std::vector<std::string> input, std::vector<std::string> comparisonitems) {
            for (std::string i : input) {
                for (std::string j : comparisonitems) {
                    if (i.find(j) != -1) {
                        return true;
                    }
                }
            }
            return false;
        }

        std::string seek(std::vector<std::string> input, std::vector<std::string> comparisonitems) {
            for (std::string i : input) {
                for (std::string j : comparisonitems) {
                    if (i.find(j) != -1) {
                        return i;
                    }
                }
            }
            return "";
        }

        void PrintPreset(bodypreset preset) {
            logger::info("Now displaying preset: {}", preset.name);
            for (slider bar : preset.sliderlist) {
                logger::info(" > {}: [Small: {}] [Big: {}]", bar.name, bar.min, bar.max);
            }
        }

        void PrintPresetList(std::vector<bodypreset> setofsets) {
            logger::trace("Trying to print the preset list.");
            for (bodypreset body : setofsets) {
                logger::info("[ {} ] Number of sliders: [ {} ]", body.name, body.sliderlist.size());
            }
        }

        // takes the list as a pointer
        void ParsePreset(std::string filename, std::vector<bodypreset>* femalelist, std::vector<bodypreset>* malelist) {
            std::string* presetname{new std::string};
            std::string* bodytype{new std::string};
            std::vector<std::string>* presetgroups{new std::vector<std::string>};

            xml_document<>* preset{new xml_document};
            xml_node<>* root_node = NULL;

            logger::trace("Beginning xml parse...");

            // suck the xml into the buffer
            std::ifstream inputFile(filename);
            std::vector<char> buffer((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
            buffer.push_back('\0');

            // parse the buffer
            preset->parse<0>(&buffer[0]);

            logger::trace("Buffer has been parsed.");
            //  find the root
            root_node = preset->first_node("SliderPresets");
            if (!root_node) {
                logger::trace("Empty xml! Ignoring.");
                return;
            }
            // parse!
            // for each preset, grab its name and go through the sliders.
            for (xml_node<>* preset_node = root_node->first_node("Preset"); preset_node;
                 preset_node = preset_node->next_sibling("Preset")) {
                std::vector<slider>* sliderset{new std::vector<slider>};

                // we need this for the set of sets, to identify them.
                *presetname = preset_node->first_attribute()->value();

                // fill the group vector with groups to identify the sex of the preset.
                for (xml_node<>* cat_node = preset_node->first_node("Group"); cat_node;
                     cat_node = cat_node->next_sibling("Group")) {
                    logger::trace("Adding {} to group list!", cat_node->first_attribute()->value());
                    presetgroups->push_back(cat_node->first_attribute()->value());
                }

                logger::trace("preset name being loaded is {}", *presetname);
                std::vector<std::string> discardkeywords{"Cloth",  "cloth",    "Outfit", "outfit", "NeverNude",
                                                         "Bikini", "Feet",     "Hands",  "OUTFIT", "push",
                                                         "Push",   "Cleavage", "Armor",  "Bra"};
                // discard clothed presets.
                if (Presets::Parsing::contains({*presetname}, discardkeywords)) {
                    logger::trace("Clothed preset found. Discarding.");
                    continue;
                }

                // for finding if we need to check for inverted sliders
                bool UUNP = false;
                *bodytype = preset_node->last_attribute()->value();

                if (bodytype->find("UNP") != -1) {
                    logger::trace("UUNP preset found. Now inverting sliders.");
                    UUNP = true;
                }

                // logger::debug("{} has entered the outer for loop", *presetname);
                std::string* slidername{new std::string};
                std::string* previousslidername{new std::string("")};

                std::string* size{new std::string("")};
                std::string* lastSize{new std::string("")};
                float defaultvalue = 0;
                float* sizevalue{new float{0}};
                float* lastsizevalue{new float{0}};
                // go through the sliders
                for (xml_node<>* slider_node = preset_node->first_node("SetSlider"); slider_node;
                     slider_node = slider_node->next_sibling("SetSlider")) {
                    // store the current values of this node
                    *slidername = slider_node->first_attribute()->value();
                    *size = slider_node->first_attribute("size")->value();
                    auto printable = *slidername;
                    // logger::debug("The slider being looked at is {} and it is {}", printable, *size);
                    //  convert the size to a morphable value (those are -1 to 1.)
                    *sizevalue = std::stoi(slider_node->first_attribute("value")->value()) / 100.0f;
                    // logger::debug("The converted value of that slider is {}", *sizevalue);
                    // if we detect that a preset is UUNP based, invert the sliders.
                    bool inverted = false;
                    if (UUNP) {
                        for (std::string slider : DefaultSliders) {
                            if (*slidername == slider) {
                                logger::trace("UUNP backwards slider found! Inverting...");
                                *sizevalue = 1.0f - *sizevalue;
                                inverted = true;
                                break;
                            }
                        }
                    }
                    // if a pair is found, push it into the sliderset vector as a full struct.
                    if (*slidername == *previousslidername) {
                        // logger::debug("{} is a paired slider and is being pushed back with a pair of values of {} and
                        // {}", *slidername, *lastsizevalue, *sizevalue);

                        sliderset->push_back({*lastsizevalue, *sizevalue, *slidername});
                    }

                    // if a pair is not found, evaluate which half it is and then push it as a
                    // full struct with default values where they belong.
                    else if (*slidername != *previousslidername) {
                        if (!slider_node->next_sibling() ||
                            slider_node->next_sibling()->first_attribute()->value() != *slidername) {
                            // logger::debug("slider {} is a singlet", *slidername);
                            if (inverted) {
                                defaultvalue -= 1.0f;
                            }

                            if (*size == "big") {
                                // logger::trace("It's a biggun");
                                sliderset->push_back({*sizevalue, defaultvalue, *slidername});
                            } else {
                                // logger::trace("It's a smallun;");
                                sliderset->push_back({defaultvalue, *sizevalue, *slidername});
                            }
                            // yet another inversion check for UUNP support.
                        }
                    }

                    *previousslidername = *slidername;
                    *lastSize = *size;
                    *lastsizevalue = *sizevalue;
                    // logger::debug("At the end of pushback we have a slider name of {} and a value of {}",
                    // *slidername, *sizevalue);
                    //  std::cout << " values: " << slidername << ",  " << size << ",  " <<
                    //  sizevalue << std::endl;
                }

                // clean up memory
                delete slidername;
                delete previousslidername;
                delete sizevalue;
                delete lastsizevalue;
                delete size;
                delete lastSize;

                slidername = nullptr;
                previousslidername = nullptr;
                sizevalue = nullptr;
                lastsizevalue = nullptr;
                size = nullptr;
                lastSize = nullptr;

                // push the preset into the master list, then erase the sliderset to startagain.
                bool fail = true;
                for (std::string item : *presetgroups) {
                    if (item.find("CBBE") != -1 || item.find("3BBB") != -1 || item.find("CBAdvanced") != -1 ||
                        (item.find("UNP") != -1) || item.find("TBD") != -1) {
                        logger::info("{} has been added to the female preset list.", *presetname);
                        fail = false;
                        femalelist->push_back(bodypreset{*sliderset, *presetname});
                        break;
                    } else if (item.find("HIMBO") != -1 || item.find("SOS") != -1 || item.find("SAM") != -1) {
                        logger::info("{} has been added to the male preset list.", *presetname);
                        fail = false;
                        malelist->push_back(bodypreset{*sliderset, *presetname});
                        break;
                    }
                }
                if (fail) {
                    logger::info("{} is not assigned to any supported preset groups! It will not be loaded.",
                                 *presetname);
                }
                delete sliderset;
            }
            //  clean up memory
            delete presetgroups;
            presetgroups = nullptr;

            return;
        }

        // finds all the xmls in a target folder, and parses them into an xml vector.
        void ParseAllInFolder(std::string path, std::vector<bodypreset>* femalelist,
                              std::vector<bodypreset>* malelist) {
            std::string filename;
            for (auto const& dir_entry : std::filesystem::directory_iterator(path)) {
                filename = dir_entry.path().string();
                if (filename.substr(filename.find_last_of(".") + 1) == "xml") {
                    size_t eraseamount = filename.find_last_of("/");
                    filename.erase(0, eraseamount + 1);

                    logger::trace("Now parsing {}...", filename);
                    ParsePreset(filename, femalelist, malelist);
                }
            }

            return;
        }

        std::string CheckConfig() {
            // we return this at the end to help the parser find the path.
            std::string presetPath;

            presetPath = "Data\\CalienteTools\\BodySlide\\SliderPresets";
 

            return presetPath;
        }

    }  // namespace Parsing

}  // namespace Presets

BodyslideReader& BodyslideReader::GetSingleton() noexcept {
    static BodyslideReader instance;
    return instance;
}

void BodyslideReader::ReloadPresets() {
    std::unique_lock lock(_lock);
    auto presetcontainer = PresetContainer::GetInstance();
    std::vector<bodypreset> newMasterSet;
    presetcontainer->femaleMasterSet = newMasterSet;
    presetcontainer->maleMasterSet = newMasterSet;
    //logger::info("female master size: {}", presetcontainer->femaleMasterSet.size());
    Presets::Parsing::ParseAllInFolder(Presets::Parsing::CheckConfig(), &presetcontainer->femaleMasterSet,
                                       &presetcontainer->maleMasterSet);
    logger::info("ReloadPresets {} presets were loaded into the female list.", presetcontainer->femaleMasterSet.size());
    logger::info("ReloadPresets {} presets were loaded into the male list.", presetcontainer->maleMasterSet.size());
        return;
}

std::vector<std::float_t> BodyslideReader::GetPresetSliderLows(std::string name) {
        std::unique_lock lock(_lock);

        auto presetcontainer = PresetContainer::GetInstance();

        logger::info("GetPresetSliderLows called and grabbed container.");
        std::vector<float_t> returnarray;

        for (int i = 0; i < presetcontainer->femaleMasterSet.size(); ++i) {
        if (presetcontainer->femaleMasterSet[i].name == name) {
            logger::info("GetPresetSliderLows Passed female compare!");
            bodypreset preset = (presetcontainer->femaleMasterSet[i]);
            for (slider bar : preset.sliderlist) {
                returnarray.push_back(bar.min);
            }
            return returnarray;
            }
        }
         
        for (int i = 0; i < presetcontainer->maleMasterSet.size(); ++i) {
        if (presetcontainer->maleMasterSet[i].name == name) {
            logger::info("GetPresetSliderLows Passed male compare!");
            bodypreset preset = (presetcontainer->maleMasterSet[i]);
            for (slider bar : preset.sliderlist) {
                returnarray.push_back(bar.min);
            }
            return returnarray;
        }  
        }
        
        logger::info("GetPresetSliderLows hit failsafe return. No data found.");
        return returnarray;
}

std::vector<std::float_t> BodyslideReader::GetPresetSliderHighs(std::string name) {
        std::unique_lock lock(_lock);

        auto presetcontainer = PresetContainer::GetInstance();

        logger::info("GetPresetSliderHighs called and grabbed container.");
        std::vector<float_t> returnarray;

        for (int i = 0; i < presetcontainer->femaleMasterSet.size(); ++i) {
        if (presetcontainer->femaleMasterSet[i].name == name) {
            logger::info("GetPresetSliderHighs Passed female compare!");
            bodypreset preset = (presetcontainer->femaleMasterSet[i]);
            for (slider bar : preset.sliderlist) {
                returnarray.push_back(bar.max);
            }
            return returnarray;
        }
        }
        
        for (int i = 0; i < presetcontainer->maleMasterSet.size(); ++i) {
        if (presetcontainer->maleMasterSet[i].name == name) {
            logger::info("GetPresetSliderHighs Passed male compare!");
            bodypreset preset = (presetcontainer->maleMasterSet[i]);
            for (slider bar : preset.sliderlist) {
                returnarray.push_back(bar.max);
            }
            return returnarray;
        }
        }
        logger::info("GetPresetSliderHighs hit failsafe return. No data found.");
        return returnarray;
}

std::vector<std::string> BodyslideReader::GetPresetSliderStrings(std::string name) {
        std::unique_lock lock(_lock);

        auto presetcontainer = PresetContainer::GetInstance();

        logger::info("GetPresetSliderStrings called and grabbed container.");
        std::vector<std::string> returnarray;

        for (int i = 0; i < presetcontainer->femaleMasterSet.size(); ++i) {
        if (presetcontainer->femaleMasterSet[i].name == name) {
            logger::info("GetPresetSliderStrings Passed female compare!");
            bodypreset preset = (presetcontainer->femaleMasterSet[i]);
            for (slider bar : preset.sliderlist) {
                returnarray.push_back(bar.name);
            }
            return returnarray;
        }
        }

        for (int i = 0; i < presetcontainer->maleMasterSet.size(); ++i) {
        if (presetcontainer->maleMasterSet[i].name == name) {
            logger::info("GetPresetSliderStrings Passed male compare!");
            bodypreset preset = (presetcontainer->maleMasterSet[i]);
            for (slider bar : preset.sliderlist) {
                returnarray.push_back(bar.name);
            }
            return returnarray;
        }
        }

        logger::info("GetPresetSliderStrings hit failsafe return. No data found.");
        return returnarray;
}

std::vector<std::string> BodyslideReader::GetPresetList(bool female) { 
    std::unique_lock lock(_lock);

    auto presetcontainer = PresetContainer::GetInstance();

    logger::info("GetPresetList called and grabbed container.");
    std::vector<std::string> returnarray;
    if (female == true) {
        logger::info("GetPresetList Passed female bool!");
        for (int i = 0; i < presetcontainer->femaleMasterSet.size(); ++i) {
            bodypreset preset = (presetcontainer->femaleMasterSet[i]);
            returnarray.push_back(preset.name);
        }
        //logger::info("GetPresetList Passed female For with array of size: {}", returnarray.size());
    } 
    else if (female == false) {
        logger::info("GetPresetList Passed male bool!");
        for (int i = 0; i < presetcontainer->maleMasterSet.size(); ++i) {
            bodypreset preset = (presetcontainer->maleMasterSet[i]);
            returnarray.push_back(preset.name);
        }
        //logger::info("GetPresetList Passed male For with array of size: {}", returnarray.size());
    }
    logger::info("GetPresetList bool: {} returning an array with {} presets.", female, returnarray.size());
    return returnarray;

}