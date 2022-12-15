scriptName BodyslideReader hidden
{
AUTHOR: Gaz
PURPOSE: Allows Papyrus scripts to read data from BodySlide Preset files found in Data\CalienteTools\BodySlide\SliderPresets
CREDIT: Massive thanks to CharmedBaryon and his series of example and tutorial CommonLib-NG projects, as well as RocketBun for much of the parse code.
}

;Clears any pre-existing internal preset data, then parses the entire Preset folder again.
;BodyslideReader scans for Presets on game init, so use this if you suspect Presets changed mid-session or the plugin data went bad.
function ReloadPresets() global native

;Returns String Array of all Preset filenames matching the specified gender, without file format suffix.
String[] function GetPresetList(Bool abFemale) global native

;Returns String Array of all sliders used by a given preset.
String[] function GetPresetSliderStrings(String asPresetName) global native

;Returns Float Array of all sliders used by a given preset at 0 Weight values. 
;Intended to be used with GetPresetSliderStrings to be meaningful.
Float[] function GetPresetSliderLows(String asPresetName) global native

;Returns Float Array of all sliders used by a given preset at 100 Weight values. 
;Intended to be used with GetPresetSliderStrings to be meaningful.
Float[] function GetPresetSliderHighs(String asPresetName) global native
