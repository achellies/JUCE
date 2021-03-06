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

#ifndef __JUCER_GENERATEDCODE_JUCEHEADER__
#define __JUCER_GENERATEDCODE_JUCEHEADER__

class JucerDocument;


//==============================================================================
/**
    A class for collecting the various snippets of c++ that will be assembled into
    the final cpp and header files.
*/
class GeneratedCode
{
public:
    GeneratedCode (const JucerDocument*);
    ~GeneratedCode();

    //==============================================================================
    void applyToCode (String& code,
                      const String& fileNameRoot,
                      const bool isForPreview,
                      const String& oldFileWithUserData = String::empty) const;

    int getUniqueSuffix();

    //==============================================================================
    const JucerDocument* const document;

    String className;
    String componentName;
    String parentClassInitialiser;  // optional parent class initialiser to go before the items in the initialisers list
    StringArray initialisers; // (a list of the member variables that need initalising after the constructor declaration)
    String parentClasses;
    String constructorParams;
    String privateMemberDeclarations;
    String publicMemberDeclarations;
    StringArray includeFilesH, includeFilesCPP;
    String constructorCode;
    String destructorCode;
    String staticMemberDefinitions;
    String jucerMetadata;

    struct CallbackMethod
    {
        String requiredParentClass;
        String returnType;
        String prototype;
        String content;
        bool hasPrePostUserSections;
    };

    OwnedArray <CallbackMethod> callbacks;

    String& getCallbackCode (const String& requiredParentClass,
                             const String& returnType,
                             const String& prototype,
                             const bool hasPrePostUserSections);

    void removeCallback (const String& returnType, const String& prototype);

    void addImageResourceLoader (const String& imageMemberName, const String& resourceName);

    String getCallbackDeclarations() const;
    String getCallbackDefinitions() const;
    StringArray getExtraParentClasses() const;

private:
    String getClassDeclaration() const;
    String getInitialiserList() const;
    int suffix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratedCode)
};


#endif   // __JUCER_GENERATEDCODE_JUCEHEADER__
