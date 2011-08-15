/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_AUDIOPLUGINMODULE_JUCEHEADER__
#define __JUCER_AUDIOPLUGINMODULE_JUCEHEADER__


//==============================================================================
namespace
{
    Value shouldBuildVST (const Project& project)                       { return project.getProjectValue ("buildVST"); }
    Value shouldBuildRTAS (const Project& project)                      { return project.getProjectValue ("buildRTAS"); }
    Value shouldBuildAU (const Project& project)                        { return project.getProjectValue ("buildAU"); }

    Value getPluginName (const Project& project)                        { return project.getProjectValue ("pluginName"); }
    Value getPluginDesc (const Project& project)                        { return project.getProjectValue ("pluginDesc"); }
    Value getPluginManufacturer (const Project& project)                { return project.getProjectValue ("pluginManufacturer"); }
    Value getPluginManufacturerCode (const Project& project)            { return project.getProjectValue ("pluginManufacturerCode"); }
    Value getPluginCode (const Project& project)                        { return project.getProjectValue ("pluginCode"); }
    Value getPluginChannelConfigs (const Project& project)              { return project.getProjectValue ("pluginChannelConfigs"); }
    Value getPluginIsSynth (const Project& project)                     { return project.getProjectValue ("pluginIsSynth"); }
    Value getPluginWantsMidiInput (const Project& project)              { return project.getProjectValue ("pluginWantsMidiIn"); }
    Value getPluginProducesMidiOut (const Project& project)             { return project.getProjectValue ("pluginProducesMidiOut"); }
    Value getPluginSilenceInProducesSilenceOut (const Project& project) { return project.getProjectValue ("pluginSilenceInIsSilenceOut"); }
    Value getPluginTailLengthSeconds (const Project& project)           { return project.getProjectValue ("pluginTailLength"); }
    Value getPluginEditorNeedsKeyFocus (const Project& project)         { return project.getProjectValue ("pluginEditorRequiresKeys"); }
    Value getPluginAUExportPrefix (const Project& project)              { return project.getProjectValue ("pluginAUExportPrefix"); }
    Value getPluginAUCocoaViewClassName (const Project& project)        { return project.getProjectValue ("pluginAUViewClass"); }
    Value getPluginRTASCategory (const Project& project)                { return project.getProjectValue ("pluginRTASCategory"); }

    int countMaxPluginChannels (const String& configString, bool isInput)
    {
        StringArray configs;
        configs.addTokens (configString, ", {}", String::empty);
        configs.trim();
        configs.removeEmptyStrings();
        jassert ((configs.size() & 1) == 0);  // looks like a syntax error in the configs?

        int maxVal = 0;
        for (int i = (isInput ? 0 : 1); i < configs.size(); i += 2)
            maxVal = jmax (maxVal, configs[i].getIntValue());

        return maxVal;
    }

    void writePluginCharacteristicsFile (ProjectSaver& projectSaver)
    {
        Project& project = projectSaver.getProject();

        MemoryOutputStream mem;

        mem << "//==============================================================================" << newLine
            << "// Audio plugin settings.." << newLine
            << newLine
            << "#define JucePlugin_Build_VST    " << ((bool) shouldBuildVST (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_Build_AU     " << ((bool) shouldBuildAU (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_Build_RTAS   " << ((bool) shouldBuildRTAS (project).getValue() ? 1 : 0) << newLine
            << newLine
            << "#define JucePlugin_Name                 "  << getPluginName (project).toString().quoted() << newLine
            << "#define JucePlugin_Desc                 "  << getPluginDesc (project).toString().quoted() << newLine
            << "#define JucePlugin_Manufacturer         "  << getPluginManufacturer (project).toString().quoted() << newLine
            << "#define JucePlugin_ManufacturerCode     '" << getPluginManufacturerCode (project).toString().trim().substring (0, 4) << "'" << newLine
            << "#define JucePlugin_PluginCode           '" << getPluginCode (project).toString().trim().substring (0, 4) << "'" << newLine
            << "#define JucePlugin_MaxNumInputChannels  "  << countMaxPluginChannels (getPluginChannelConfigs (project).toString(), true) << newLine
            << "#define JucePlugin_MaxNumOutputChannels "  << countMaxPluginChannels (getPluginChannelConfigs (project).toString(), false) << newLine
            << "#define JucePlugin_PreferredChannelConfigurations   " << getPluginChannelConfigs (project).toString() << newLine
            << "#define JucePlugin_IsSynth              "  << ((bool) getPluginIsSynth (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_WantsMidiInput       "  << ((bool) getPluginWantsMidiInput (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_ProducesMidiOutput   "  << ((bool) getPluginProducesMidiOut (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_SilenceInProducesSilenceOut  " << ((bool) getPluginSilenceInProducesSilenceOut (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_TailLengthSeconds    "  << (double) getPluginTailLengthSeconds (project).getValue() << newLine
            << "#define JucePlugin_EditorRequiresKeyboardFocus  " << ((bool) getPluginEditorNeedsKeyFocus (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_VersionCode          "  << project.getVersionAsHex() << newLine
            << "#define JucePlugin_VersionString        "  << project.getVersion().toString().quoted() << newLine
            << "#define JucePlugin_VSTUniqueID          JucePlugin_PluginCode" << newLine
            << "#define JucePlugin_VSTCategory          "  << ((bool) getPluginIsSynth (project).getValue() ? "kPlugCategSynth" : "kPlugCategEffect") << newLine
            << "#define JucePlugin_AUMainType           "  << ((bool) getPluginIsSynth (project).getValue() ? "kAudioUnitType_MusicDevice" : "kAudioUnitType_Effect") << newLine
            << "#define JucePlugin_AUSubType            JucePlugin_PluginCode" << newLine
            << "#define JucePlugin_AUExportPrefix       "  << getPluginAUExportPrefix (project).toString() << newLine
            << "#define JucePlugin_AUExportPrefixQuoted "  << getPluginAUExportPrefix (project).toString().quoted() << newLine
            << "#define JucePlugin_AUManufacturerCode   JucePlugin_ManufacturerCode" << newLine
            << "#define JucePlugin_CFBundleIdentifier   "  << project.getBundleIdentifier().toString() << newLine
            << "#define JucePlugin_AUCocoaViewClassName "  << getPluginAUCocoaViewClassName (project).toString() << newLine
            << "#define JucePlugin_RTASCategory         "  << ((bool) getPluginIsSynth (project).getValue() ? "ePlugInCategory_SWGenerators" : "ePlugInCategory_None") << newLine
            << "#define JucePlugin_RTASManufacturerCode JucePlugin_ManufacturerCode" << newLine
            << "#define JucePlugin_RTASProductId        JucePlugin_PluginCode" << newLine
            << newLine;

        projectSaver.setExtraAppConfigFileContent (mem.toString());
    }
}

//==============================================================================
namespace VSTHelpers
{
    static Value getVSTFolder (const ProjectExporter& exporter)         { return exporter.getSetting (Ids::vstFolder); }

    static void addVSTFolderToPath (const ProjectExporter& exporter, StringArray& searchPaths)
    {
        const String vstFolder (getVSTFolder (exporter).toString());

        if (vstFolder.isNotEmpty())
        {
            RelativePath path (exporter.rebaseFromProjectFolderToBuildTarget (RelativePath (vstFolder, RelativePath::projectFolder)));

            if (exporter.isVisualStudio())
                searchPaths.add (path.toWindowsStyle());
            else if (exporter.isLinux() || exporter.isXcode())
                searchPaths.insert (0, path.toUnixStyle());
        }
    }

    static void createVSTPathEditor (const ProjectExporter& exporter, Array <PropertyComponent*>& props)
    {
        props.add (new TextPropertyComponent (getVSTFolder (exporter), "VST Folder", 1024, false));
        props.getLast()->setTooltip ("If you're building a VST, this must be the folder containing the VST SDK. This should be an absolute path.");
    }

    static void fixMissingVSTValues (const ProjectExporter& exporter)
    {
        if (getVSTFolder(exporter).toString().isEmpty())
            getVSTFolder(exporter) = (exporter.isVisualStudio() ? "c:\\SDKs\\vstsdk2.4"
                                                                : "~/SDKs/vstsdk2.4");
    }

    static void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver)
    {
        fixMissingVSTValues (exporter);
        writePluginCharacteristicsFile (projectSaver);

        exporter.makefileTargetSuffix = ".so";

        Project::Item group (Project::Item::createGroup (const_cast<ProjectExporter&> (exporter).getProject(),
                                                         "Juce VST Wrapper", "__jucevstfiles"));

        RelativePath juceWrapperFolder (exporter.getProject().getGeneratedCodeFolder(),
                                        exporter.getTargetFolder(), RelativePath::buildTargetFolder);

        addVSTFolderToPath (exporter, exporter.extraSearchPaths);

        if (exporter.isVisualStudio())
            exporter.extraSearchPaths.add (juceWrapperFolder.toWindowsStyle());
        else if (exporter.isLinux())
            exporter.extraSearchPaths.add (juceWrapperFolder.toUnixStyle());
    }

    static void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props)
    {
        fixMissingVSTValues (exporter);
        createVSTPathEditor (exporter, props);
    }
}

//==============================================================================
namespace RTASHelpers
{
    static Value getRTASFolder (const ProjectExporter& exporter)        { return exporter.getSetting (Ids::rtasFolder); }

    static void fixMissingRTASValues (const ProjectExporter& exporter)
    {
        if (getRTASFolder (exporter).toString().isEmpty())
        {
            if (exporter.isVisualStudio())
                getRTASFolder (exporter) = "c:\\SDKs\\PT_80_SDK";
            else
                getRTASFolder (exporter) = "~/SDKs/PT_80_SDK";
        }
    }

    static void addExtraSearchPaths (ProjectExporter& exporter)
    {
        RelativePath rtasFolder (getRTASFolder (exporter).toString(), RelativePath::projectFolder);

        if (exporter.isVisualStudio())
        {
            RelativePath juceWrapperFolder (exporter.getProject().getGeneratedCodeFolder(),
                                            exporter.getTargetFolder(), RelativePath::buildTargetFolder);

            exporter.extraSearchPaths.add (juceWrapperFolder.toWindowsStyle());

            const char* p[] = { "AlturaPorts/TDMPlugins/PluginLibrary/EffectClasses",
                                "AlturaPorts/TDMPlugins/PluginLibrary/ProcessClasses",
                                "AlturaPorts/TDMPlugins/PluginLibrary/ProcessClasses/Interfaces",
                                "AlturaPorts/TDMPlugins/PluginLibrary/Utilities",
                                "AlturaPorts/TDMPlugins/PluginLibrary/RTASP_Adapt",
                                "AlturaPorts/TDMPlugins/PluginLibrary/CoreClasses",
                                "AlturaPorts/TDMPlugins/PluginLibrary/Controls",
                                "AlturaPorts/TDMPlugins/PluginLibrary/Meters",
                                "AlturaPorts/TDMPlugins/PluginLibrary/ViewClasses",
                                "AlturaPorts/TDMPlugins/PluginLibrary/DSPClasses",
                                "AlturaPorts/TDMPlugins/PluginLibrary/Interfaces",
                                "AlturaPorts/TDMPlugins/common",
                                "AlturaPorts/TDMPlugins/common/Platform",
                                "AlturaPorts/TDMPlugins/SignalProcessing/Public",
                                "AlturaPorts/TDMPlugIns/DSPManager/Interfaces",
                                "AlturaPorts/SADriver/Interfaces",
                                "AlturaPorts/DigiPublic/Interfaces",
                                "AlturaPorts/Fic/Interfaces/DAEClient",
                                "AlturaPorts/NewFileLibs/Cmn",
                                "AlturaPorts/NewFileLibs/DOA",
                                "AlturaPorts/AlturaSource/PPC_H",
                                "AlturaPorts/AlturaSource/AppSupport",
                                "AvidCode/AVX2sdk/AVX/avx2/avx2sdk/inc",
                                "xplat/AVX/avx2/avx2sdk/inc" };

            for (int i = 0; i < numElementsInArray (p); ++i)
                exporter.addToExtraSearchPaths (rtasFolder.getChildFile (p[i]));
        }
        else if (exporter.isXcode())
        {
            exporter.extraSearchPaths.add ("/Developer/Headers/FlatCarbon");

            const char* p[] = { "AlturaPorts/TDMPlugIns/PlugInLibrary/Controls",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/CoreClasses",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/DSPClasses",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/EffectClasses",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/MacBuild",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/Meters",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses/Interfaces",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/RTASP_Adapt",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/Utilities",
                                "AlturaPorts/TDMPlugIns/PlugInLibrary/ViewClasses",
                                "AlturaPorts/TDMPlugIns/DSPManager/**",
                                "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/Encryption",
                                "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/GraphicsExtensions",
                                "AlturaPorts/TDMPlugIns/common/**",
                                "AlturaPorts/TDMPlugIns/common/PI_LibInterface",
                                "AlturaPorts/TDMPlugIns/PACEProtection/**",
                                "AlturaPorts/TDMPlugIns/SignalProcessing/**",
                                "AlturaPorts/OMS/Headers",
                                "AlturaPorts/Fic/Interfaces/**",
                                "AlturaPorts/Fic/Source/SignalNets",
                                "AlturaPorts/DSIPublicInterface/PublicHeaders",
                                "DAEWin/Include",
                                "AlturaPorts/DigiPublic/Interfaces",
                                "AlturaPorts/DigiPublic",
                                "AlturaPorts/NewFileLibs/DOA",
                                "AlturaPorts/NewFileLibs/Cmn",
                                "xplat/AVX/avx2/avx2sdk/inc",
                                "xplat/AVX/avx2/avx2sdk/utils" };

            for (int i = 0; i < numElementsInArray (p); ++i)
                exporter.addToExtraSearchPaths (rtasFolder.getChildFile (p[i]));
        }
    }

    static void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver, const File& moduleFolder)
    {
        fixMissingRTASValues (exporter);

        exporter.xcodeCanUseDwarf = false;

        exporter.msvcTargetSuffix = ".dpm";
        exporter.msvcNeedsDLLRuntimeLib = true;

        RelativePath rtasFolder (getRTASFolder (exporter).toString(), RelativePath::projectFolder);
        exporter.msvcExtraPreprocessorDefs.set ("JucePlugin_WinBag_path", CodeHelpers::addEscapeChars (rtasFolder.getChildFile ("WinBag")
                                                                                .toWindowsStyle().quoted()));

        String msvcPathToRTASFolder (exporter.getJucePathFromTargetFolder()
                                             .getChildFile ("modules/juce_audio_plugin_client/RTAS")
                                             .toWindowsStyle() + "\\");

        exporter.msvcExtraLinkerOptions = "/FORCE:multiple";
        exporter.msvcDelayLoadedDLLs = "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; DSPManager.dll";
        exporter.msvcModuleDefinitionFile = msvcPathToRTASFolder + "juce_RTAS_WinExports.def";

        exporter.msvcPostBuildOutputs = "\"$(TargetPath)\".rsr";
        exporter.msvcPostBuildCommand = "copy /Y \"" + msvcPathToRTASFolder + "juce_RTAS_WinResources.rsr"
                                            + "\" \"$(TargetPath)\".rsr";

        exporter.xcodeExtraLibrariesDebug.add   (rtasFolder.getChildFile ("MacBag/Libs/Debug/libPluginLibrary.a"));
        exporter.xcodeExtraLibrariesRelease.add (rtasFolder.getChildFile ("MacBag/Libs/Release/libPluginLibrary.a"));

        writePluginCharacteristicsFile (projectSaver);

        addExtraSearchPaths (exporter);
    }

    static void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props)
    {
        if (exporter.isXcode() || exporter.isVisualStudio())
        {
            fixMissingRTASValues (exporter);

            props.add (new TextPropertyComponent (getRTASFolder (exporter), "RTAS Folder", 1024, false));
            props.getLast()->setTooltip ("If you're building an RTAS, this must be the folder containing the RTAS SDK. This should be an absolute path.");
        }
    }
}

//==============================================================================
namespace AUHelpers
{
    static void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver)
    {
        writePluginCharacteristicsFile (projectSaver);

        if (exporter.isXcode())
        {
            exporter.extraSearchPaths.add ("$(DEVELOPER_DIR)/Extras/CoreAudio/PublicUtility");
            exporter.extraSearchPaths.add ("$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/Utility");

            exporter.xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);

            Project::Item subGroup (projectSaver.getGeneratedCodeGroup().addNewSubGroup ("Juce AU Wrapper", -1));
            subGroup.setID ("__juceappleaufiles");

            {
                #define JUCE_AU_PUBLICUTILITY   "${DEVELOPER_DIR}/Extras/CoreAudio/PublicUtility/"
                #define JUCE_AU_PUBLIC          "${DEVELOPER_DIR}/Extras/CoreAudio/AudioUnits/AUPublic/"

                const char* appleAUFiles[] = {  JUCE_AU_PUBLICUTILITY "CADebugMacros.h",
                                                JUCE_AU_PUBLICUTILITY "CAAUParameter.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAAUParameter.h",
                                                JUCE_AU_PUBLICUTILITY "CAAudioChannelLayout.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAAudioChannelLayout.h",
                                                JUCE_AU_PUBLICUTILITY "CAMutex.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAMutex.h",
                                                JUCE_AU_PUBLICUTILITY "CAStreamBasicDescription.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAStreamBasicDescription.h",
                                                JUCE_AU_PUBLICUTILITY "CAVectorUnitTypes.h",
                                                JUCE_AU_PUBLICUTILITY "CAVectorUnit.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAVectorUnit.h",
                                                JUCE_AU_PUBLIC "AUViewBase/AUViewLocalizedStringKeys.h",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewDispatch.cpp",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewControl.cpp",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewControl.h",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/CarbonEventHandler.cpp",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/CarbonEventHandler.h",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewBase.cpp",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewBase.h",
                                                JUCE_AU_PUBLIC "AUBase/AUBase.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUBase.h",
                                                JUCE_AU_PUBLIC "AUBase/AUDispatch.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUDispatch.h",
                                                JUCE_AU_PUBLIC "AUBase/AUInputElement.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUInputElement.h",
                                                JUCE_AU_PUBLIC "AUBase/AUOutputElement.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUOutputElement.h",
                                                JUCE_AU_PUBLIC "AUBase/AUResources.r",
                                                JUCE_AU_PUBLIC "AUBase/AUScopeElement.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUScopeElement.h",
                                                JUCE_AU_PUBLIC "AUBase/ComponentBase.cpp",
                                                JUCE_AU_PUBLIC "AUBase/ComponentBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/AUMIDIBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/AUMIDIBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/AUMIDIEffectBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/AUMIDIEffectBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/AUOutputBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/AUOutputBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/MusicDeviceBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/MusicDeviceBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/AUEffectBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/AUEffectBase.h",
                                                JUCE_AU_PUBLIC "Utility/AUBuffer.cpp",
                                                JUCE_AU_PUBLIC "Utility/AUBuffer.h",
                                                JUCE_AU_PUBLIC "Utility/AUDebugDispatcher.cpp",
                                                JUCE_AU_PUBLIC "Utility/AUDebugDispatcher.h",
                                                JUCE_AU_PUBLIC "Utility/AUInputFormatConverter.h",
                                                JUCE_AU_PUBLIC "Utility/AUSilentTimeout.h",
                                                JUCE_AU_PUBLIC "Utility/AUTimestampGenerator.h", 0 };

                for (const char** f = appleAUFiles; *f != 0; ++f)
                {
                    const RelativePath file (*f, RelativePath::projectFolder);
                    subGroup.addRelativeFile (file, -1, file.hasFileExtension ("cpp;mm"));
                    subGroup.getChild (subGroup.getNumChildren() - 1).getShouldInhibitWarningsValue() = true;
                }
            }
        }
    }
}

#endif   // __JUCER_AUDIOPLUGINMODULE_JUCEHEADER__
