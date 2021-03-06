/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef __JUCER_SOURCECODEEDITOR_JUCEHEADER__
#define __JUCER_SOURCECODEEDITOR_JUCEHEADER__

#include "../Project/jucer_Project.h"
#include "../Application/jucer_DocumentEditorComponent.h"


//==============================================================================
class SourceCodeDocument  : public OpenDocumentManager::Document
{
public:
    SourceCodeDocument (Project*, const File&);

    bool loadedOk() const                           { return true; }
    bool isForFile (const File& file) const         { return getFile() == file; }
    bool isForNode (const ValueTree&) const         { return false; }
    bool refersToProject (Project& p) const         { return project == &p; }
    Project* getProject() const                     { return project; }
    String getName() const                          { return getFile().getFileName(); }
    String getType() const                          { return getFile().getFileExtension() + " file"; }
    File getFile() const                            { return modDetector.getFile(); }
    bool needsSaving() const                        { return codeDoc != nullptr && codeDoc->hasChangedSinceSavePoint(); }
    bool hasFileBeenModifiedExternally()            { return modDetector.hasBeenModified(); }
    void fileHasBeenRenamed (const File& newFile)   { modDetector.fileHasBeenRenamed (newFile); }
    String getState() const                         { return lastState != nullptr ? lastState->toString() : String::empty; }
    void restoreState (const String& state)         { lastState = new CodeEditorComponent::State (state); }

    File getCounterpartFile() const
    {
        const File file (getFile());

        if (file.hasFileExtension ("cpp;c;mm;m"))
        {
            const char* extensions[] = { "h", "hpp", nullptr };
            return findCounterpart (file, extensions);
        }

        if (file.hasFileExtension ("h;hpp"))
        {
            const char* extensions[] = { "cpp", "mm", "cc", "cxx", "c", "m", nullptr };
            return findCounterpart (file, extensions);
        }

        return File::nonexistent;
    }

    static File findCounterpart (const File& file, const char** extensions)
    {
        while (*extensions != nullptr)
        {
            const File f (file.withFileExtension (*extensions++));

            if (f.existsAsFile())
                return f;
        }

        return File::nonexistent;
    }

    void reloadFromFile();
    bool save();
    bool saveAs();

    Component* createEditor();
    Component* createViewer()       { return createEditor(); }

    void updateLastState (CodeEditorComponent& editor);
    void applyLastState (CodeEditorComponent& editor) const;

    CodeDocument& getCodeDocument();

    //==============================================================================
    struct Type  : public OpenDocumentManager::DocumentType
    {
        bool canOpenFile (const File& file)
        {
            if (file.hasFileExtension ("cpp;h;hpp;mm;m;c;cc;cxx;txt;inc;tcc;xml;plist;rtf;html;htm;php;py;rb;cs"))
                return true;

            MemoryBlock mb;
            if (file.loadFileAsData (mb)
                 && seemsToBeText (static_cast <const char*> (mb.getData()), (int) mb.getSize())
                 && ! file.hasFileExtension ("svg"))
                return true;

            return false;
        }

        static bool seemsToBeText (const char* const chars, const int num) noexcept
        {
            for (int i = 0; i < num; ++i)
            {
                const char c = chars[i];
                if ((c < 32 && c != '\t' && c != '\r' && c != '\n') || chars[i] > 126)
                    return false;
            }

            return true;
        }

        Document* openFile (Project* p, const File& file)   { return new SourceCodeDocument (p, file); }
    };

protected:
    FileModificationDetector modDetector;
    ScopedPointer<CodeDocument> codeDoc;
    Project* project;

    ScopedPointer<CodeEditorComponent::State> lastState;

    void reloadInternal();
};

//==============================================================================
class SourceCodeEditor  : public DocumentEditorComponent,
                          private ValueTree::Listener,
                          private CodeDocument::Listener
{
public:
    SourceCodeEditor (OpenDocumentManager::Document* document, CodeDocument&);
    SourceCodeEditor (OpenDocumentManager::Document* document, CodeEditorComponent*);
    ~SourceCodeEditor();

    void scrollToKeepRangeOnScreen (Range<int> range);
    void highlight (Range<int> range, bool cursorAtStart);

    ScopedPointer<CodeEditorComponent> editor;

private:
    void resized();

    void valueTreePropertyChanged (ValueTree&, const Identifier&);
    void valueTreeChildAdded (ValueTree&, ValueTree&);
    void valueTreeChildRemoved (ValueTree&, ValueTree&);
    void valueTreeChildOrderChanged (ValueTree&);
    void valueTreeParentChanged (ValueTree&);
    void valueTreeRedirected (ValueTree&);

    void codeDocumentTextInserted (const String&, int);
    void codeDocumentTextDeleted (int, int);

    void setEditor (CodeEditorComponent*);
    void updateColourScheme();
    void checkSaveState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceCodeEditor)
};


//==============================================================================
class GenericCodeEditorComponent  : public CodeEditorComponent
{
public:
    GenericCodeEditorComponent (const File&, CodeDocument&, CodeTokeniser*);
    ~GenericCodeEditorComponent();

    void addPopupMenuItems (PopupMenu&, const MouseEvent*);
    void performPopupMenuAction (int menuItemID);

    void getAllCommands (Array<CommandID>&);
    void getCommandInfo (CommandID, ApplicationCommandInfo&);
    bool perform (const InvocationInfo&);

    void showFindPanel();
    void hideFindPanel();
    void findSelection();
    void findNext (bool forwards, bool skipCurrentSelection);
    void handleEscapeKey();

    void resized();

    static String getSearchString()                 { return getAppSettings().getGlobalProperties().getValue ("searchString"); }
    static void setSearchString (const String& s)   { getAppSettings().getGlobalProperties().setValue ("searchString", s); }
    static bool isCaseSensitiveSearch()             { return getAppSettings().getGlobalProperties().getBoolValue ("searchCaseSensitive"); }
    static void setCaseSensitiveSearch (bool b)     { getAppSettings().getGlobalProperties().setValue ("searchCaseSensitive", b); }

private:
    File file;
    class FindPanel;
    ScopedPointer<FindPanel> findPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericCodeEditorComponent)
};

//==============================================================================
class CppCodeEditorComponent  : public GenericCodeEditorComponent
{
public:
    CppCodeEditorComponent (const File& file, CodeDocument& codeDocument);
    ~CppCodeEditorComponent();

    void addPopupMenuItems (PopupMenu&, const MouseEvent*);
    void performPopupMenuAction (int menuItemID);

    void handleReturnKey();
    void insertTextAtCaret (const String& newText);

private:
    void insertComponentClass();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CppCodeEditorComponent)
};


#endif   // __JUCER_SOURCECODEEDITOR_JUCEHEADER__
