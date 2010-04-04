/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "jucer_ComponentDocument.h"
#include "Component Types/jucer_TextButton.h"
#include "Component Types/jucer_ToggleButton.h"


//==============================================================================
static const char* const componentDocumentTag   = "COMPONENT";
static const char* const componentGroupTag      = "COMPONENTS";

static const char* const idProperty             = "id";
static const char* const compBoundsProperty     = "position";
static const char* const memberNameProperty     = "memberName";
static const char* const compNameProperty       = "name";

static const char* const metadataTagStart       = "JUCER_" "COMPONENT_METADATA_START"; // written like this to avoid thinking this file is a component!
static const char* const metadataTagEnd         = "JUCER_" "COMPONENT_METADATA_END";

//==============================================================================
static const String componentBoundsToString (const Rectangle<int>& bounds)
{
    return bounds.toString();
}

static const Rectangle<int> stringToComponentBounds (const String& s)
{
    return Rectangle<int>::fromString (s);
}

//==============================================================================
ComponentTypeHandler::ComponentTypeHandler (const String& name_, const String& xmlTag_,
                                            const String& memberNameRoot_)
    : name (name_), xmlTag (xmlTag_),
      memberNameRoot (memberNameRoot_)
{
}

ComponentTypeHandler::~ComponentTypeHandler()
{
}

void ComponentTypeHandler::updateComponent (Component* comp, const ValueTree& state)
{
    comp->setBounds (stringToComponentBounds (state [compBoundsProperty]));
    comp->setName (state [compNameProperty]);
}

const ValueTree ComponentTypeHandler::createNewItem (ComponentDocument& document)
{
    ValueTree v (getXmlTag());
    v.setProperty (idProperty, createAlphaNumericUID(), 0);
    v.setProperty (compNameProperty, String::empty, 0);
    v.setProperty (memberNameProperty, document.getNonExistentMemberName (getMemberNameRoot()), 0);
    v.setProperty (compBoundsProperty,
                   componentBoundsToString (getDefaultSize().withPosition (Point<int> (Random::getSystemRandom().nextInt (100) + 100,
                                                                                       Random::getSystemRandom().nextInt (100) + 100))), 0);
    return v;
}


//==============================================================================
class ComponentTypeManager  : public DeletedAtShutdown
{
public:
    ComponentTypeManager()
    {
        handlers.add (new TextButtonHandler());
        handlers.add (new ToggleButtonHandler());
    }

    ~ComponentTypeManager()
    {
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (ComponentTypeManager);

    Component* createFromStoredType (const ValueTree& value)
    {
        ComponentTypeHandler* handler = getHandlerFor (value.getType());
        if (handler == 0)
            return 0;

        Component* c = handler->createComponent();
        if (c != 0)
            handler->updateComponent (c, value);

        return c;
    }

    ComponentTypeHandler* getHandlerFor (const String& type)
    {
        for (int i = handlers.size(); --i >= 0;)
            if (handlers.getUnchecked(i)->getXmlTag() == type)
                return handlers.getUnchecked(i);

        return 0;
    }

    const StringArray getTypeNames() const
    {
        StringArray s;
        for (int i = 0; i < handlers.size(); ++i)
            s.add (handlers.getUnchecked(i)->getName());

        return s;
    }

    int getNumHandlers() const                                      { return handlers.size(); }
    ComponentTypeHandler* getHandler (const int index) const        { return handlers[index]; }

private:
    OwnedArray <ComponentTypeHandler> handlers;
};

juce_ImplementSingleton_SingleThreaded (ComponentTypeManager);


//==============================================================================
ComponentDocument::ComponentDocument (Project* project_, const File& cppFile_)
   : project (project_), cppFile (cppFile_), root (componentDocumentTag),
     changedSinceSaved (false)
{
    reload();
    checkRootObject();

    root.addListener (this);
}

ComponentDocument::~ComponentDocument()
{
    root.removeListener (this);
}

void ComponentDocument::beginNewTransaction()
{
    undoManager.beginNewTransaction();
}

void ComponentDocument::valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const var::identifier& property)
{
    changedSinceSaved = true;
}

void ComponentDocument::valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)
{
    changedSinceSaved = true;
}

void ComponentDocument::valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)
{
    changedSinceSaved = true;
}

bool ComponentDocument::isComponentFile (const File& file)
{
    if (! file.hasFileExtension (".cpp"))
        return false;

    InputStream* in = file.createInputStream();

    if (in != 0)
    {
        BufferedInputStream buf (in, 8192, true);

        while (! buf.isExhausted())
            if (buf.readNextLine().contains (metadataTagStart))
                return true;
    }

    return false;
}

void ComponentDocument::writeCode (OutputStream& cpp, OutputStream& header)
{
    cpp << "/**  */"
        << newLine << newLine;

    header << "/**  */"
           << newLine << newLine;
}

void ComponentDocument::writeMetadata (OutputStream& out)
{
    out << "#if 0" << newLine
        << "/** Jucer-generated metadata section - Edit this data at own risk!" << newLine
        << metadataTagStart << newLine << newLine;

    ScopedPointer<XmlElement> xml (root.createXml());
    jassert (xml != 0);

    if (xml != 0)
        xml->writeToStream (out, String::empty, false, false);

    out << newLine
        << metadataTagEnd << " */" << newLine
        << "#endif" << newLine;
}

bool ComponentDocument::save()
{
    MemoryOutputStream cpp, header;
    writeCode (cpp, header);
    writeMetadata (cpp);

    bool savedOk = overwriteFileWithNewDataIfDifferent (cppFile, cpp)
                    && overwriteFileWithNewDataIfDifferent (cppFile.withFileExtension (".h"), header);

    if (savedOk)
        changedSinceSaved = false;

    return savedOk;
}

bool ComponentDocument::reload()
{
    String xmlString;

    {
        InputStream* in = cppFile.createInputStream();

        if (in == 0)
            return false;

        BufferedInputStream buf (in, 8192, true);
        String::Concatenator xml (xmlString);

        while (! buf.isExhausted())
        {
            String line (buf.readNextLine());

            if (line.contains (metadataTagStart))
            {
                while (! buf.isExhausted())
                {
                    line = buf.readNextLine();
                    if (line.contains (metadataTagEnd))
                        break;

                    xml.append (line);
                    xml.append (newLine);
                }

                break;
            }
        }
    }

    XmlDocument doc (xmlString);
    ScopedPointer<XmlElement> xml (doc.getDocumentElement());

    if (xml != 0 && xml->hasTagName (componentDocumentTag))
    {
        ValueTree newTree (ValueTree::fromXml (*xml));

        if (newTree.isValid())
        {
            root = newTree;
            checkRootObject();
            undoManager.clearUndoHistory();
            changedSinceSaved = false;
            return true;
        }
    }

    return false;
}

bool ComponentDocument::hasChangedSinceLastSave()
{
    return changedSinceSaved;
}

void ComponentDocument::checkRootObject()
{
    jassert (root.hasType (componentDocumentTag));

    if (! getComponentGroup().isValid())
        root.addChild (ValueTree (componentGroupTag), -1, 0);

    if (getClassName().toString().isEmpty())
        getClassName() = "NewComponent";
}

//==============================================================================
const int menuItemOffset = 0x63451fa4;

void ComponentDocument::addNewComponentMenuItems (PopupMenu& menu) const
{
    const StringArray typeNames (ComponentTypeManager::getInstance()->getTypeNames());

    for (int i = 0; i < typeNames.size(); ++i)
        menu.addItem (i + menuItemOffset, "New " + typeNames[i]);
}

void ComponentDocument::performNewComponentMenuItem (int menuResultCode)
{
    const StringArray typeNames (ComponentTypeManager::getInstance()->getTypeNames());

    if (menuResultCode >= menuItemOffset && menuResultCode < menuItemOffset + typeNames.size())
    {
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandler (menuResultCode - menuItemOffset);
        jassert (handler != 0);

        if (handler != 0)
            getComponentGroup().addChild (handler->createNewItem (*this), -1, getUndoManager());
    }
}

//==============================================================================
ValueTree ComponentDocument::getComponentGroup() const
{
    return root.getChildWithName (componentGroupTag);
}

int ComponentDocument::getNumComponents() const
{
    return getComponentGroup().getNumChildren();
}

const ValueTree ComponentDocument::getComponent (int index) const
{
    return getComponentGroup().getChild (index);
}

const ValueTree ComponentDocument::getComponentWithMemberName (const String& name) const
{
    const ValueTree comps (getComponentGroup());

    for (int i = comps.getNumChildren(); --i >= 0;)
    {
        const ValueTree v (comps.getChild(i));
        if (v [memberNameProperty] == name)
            return v;
    }

    return ValueTree::invalid;
}

Component* ComponentDocument::createComponent (int index) const
{
    const ValueTree v (getComponentGroup().getChild (index));

    if (v.isValid())
    {
        Component* c = ComponentTypeManager::getInstance()->createFromStoredType (v);
        c->getProperties().set (idProperty, v[idProperty]);
        jassert (c->getProperties()[idProperty].toString().isNotEmpty());
        return c;
    }

    return 0;
}

void ComponentDocument::updateComponent (Component* comp) const
{
    const ValueTree v (getComponentState (comp));

    if (v.isValid())
    {
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandlerFor (v.getType());
        jassert (handler != 0);

        if (handler != 0)
            handler->updateComponent (comp, v);
    }
}

bool ComponentDocument::containsComponent (Component* comp) const
{
    const ValueTree comps (getComponentGroup());

    for (int i = 0; i < comps.getNumChildren(); ++i)
        if (isStateForComponent (comps.getChild(i), comp))
            return true;

    return false;
}

const ValueTree ComponentDocument::getComponentState (Component* comp) const
{
    jassert (comp != 0);
    const ValueTree comps (getComponentGroup());

    for (int i = 0; i < comps.getNumChildren(); ++i)
        if (isStateForComponent (comps.getChild(i), comp))
            return comps.getChild(i);

    jassertfalse;
    return ValueTree::invalid;
}

bool ComponentDocument::isStateForComponent (const ValueTree& storedState, Component* comp) const
{
    jassert (comp != 0);
    jassert (! storedState [idProperty].isVoid());
    return storedState [idProperty] == comp->getProperties() [idProperty];
}

const String ComponentDocument::getNonExistentMemberName (String suggestedName)
{
    suggestedName = makeValidCppIdentifier (suggestedName, false, true, false);
    const String original (suggestedName);
    int num = 1;

    while (getComponentWithMemberName (suggestedName).isValid())
    {
        suggestedName = original;
        while (String ("0123456789").containsChar (suggestedName.getLastCharacter()))
            suggestedName = suggestedName.dropLastCharacters (1);

        suggestedName << num++;
    }

    return suggestedName;
}

//==============================================================================
class ComponentDocument::DragHandler
{
public:
    DragHandler (ComponentDocument& document_,
                 const Array<Component*>& items,
                 const MouseEvent& e,
                 const ResizableBorderComponent::Zone& zone_)
        : document (document_),
          zone (zone_)
    {
        for (int i = 0; i < items.size(); ++i)
        {
            jassert (items.getUnchecked(i) != 0);
            const ValueTree v (document.getComponentState (items.getUnchecked(i)));
            draggedComponents.add (v);
            originalPositions.add (stringToComponentBounds (v [compBoundsProperty]));
        }

        document.beginNewTransaction();
    }

    ~DragHandler()
    {
        document.beginNewTransaction();
    }

    void drag (const MouseEvent& e)
    {
        document.getUndoManager()->undoCurrentTransactionOnly();

        for (int i = 0; i < draggedComponents.size(); ++i)
            move (draggedComponents.getReference(i), e.getOffsetFromDragStart(), originalPositions.getReference(i));
    }

    void move (ValueTree& v, const Point<int>& distance, const Rectangle<int>& originalPos)
    {
        Rectangle<int> newBounds (zone.resizeRectangleBy (originalPos, distance));
        v.setProperty (compBoundsProperty, componentBoundsToString (newBounds), document.getUndoManager());
    }

private:
    ComponentDocument& document;
    Array <ValueTree> draggedComponents;
    Array <Rectangle<int> > originalPositions;
    const ResizableBorderComponent::Zone zone;
};

void ComponentDocument::beginDrag (const Array<Component*>& items, const MouseEvent& e, const ResizableBorderComponent::Zone& zone)
{
    dragger = new DragHandler (*this, items, e, zone);
}

void ComponentDocument::continueDrag (const MouseEvent& e)
{
    if (dragger != 0)
        dragger->drag (e);
}

void ComponentDocument::endDrag (const MouseEvent& e)
{
    if (dragger != 0)
    {
        dragger->drag (e);
        dragger = 0;
    }
}