#ifndef _INCLUDE_UTILS_TINYXML2UTILS_HPP_
#define _INCLUDE_UTILS_TINYXML2UTILS_HPP_

#include <vector>
#include <string>
#include <initializer_list>
#include <map>
#include <utility>
#include <type_traits>
#include <iostream>
#include <exception>
#include <tinyxml2.h>


namespace std {

//------------------------------------------------------------------------------
/// @brief      Split text string into multiple sub-strings according to a given delimiter character.
///
/// @param[in]  data       Input text string.
/// @param[in]  separator  Delimiter/separator character.
///
/// @return     Vector of words/tokens matching substrings delimited by separator char. Empty vector if separator is not found.
///
vector< string > tokenize(const string& data, char separator = ',', bool single_split = false) {
    vector< string > words;
    size_t start;
    size_t end = 0;

    while ((start = data.find_first_not_of(separator, end)) != std::string::npos) {
        end = data.find(separator, start);
        words.push_back(data.substr(start, end - start));
        if (single_split && end != string::npos) {
            words.push_back(data.substr(end + 1));
            break;
        }
    }

    return words;
}



//------------------------------------------------------------------------------
/// @brief      Replaces occurrences of "old_text" with "new_text" in given text string
///
/// @param      source    The source
/// @param[in]  old_text  The old text
/// @param[in]  new_text  The new text
///
/// @return     { description_of_the_return_value }
///
size_t replace(string* source, const string& old_text, const string& new_text) {
    size_t count = 0;
    auto pos = source->find(old_text);

    while (pos != std::string::npos) {
        source->replace(pos, old_text.size(), new_text);
        count++;
        pos = source->find(old_text);
    }

    return count;  // += replace(source.substr(pos), old_text, new_text);
}



//------------------------------------------------------------------------------
/// @brief      Moves a vector into another.
///
/// @param      container  Container where data is moved to.
/// @param      data       Data container holding data to be moved. Content is removed afterwards.
///
/// @tparam     T          Data type, common to both vectors.
///
/// @note       Implementation of move semantincs when appending data from a vector into another.
///             Useful when data is allocated to a temporary vector, reducing overhead/complexity.
///
/// @note       May not make much difference in some cases, as compilers implementing copy ellision and RVO should take care if it.
///
template < typename T >
void move_into(vector< T >& container, vector< T >& data) {
    if (container.empty()) {
        container = move(data);
    } else {
        container.reserve(container.size() + data.size());
        move(data.begin(), data.end(), back_inserter(container));
        data.clear();
    }
}


//------------------------------------------------------------------------------
/// @brief      Moves a vector into another.
///
/// @param      container  Container where data is moved to.
/// @param      data       Data container holding data to be moved. Content is removed afterwards.
///
/// @tparam     T          Data type, common to both vectors.
///
/// @note       Convenience overload of move_into for temporary types/rvalue references.
///
template < typename T >
void move_into(vector< T >& container, vector< T >&& data) {
    move_into(container, data);
}



}  // namespace std



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

namespace tinyxml2 {

namespace extra {


// //------------------------------------------------------------------------------
// /// @brief      Loads a XML file into a XMLDocument instance.
// ///
// /// @param[in]  path  Relative/absolute path to XML file.
// ///
// inline XMLDocument loadDocument(const std::string& path) {
//     XMLDocument document;
//     auto load_error = document.LoadFile(path.data());
//     if (load_error) {
//         throw std::invalid_argument("[" + std::string(__func__) + "] Unable to load data file: " + path);
//     }
//     return document;
// }


//------------------------------------------------------------------------------
/// @brief      Create a descriptor text string for a particular XML attribute
///
/// @param[in]  attribute  XML attribute
/// @param[in]  separator  Separator character. Defaults to ':'
///
/// @return     String formated as "name:value".
///
inline std::string descriptorOf(const XMLAttribute* attribute, char separator = ':') {
    return std::string(attribute->Name()) + separator + std::string(attribute->Value());
}



//------------------------------------------------------------------------------
/// @brief      Construts a formatted text string with the full family tree of a given XML element.
///
/// @param[in]  element       XML element. Specific to element nodes (only elements have names), thus XMLElement is used instead of XMLNode.
/// @param[in]  id_attribute  Name of the id/name attribute to use as identifier. Defaults to empty, wherein no identifiers is used.
///
/// @return     Text string formatted as "Root/Parent:ID_0/Child:ID_1/Grandchild:ID_2"
///
/// @todo       rename as  descriptorOf() overload
///
inline std::string treeOf(const XMLElement* element, bool include_self = true, const std::string& id_attribute = "name") {
    // check if input element is valid
    if (!element) {
        return "";
    }

    // move element node upwards one level
    const XMLElement* parent = element->Parent()->ToElement();

    // check if element should be included, otherwise skip to parent node
    if (!include_self) {
        return treeOf(parent, true, id_attribute);
    }

    // parse element name
    std::string name = element->Name();

    // parse id attribute (if present)
    if (!id_attribute.empty()) {
        auto id_attribute_value = element->Attribute(id_attribute.data());
        if (id_attribute_value) {
            name += ":" + std::string(id_attribute_value);
        }
    }

    // construct string recursively until root node is reached
    std::string tree = treeOf(parent, true, id_attribute) + "/" + name;

    return tree;
}



//------------------------------------------------------------------------------
/// @brief      Checks if given XML element fits a formatted list of attributes.
///
/// @param[in]  element    XML element
/// @param[in]  attribute  Attribute list, formatted as {"name0:value0", "name1:value1", ... }
///
/// @return     True if element has attributes with values given, False otherwise.
///
inline bool hasAttributes(const XMLElement* element, const std::vector< std::string >& attributes) {
    // check if input element is valid
    if (!element) {
        return false;
    }

    // parse attribute list
    for (const auto& attribute : attributes) {
        auto atts = tokenize(attribute, ':');
        if (atts.size() < 2) {
            return false;
        }
        if (!element->Attribute(atts[0].data(), atts[1].data())) {
            return false;
        }
    }

    // true by default (e.g. empty attribute list)
    return true;
}



//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attriutes in the form of a brace-enclosed list.
///
inline bool hasAttributes(const XMLElement* element, const std::initializer_list< std::string >& attributes) {
    return hasAttributes(element, std::vector< std::string >(attributes));
}



//------------------------------------------------------------------------------
/// @brief      Sets/adds attributes to given element.
///
/// @param[in]  element    XML Element
/// @param[in]  attribute  Attribute list, formatted as {"name0:value0", "name1:value1", ... }
///
/// @return     Number of attributes set/assigned.
///
inline int setAttributes(XMLElement* element, const std::vector< std::string >& attributes) {
    // check if input element is valid
    if (!element) {
        return 0;
    }

    // parse attribute list
    int count = 0;
    for (const auto& attribute : attributes) {
        auto atts = tokenize(attribute, ':');
        if (atts.size() < 2) {
            continue;
        }
        element->SetAttribute(atts[0].data(), atts[1].data());
        count++;
    }

    return count;
}



//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attriutes in the form of a brace-enclosed list.
///
inline int setAttributes(XMLElement* element, const std::initializer_list< std::string >& attributes) {
    return setAttributes(element, std::vector< std::string >(attributes));
}



//------------------------------------------------------------------------------
/// @brief      Sets/adds attributes to given element.
///
/// @param[in]  element    XML Element
/// @param[in]  attribute  Attribute list, as a map with text keys (name) and values.
///
/// @return     Number of attributes set/assigned.
///
inline int setAttributes(XMLElement* element, const std::map< std::string, std::string >& attributes) {
    // check if input element is valid
    if (!element) {
        return 0;
    }

    // parse attribute list
    int count = 0;
    for (const auto& attribute : attributes) {
        element->SetAttribute(attribute.first.data(), attribute.second.data());
        count++;
    }

    return count;
}



//------------------------------------------------------------------------------
/// @brief      Gets the child XML element from a formatted descriptor string.
///             Useful to get elements directly from multi-level descriptors.
///
/// @param      root          Root node (XMLNode is used in order to allow using a XMLDocument as well as a XMLElement).
/// @param[in]  descriptor    Descriptor string, with levels (parent/childs) delimited by '/' and id attribute values specified with ':' e.g. Root/Node/Child:Value/Element:Name.
/// @param[in]  id_attribute  Identifier attribute name, defaults to "name".
///
/// @return     XMLElement pointer, null if not present (in relation to root node).
///
/// @todo       Add additional attribute check for last level child node (?)
///
/// @todo       Define class to parse (tokenize) descriptor and id_attribute arguments?
///
inline XMLElement* elementAt(XMLNode* root, const std::string& descriptor, const std::string& id_attribute = "name") {
    // argument check to avoid segmentation fault when accessing child node
    if (!root) {
        return 0;
    }

    // if empty descriptor, return root node as element (will fail if root is not an element e.g. XMLDocument)
    if (descriptor.empty()) {
        return root->ToElement();
    }

    // get first child element (level) from descriptor
    auto levels = tokenize(descriptor, '/', true);

    // parse element name and attribute, if given
    auto ws = tokenize(levels[0], ':');

    // move to first element of matching name
    XMLElement* child = root->FirstChildElement(ws[0].data());

    // move to sibling with matching id attribute, if given
    if (ws.size() >= 2) {
        while (child) {
            // if (hasAttributes(child, { id_attribute, ws[1]})) {
            //     break;
            // }
            if (child->Attribute(id_attribute.data(), ws[1].data())) {
                break;
            }
            child = child->NextSiblingElement(ws[0].data());
        }
    }

    // if at last level, return child
    if (levels.size() == 1) {
        return child;
    }

    // recursive call to child element with split descriptor if not at last level
    return elementAt(child, levels.back(), id_attribute);
}



//------------------------------------------------------------------------------
/// @brief      Gets the child XML element from a formatted descriptor string.
///             Useful to get elements directly from multi-level descriptors.
///
/// @param      root          Root node (XMLNode is used in order to allow using a XMLDocument as well as a XMLElement).
/// @param[in]  descriptor    Descriptor string, with levels (parent/childs) delimited by '/' and id attribute values specified with ':' e.g. Root/Node/Child:Value/Element:Name.
/// @param[in]  id_attribute  Identifier attribute name, defaults to "name".
///
/// @return     const XMLElement pointer, null if not present (in relation to root node).
///
/// @note       Const overload, for acessing elements on const nodes.
///
/// @todo       Add additional attribute check for last level child node (?)
///
/// @todo       Define class to parse (tokenize) descriptor and id_attribute arguments?
///
inline const XMLElement* elementAt(const XMLNode* root, const std::string& descriptor, const std::string& id_attribute = "name") {
    // @note maybe not the most appropriate way to implement this, but better than repeating whole implmenetation with a slight change
    // @note returned pointer is imoplicelty cast to const by return type of this overload
    return elementAt(const_cast< XMLNode* >(root), descriptor, id_attribute);

    // // argument check to avoid segmentation fault when accessing child node
    // if (!root) {
    //     return 0;
    // }

    // // if empty descriptor, return root node as element (will fail if root is not an element e.g. XMLDocument)
    // if (descriptor.empty()) {
    //     return root->ToElement();
    // }

    // // get first child element (level) from descriptor
    // auto levels = tokenize(descriptor, '/', true);

    // // parse element name and attribute, if given
    // auto ws = tokenize(levels[0], ':');

    // // move to first element of matching name
    // const XMLElement* child = root->FirstChildElement(ws[0].data());

    // // move to sibling with matching id attribute, if given
    // if (ws.size() >= 2) {
    //     while (child) {
    //         // if (hasAttributes(child, { id_attribute, ws[1]})) {
    //         //     break;
    //         // }
    //         if (child->Attribute(id_attribute.data(), ws[1].data())) {
    //             break;
    //         }
    //         child = child->NextSiblingElement(ws[0].data());
    //     }
    // }

    // // if at last level, return child
    // if (levels.size() == 1) {
    //     return child;
    // }

    // // recursive call to child element with split descriptor if not at last level
    // return elementAt(child, levels.back(), id_attribute);
}




//------------------------------------------------------------------------------
/// @brief      Gets multiple XMLElements with matching attributes.
///
/// @param[in]  root            Root node (XMLNode is used in order to allow using a XMLDocument as well as a XMLElement).
/// @param[in]  attribute       Attribute list, formatted as {"name0:value0", "name1:value1", ... }
/// @param[in]  element_name    Name of the child element to parse (defaults to empty string, wherein all elements are considered)
/// @param[in]  deep            Deep/recursive call flag. If true, list will contain all child nodes with matching attributes. *may add significant overhead!*
/// @param[in]  single          Single element search flag. Defaults to false, when true only the first matching element is returned.
///
/// @return     Vector of pointers to valid/matching elements.
///
/// @note       Useful to hold the elements on an STL container as it allows range-based loop, among other functionalities.
///
/// @note       Deep/recursive call can be used to search for children elements under a node (not very efficient as it builds a vector in the proccess)
///
inline std::vector< XMLElement* > elementsUnder(XMLNode* root, bool deep, const std::vector< std::string >& attributes, const std::string& element_name = "", bool single = false) {
    // initiaze output vector
    std::vector< XMLElement* > valid_elements;
    // std::vector< decltype(root->ToElement) > valid_elements;

    // argument check to avoid segmentation fault when accessing child node
    // returns empty container
    if (!root) {
        return valid_elements;
    }

    // loop over child elements and append matching ones
    XMLElement* child;
    if (element_name.empty()) {
        child = root->FirstChildElement();
    } else {
        child = root->FirstChildElement(element_name.data());
    }

    while (child) {
        // append copy to output vector
        if (hasAttributes(child, attributes)) {
            valid_elements.push_back(child);
            // if single element search, return on first find
            if (single) {
                return valid_elements;
            }
        }

        // if single element search, return on first find
        if (single && valid_elements.size() > 0) {
            return valid_elements;
        }

        // add children elements (recursive)
        if (deep) {
            // *move* descendent matching elements into vector
            std::move_into(valid_elements, elementsUnder(child, true, attributes, element_name));
        }

        // move to next sibling
        if (element_name.empty()) {
            child = child->NextSiblingElement();
        } else {
            child = child->NextSiblingElement(element_name.data());
        }
    }

    return valid_elements;
}



//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attributes in the form of a brace-enclosed list.
///
inline std::vector< XMLElement* > elementsUnder(XMLNode* root, bool deep = false, const std::initializer_list< std::string >& attributes = { /* ... */ }, const std::string& element_name = "", bool single = false) {
    return elementsUnder(root, deep, std::vector< std::string >(attributes), element_name, single);
}



//------------------------------------------------------------------------------
/// @brief      Gets multiple XMLElements with matching attributes (const overload).
///
/// @param[in]  root            Root node (XMLNode is used in order to allow using a XMLDocument as well as a XMLElement).
/// @param[in]  attribute       Attribute list, formatted as {"name0:value0", "name1:value1", ... }
/// @param[in]  element_name    Name of the child element to parse (defaults to empty string, wherein all elements are considered)
/// @param[in]  deep            Deep/recursive call flag. If true, list will contain all child nodes with matching attributes. *may add significant overhead!*
/// @param[in]  single          Single element search flag. Defaults to false, when true only the first matching element is returned.
///
/// @return     Vector of pointers to valid/matching elements.
///
/// @note       Useful to hold the elements on an STL container as it allows range-based loop, among other functionalities.
///
/// @note       Deep/recursive call can be used to search for children elements under a node (not very efficient as it builds a vector in the proccess)
///
inline std::vector< const XMLElement* > elementsUnder(const XMLNode* root, bool deep, const std::vector< std::string >& attributes, const std::string& element_name = "", bool single = false) {
    // initiaze output vector
    std::vector< const XMLElement* > valid_elements;

    // auto children = elementsUnder(const_cast< XMLNode* >(root), deep, attributes, element_name, single);

    // std::vector< const XMLElement* > c_children;
    // for (const auto& child : children) {
    //     c_children.emplace_back(child);
    // }
    //
    // return c_children;


    // argument check to avoid segmentation fault when accessing child node
    // returns empty container
    if (!root) {
        return valid_elements;
    }

    // loop over child elements and append matching ones
    const XMLElement* child;
    if (element_name.empty()) {
        child = root->FirstChildElement();
    } else {
        child = root->FirstChildElement(element_name.data());
    }

    while (child) {
        // append copy to output vector
        if (hasAttributes(child, attributes)) {
            valid_elements.push_back(child);
            // if single element search, return on first find
            if (single) {
                return valid_elements;
            }
        }

        // if single element search, return on first find
        if (single && valid_elements.size() > 0) {
            return valid_elements;
        }

        // add children elements (recursive)
        if (deep) {
            // *move* descendent matching elements into vector
            std::move_into(valid_elements, elementsUnder(child, true, attributes, element_name));
        }

        // move to next sibling
        if (element_name.empty()) {
            child = child->NextSiblingElement();
        } else {
            child = child->NextSiblingElement(element_name.data());
        }
    }

    return valid_elements;
}




//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attributes in the form of a brace-enclosed list (const overload)
///
inline std::vector< const XMLElement* > elementsUnder(const XMLNode* root, bool deep = false, const std::initializer_list< std::string >& attributes = { /* ... */ }, const std::string& element_name = "", bool single = false) {
    return elementsUnder(root, deep, std::vector< std::string >(attributes), element_name, single);
}



//------------------------------------------------------------------------------
/// @brief      Search for element under node, matching name and/or attribute.
///
/// @param[in]  root            Root node (XMLNode is used in order to allow using a XMLDocument as well as a XMLElement).
/// @param[in]  element_name    Name of the child element to parse (empty string -> ignores name)
///
/// @return     XMLElement pointer to valid/matching elements.
///
/// @note       Wraps around elementsUnder() for a more intuitive signature when searching for children elements
///
inline XMLElement* elementUnder(XMLNode* root, const std::string& element_name, const std::vector< std::string >& attributes = { /* ... */ }) {
    auto elements = elementsUnder(root, true, attributes, element_name, true);
    if (elements.size() > 0) {
        return elements[0];
    }
    return 0;
}



//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attriutes in the form of a brace-enclosed list.
///
inline XMLElement* elementUnder(XMLElement* element, const std::string& element_name, const std::initializer_list< std::string >& attributes = { /* ... */ }) {
    return elementUnder(element, element_name, std::vector< std::string >(attributes));
}



//------------------------------------------------------------------------------
/// @brief      Constructs an attribute list for a particular XML element.
///
/// @param[in]  element         XML element. Specific to element nodes (only elements have attributes).
/// @param[in]  attribute_name  Attribute name to filter output. Defaults to empty, wherein all attributes are added to output.
/// @param[in]  deep            Recrusive flag. Defaults to false.
/// @param[in]  element_name    Element name to filter output. Defaults to empty, wherein all elements are visited.
///
/// @return     Vector of XMLAttribute const pointers.
///
/// @note       Useful to hold the attributes on an STL container as it allows range-based loops, among other functionalities.
///
/// @note       Useful when performing bulk operations (e.g. reading values) on a large number of attributes.
///
/// @note       Single element recursive overload.
///
inline std::vector< const XMLAttribute* > attributesOf(const XMLElement* element, const std::string& attribute_name = "", bool deep = false, const std::string& element_name = "") {
    // initialize empty output container
    std::vector< const XMLAttribute* > attributes;

    // check if input element is valid
    if (!element) {
        return attributes;
    }

    // recursive call to get children attributes
    if (deep) {
        // @note elementsUnder(...) filters on relevant attributes & element name
        for (const auto& child : elementsUnder(element, false, { attribute_name }, element_name)) {
            std::move_into(attributes, attributesOf(child, attribute_name, true, element_name));
        }
    }

    // loop over attributes & append to container
    const XMLAttribute* attribute = element->FirstAttribute();
    while (attribute) {
        // if attribute name was specified and name doesn't match, skip
        if (!attribute_name.empty() && attribute_name.compare(attribute->Name()) != 0) {
            attribute = attribute->Next();
            continue;
        }
        attributes.emplace_back(attribute);
        attribute = attribute->Next();
    }

    return attributes;
}



//------------------------------------------------------------------------------
/// @brief      Constructs an attribute list for multiple XML elements,
///             either a single attribute per element or all attributes for each element.
///
/// @param[in]  elements        XML Elements (as a vector of pointers)
/// @param[in]  attribute_name  Attribute name to filter output. Defaults to empty, wherein all attributes are added to output.
/// @param[in]  element_name    Element name to filter output. Defaults to empty, wherein all elements are visited.
///
/// @return     Vector of XMLAttribute const pointers.
///
///
/// @note       Multiple element non-recrusive overload.
///
inline std::vector< const XMLAttribute* > attributesOf(const std::vector< XMLElement* >& elements, const std::string& attribute_name = "", const std::string& element_name = "") {
    // initialize empty output container
    std::vector< const XMLAttribute* > attributes;

    // loop over input element list
    for (const auto& element : elements) {
        // if invalid, skip
        if (!element) {
            continue;
        }
        // if element_name was given and doesn't match, skip
        if (!element_name.empty() && element_name.compare(element->Name()) != 0) {
            continue;
        }
        // call single element overload and append/move into vector
        std::move_into(attributes, attributesOf(element, attribute_name, false, element_name));
    }

    return attributes;
}



//------------------------------------------------------------------------------
/// @brief      Parses attribute values of multiple XML elements.
///
/// @param[in]  elements      XML elements (as a vector of const pointers)
/// @param[in]  id_attribute  ID dttribute name. Defaults to "name".
///
/// @return     Vector of text string with values of the id attributes for each valid element in the input list.
///
/// @note       Allows recursive operations (optional), when used in conjuction with elementsUnder(...).
///
/// @note       Not the most efficient way to implement this, but it may be a reasonable compromise as it greatly increases simplicity, readibility and versatility of the code.
///
inline std::vector< std::string > valuesOf(const std::vector< XMLElement* >& elements, const std::string& id_attribute = "name") {
    // initialize output container
    std::vector< std::string > element_ids;

    // loop over input element list
    for (const auto& element : elements) {
        // if element isn't valid, skip!
        if (!element) {
            continue;
        }
        // get attribute value, if valid append to list
        auto id = element->Attribute(id_attribute.data());
        if (id) {
            element_ids.emplace_back(id);
        }
    }

    return element_ids;
}



//------------------------------------------------------------------------------
/// @brief      Parses the values of multiple XML attributes.
///
/// @param[in]  attributes      XML attributes (as a vector of const pointers)
/// @param[in]  attribute_name  Attribute name. Defaults to empty, wherein all attributes are added to output.
///
/// @return     Vector of text string with values of the attributes which fit input criteria.
///
/// @note       Allows recursive operations (optional), when used in conjuction with elementsUnder(...).
///
/// @note       Not the most efficient way to implement this, but it may be a reasonable compromise as it greatly increases simplicity, readibility and versatility of the code.
///
/// @todo       Create a similar function template that casts attribute values to a (literal) data type (?).
///
/// @todo       Profile against argumentValuesOf, delete less efficient function.
///
/// @todo       Clean unnecessary arguments accross tinyxml2_extra.hpp!!!
///
inline std::vector< std::string > valuesOf(const std::vector< const XMLAttribute* >& attributes, const std::string& attribute_name = "") {
    // initialize output container
    std::vector< std::string > attribute_values;

    // loop over input attribute list
    for (const auto& attribute : attributes) {
        // if attrbute isn't valid, skip!
        if (!attribute) {
            continue;
        }
        // if attribute name was specified and name doesn't match, skip
        if (!attribute_name.empty() && attribute_name.compare(attribute->Name()) != 0) {
            continue;
        }
        attribute_values.emplace_back(attribute->Value());
    }

    return attribute_values;
}



//------------------------------------------------------------------------------
/// @brief      From multiple XML elements, create a list with the values for a specific/multiple attribute/s.
///
/// @param[in]  elements        XML Elements (as a vector of pointers).
/// @param[in]  id_attribute    Identifier attribute name, defaults to "name".
/// @param[in]  element_name    Name of the child element to parse (defaults to empty string, wherein all elements are considered).
///
/// @return     Vector of text strings (attribute values as text) for the elements which fit input criteria.
///
/// @note       Allows recursive operations (optional), when used in conjuction with elementsUnder(...).
///
/// @note       In practice, equivalent to valuesOf(attributesOf(...)), but provides (in theory) a more efficient implementation as it
///             doesn't require the allocation of a temporary std::vector< const XMLAttribute* > (which can become costly for large XML files/node trees)
///             and values are parsed directly from the vector of XMLElement(s).
///
/// @todo       Remove element_name argument, if one needs to filter then should edit the input element vector directly. Specifying element-name outside elementsUnder only makes sense in recursive functions!
///
inline std::vector< std::string > attributeValuesOf(const std::vector< const XMLElement* >& elements, const std::string& attribute_name = "", const std::string& element_name = "") {
    // initialize output container
    std::vector< std::string > attribute_values;

    // loop over input element list
    for (const auto& element : elements) {
        // if invalid, skip
        if (!element) {
            continue;
        }
        // if element_name was given and doesn't match, skip
        if (!element_name.empty() && element_name.compare(element->Name()) != 0) {
            continue;
        }
        // @note attributesOf filters according to attribute name
        auto element_attributes = attributesOf(element, attribute_name);
        for (const auto& attribute : element_attributes) {
            attribute_values.emplace_back(attribute->Value());
        }

    }

    return attribute_values;
}

// inline std::vector< std::string > attributeValuesOf(const std::vector< const XMLElement* >& elements, const std::string& attribute_name = "", const std::string& element_name = "") {
//     // initialize output container
//     std::vector< std::string > attribute_values;

//     // loop over input element list
//     for (const auto& element : elements) {
//         // if invalid, skip
//         if (!element) {
//             continue;
//         }
//         // if element_name was given and doesn't match, skip
//         if (!element_name.empty() && element_name.compare(element->Name()) != 0) {
//             continue;
//         }
//         // @note attributesOf filters according to attribute name
//         auto element_attributes = attributesOf(element, attribute_name);
//         for (const auto& attribute : element_attributes) {
//             attribute_values.emplace_back(attribute->Value());
//         }

//     }

//     return attribute_values;
// }

//------------------------------------------------------------------------------
/// @brief      Parses the attributes of a XML element, creating a list of formatted descriptor strings.
///
/// @param[in]  element    XML element. Specific to element nodes (only elements have attributes), thus XMLElement is used instead of XMLNode.
/// @param[in]  separator  Separator/delimiter character. Defaults to ':'.
///
/// @return     List of attributes formatted as "name[separator]value" (cf. ::descriptorOf()).
///
/// @note       Useful to pass directly to functions that require a formatted attribute list.
///
inline std::vector< std::string > attributeDescriptorOf(const XMLElement* element, char separator = ':') {
    // initialize output container
    std::vector< std::string > list;

    // loop over attributes and add to map
    if (element) {
        const XMLAttribute* attribute = element->FirstAttribute();
        while (attribute) {
            list.emplace_back(descriptorOf(attribute, separator));
            attribute = attribute->Next();
        }
    }

    return list;
}



//------------------------------------------------------------------------------
/// @brief      Parses the attributes of a XML element, creating a map of text keys to text values.
///
/// @param[in]  element  XML element. Specific to element nodes (only elements have attributes), thus XMLElement is used instead of XMLNode.
///
/// @return     Map of attribute names to atribute values. Refer to std::map<> documentation.
///
/// @note       For use when a high-level structure such a std::map is required/useful while parsing a single
///             XML element with a great number of arguments.
///
inline std::map< std::string, std::string > attributeMapOf(const XMLElement* element) {
    // initialize output container
    std::map< std::string, std::string > list;

    // loop over attributes and add to map
    if (element) {
        const XMLAttribute* attribute = element->FirstAttribute();
        while (attribute) {
            list.insert(std::make_pair(attribute->Name(), attribute->Value()));
            attribute = attribute->Next();
        }
    }

    return list;
}



//------------------------------------------------------------------------------
/// @brief      Adds a dependent element below given node. Useful to provide a common interface to add elements to any node (documents and elements).
///
/// @param[in]  root  XML root/parent node
/// @param[in]  name  Name of the child element to add
///
/// @return     XMLElement pointer to newly created element.
///
inline XMLElement* addElementUnder(XMLNode* root, const std::string& name) {
    // instantiate empty/null pointer
    XMLElement* element;

    // create new child node
    if (root->ToElement()) {
        element =  root->ToElement()->InsertNewChildElement(name.data());
    } else if (root->ToDocument()) {
        element =  root->ToDocument()->NewElement(name.data());
    } else {
        return 0;  // can't add element for some reason!
    }

    // insert into root node (required!)
    return root->InsertEndChild(element)->ToElement();
}



//------------------------------------------------------------------------------
/// @brief      Adds an element under a given node.
///
/// @param[in]  root            XML root/parent node
/// @param[in]  descriptor      Descriptor string, with levels (parent/childs) delimited by '/' and id attribute values specified with ':' e.g. Root/Node/Child:Value/Element:Name.
/// @param[in]  attributes      Attribute list, formatted as {"name0:value0", "name1:value1", ... }
///
/// @return     XMLElement pointer to newly created element.
///
inline XMLElement* addElementTo(XMLNode* root, const std::string& descriptor, const std::vector< std::string >& attributes, const std::string& id_attribute = "name") {
    // argument check to avoid segmentation fault when accessing child nodes
    if (!root || descriptor.empty()) {
        return 0;
    }

    // parse element name
    auto levels = tokenize(descriptor, '/', true /* single split */);

    // check if upper level/child exists
    XMLElement* child = elementAt(root, levels[0], id_attribute);

    // if not, create/add top element
    if (!child || levels.size() == 1) {
        // parse name
        auto ws = tokenize(levels[0], ':', true /* single split */);
        // insert node
        child = addElementUnder(root, ws[0]);
        // set id attribute
        if (child && ws.size() > 1) {
            child->SetAttribute(id_attribute.data(), ws[1].data());
        }
    }

    // if at last level, return child
    if (levels.size() == 1) {
        if (!hasAttributes(child, attributes)) {  // @note may not be necesary to verify - result should be the same
            // set attributes
            setAttributes(child, attributes);
        }
        return child;
    }

    // recursive call
    return addElementTo(child, levels.back(), attributes, id_attribute);
}



//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attriutes in the form of a brace-enclosed list.
///
inline XMLElement* addElementTo(XMLNode* root, const std::string& descriptor, const std::initializer_list< std::string >& attributes = { /* ... */ }, const std::string& id_attribute = "name") {
    return addElementTo(root, descriptor, std::vector< std::string >(attributes), id_attribute);
}


//------------------------------------------------------------------------------
/// @brief      Adds an element using another element family tree/node hierarchy as example.
///
/// @param      element       XML element to use
/// @param      root          The root
/// @param[in]  attributes    The attributes
/// @param[in]  id_attribute  The identifier attribute
///
/// @return     XMLElement pointer to newly created element.
///
/// @note       Wraps around addElement and treeOf.
///
/// @note       Similar to a shallow clone of a node, but clearing all midlevel attributes with the exception of id one (if present)
///
inline XMLElement* addElementLike(XMLElement* element, XMLNode* root, const std::vector< std::string >& attributes = { /* ... */ }, const std::string& id_attribute = "name") {
    return addElementTo(root, treeOf(element, true, id_attribute), attributes);
}



//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attriutes in the form of a brace-enclosed list.
///
inline XMLElement* addElementLike(XMLElement* element, XMLNode* root, const std::initializer_list< std::string >& attributes = { /* ... */ }, const std::string& id_attribute = "name") {
    return addElementLike(element, root, std::vector< std::string >(attributes), id_attribute);
}



//------------------------------------------------------------------------------
/// @brief      Copies a given XML element into a different node (same/different document).
///
/// @param      target     Target XML (parent) node.
/// @param      element    XML element to clone.
/// @param[in]  whole_tree  Whether or not to maintain same family tree/node hierarchy of source element (relative to target node). Defaults to false.
///
/// @return     XMLElement pointer to newly created (cloned) element.
///
inline XMLElement* cloneInto(XMLNode* target, const XMLElement* element, bool whole_tree = false, const std::string& descriptor = "", const std::string& id_attribute = "name") {
    // check input arguments, strictly required!
    if (!element || !target) {
        return 0;
    }

    // initialize target node at root of descriptor (e.g. document root before parsing descriptor)
    XMLNode* dst = target;

    // parse optional descriptor string
    // adds elements according to hiearchy and ids, input element is cloned into bottommost element
    if (!descriptor.empty() && !elementAt(dst, descriptor)) {
        dst = addElementTo(dst, descriptor, { /* */ }, id_attribute);
        // std::cout << treeOf(dst->ToElement()) << std::endl;
    }

    // copy parents/ascendants unde target node
    // shifts dst towards end of tree
    if (whole_tree) {
        // check if target element exists, if it does skip and perform deep copy
        if (!elementAt(dst, treeOf(element, false /* only parent levels */, id_attribute))) {
            // tokenize source tree / parse element name
            auto levels = std::tokenize(treeOf(element, false /* only parent levels */, id_attribute), '/');
            // get source element (to copy)
            const XMLElement* src = elementAt(element->GetDocument(), levels[0]);
            // loop over parents, perform shallow copies of each one
            // afaik, impossible to implement this recursively
            for (int idx = 1; idx < levels.size(); idx++) {
                std::cout << "copying " << levels[idx - 1] << std::endl;
                // shallow copy
                XMLNode* parent_copy = src->ShallowClone(target->GetDocument());
                // insert into target
                dst = dst->InsertEndChild(parent_copy);
                // update source element
                src = elementAt(src, levels[idx], id_attribute);
            }
        }
    }

    // when called on end child, target parent will already exist!
    XMLNode* copy = element->DeepClone(target->GetDocument());
    dst = dst->InsertEndChild(copy);

    return dst->ToElement();
}



// //------------------------------------------------------------------------------
// /// @brief      Clones/copies a XML node into a different XML document, while keeping the same node hierarchy
// ///
// /// @param      source   Source document
// /// @param      target   Target document
// /// @param[in]  element  Name of the XML element to clone
// ///
// /// @return     Node of copied element on target document
// ///
// /// @throws     std::invalid_argument if element is not present on source or error occurs during copy.
// ///
// inline XMLNode* clone(XMLNode* source, XMLDocument* target, const std::string& element) {
//     auto elements = tokenize(element, '/');
//     XMLNode* target_node = target;
//     XMLNode* source_node = source->FirstChildElement(elements[0].data());
//     // shallow copy of parent nodes
//     for (int idx = 0; idx < elements.size() - 1; idx++) {
//         // fetch node on source document
//         // this is needed instead of getHandle as we need to perform shallow copies of every parent node
//         if (!source_node) {
//             // std::cout << "'"<< elements[idx] << "': missing on source document, ABORTING! " << std::endl;
//             throw std::invalid_argument("Invalid element name!");
//         }
//         // shallow copy of level
//         auto copy = source_node->ShallowClone(target);
//         if (!copy) {
//             throw std::invalid_argument("Unable to clone source element!");
//         }
//         // check if present on target document
//         if (!target_node->LastChildElement(elements[idx].data())) {
//             // std::cout << "'"<< elements[idx] << "': missing on target document, adding element! " << std::endl;
//             target_node = target_node->InsertEndChild(copy);
//         } else {
//             // std::cout << elements[idx] << std::endl;
//             target_node = target_node->LastChildElement(elements[idx].data());
//         }
//         // assign new node ptrs
//         source_node = source_node->FirstChildElement(elements[idx + 1].data());
//         // target_node = target_node->LastChildElement(elements[idx + 1].data());
//     }
//     // deep copy of last/target child
//     auto copy = source_node->DeepClone(target);
//     target_node->InsertEndChild(copy);
//     return target_node;
// }



//------------------------------------------------------------------------------
/// @brief      Removes a given XML element from document/parent.
///
/// @param      element  XML element
///
/// @return     True if element was removed, False otherwise.
///
/// @note       Single element overload.
///
inline bool remove(XMLElement* element) {
    // check if input element is valid
    if (!element) {
        return false;
    }

    // delete all children (not necessary, may be overkill)
    element->DeleteChildren();
    // delete node from parent or directly from document if at top level
    if (element->Parent()) {
        element->Parent()->DeleteChild(element);
    } else {
        element->GetDocument()->DeleteNode(element);
    }

    return true;
}



//------------------------------------------------------------------------------
/// @brief      Removes multiple elements.
///
/// @param[in]  elements        XML Elements (as a vector of pointers)
/// @param[in]  element_name    Element name filter. Defaults to empty string, wherein all elements are considered.
///
/// @return     Number of elements removed.
///
/// @note       Multiple element overload.
///
inline size_t remove(const std::vector< XMLElement* > elements, const std::string& element_name = "") {
    size_t count = 0;

    for (const auto& element : elements) {
        // check if element is valid
        if (!element) {
            continue;
        }
        // if element_name was given and doesn't match, skip
        if (!element_name.empty() && element_name.compare(element->Name()) != 0) {
            continue;
        }
        // remove element & increment counter
        if (remove(element)) {
            count++;
        }
    }

    return count;
}



//------------------------------------------------------------------------------
/// @brief      Removes multiple atributes from a given XML element.
///
/// @param      element       XML element
/// @param[in]  attribute     Attribute list, formatted as {"name0:value0", "name1:value1", ... }
/// @param[in]  deep          Recursive flag, removes matching attributes in children as well.
/// @param[in]  element_name  Element name filter. Defaults to empty string, wherein all elements are considered.
///
/// @return     Number of attributes removed
///
/// @note       Single element recursive overload.
///
inline size_t removeAttributesOf(XMLElement* element, const std::vector< std::string >& attributes, bool deep = false, const std::string& element_name = "") {
    size_t count = 0;

    // remove in children (recursive)
    if (deep) {
        // @note elementsUnder(...) filters on relevant attributes & element name
        for (const auto& child : elementsUnder(element, false, attributes, element_name)) {
            count += removeAttributesOf(child, attributes, deep);
        }
    }

    for (const auto& attribute : attributes) {
        // parse attribue descritor string
        auto atts = tokenize(attribute, ':');
        if (atts.size() == 0) {
            continue;
        }
        // if value is given but no argument matches
        if (atts.size() > 1) {
            if (element->Attribute(atts[0].data(), atts[1].data())) {
                continue;
            }
        }
        // dele attribute by name
        element->DeleteAttribute(atts[0].data());
        count++;
    }

    return count;
}




//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attriutes in the form of a brace-enclosed list.
///
inline size_t removeAttributesOf(XMLElement* element, const std::initializer_list< std::string >& attributes = { /* ... */ }, bool deep = false, const std::string& element_name = "") {
    return removeAttributesOf(element, std::vector< std::string >(attributes), deep, element_name);
}




//------------------------------------------------------------------------------
/// @brief      Removes multiple atributes from a given XML element list.
///
/// @param[in]  elements        XML Elements (as a vector of pointers).
/// @param[in]  attribute       Attribute list, formatted as {"name0:value0", "name1:value1", ... }
/// @param[in]  element_name    Element name to filter output. Defaults to empty, wherein all elements are visited.
///
/// @return     Number of attributes removed
///
/// @note       Multiple element non-recursive overload.
///
inline size_t removeAttributesOf(const std::vector< XMLElement* > elements, const std::vector< std::string >& attributes, const std::string& element_name = "") {
    size_t count = 0;

    // loop over input element list
    for (const auto& element : elements) {
        // if invalid, skip
        if (!element) {
            continue;
        }
        // if element_name was given and doesn't match, skip
        if (!element_name.empty() && element_name.compare(element->Name()) != 0) {
            continue;
        }
        // if empty, append all atributes to vector
        count += removeAttributesOf(element, attributes, false, element_name);
    }

    return count;
}



//------------------------------------------------------------------------------
/// @brief      Convenience over, accepting attriutes in the form of a brace-enclosed list.
///
inline size_t removeAttributesOf(const std::vector< XMLElement* > elements, const std::initializer_list< std::string >& attributes = { /* ... */ }, const std::string& element_name = "") {
    return removeAttributesOf(elements, std::vector< std::string >(attributes), element_name);
}



//------------------------------------------------------------------------------
/// @brief      Replace text string in the content of every XML element.
///
/// @param      element         XML Element.
/// @param[in]  old_value       Text to replace.
/// @param[in]  new_value       New attribute value (replacement)
/// @param[in]  deep            Recursive flag, when True changes are applied to all child nodes. Defaults to True.
/// @param[in]  element_name    Optional element name for filtering. Defaults to empty string, wherein all element are considered.
///
/// @return     Number of replacements performed.
///
/// @note       In theory, can be used to check/count how many ocurrences of a string there are under a particular node (e.g. if new_value = old_value),
///             although it is not a very efficient way to do so.
///
/// @note       In practice, equivalent to replaceIn(elementsUnder(...)), but provides (in theory) a more efficient implementation as it
///             doesn't require the allocation of a temporary std::vector< const XMLElement* > and values are parsed directly from the vector of XMLElement (s),
///             which can become costly for large XML files/node trees.
///
/// @note       Single element recursive overload.
///
inline size_t replaceIn(XMLElement* element, const std::string& old_value, const std::string& new_value, bool deep = false, const std::string& element_name = "") {
    // change counter
    size_t count = 0;

    // replace in children (recursive)
    if (deep) {
        // @note elementsUnder(...) filters on relevant element name
        for (const auto& child : elementsUnder(element, false, { /* */ }, element_name)) {
            count += replaceIn(child, old_value, new_value, deep, element_name);
        }
    }

    // loop over attributes and parse text content
    if (element) {
        if ((!element_name.empty() && element_name.compare(element->Name()) != 0) || !element->GetText()) {
            return count;
        }
        auto text = std::string(element->GetText());
        count += std::replace(&text, old_value, new_value);
        element->SetText(text.data());
    }

    return count;
}



//------------------------------------------------------------------------------
/// @brief      Replace text string in the content of every XML element.
///
/// @param      element         XML Element.
/// @param[in]  old_value       Text to replace.
/// @param[in]  new_value       New attribute value (replacement)
/// @param[in]  element_name    Optional element name for filtering. Defaults to empty string, wherein all element are considered.
///
/// @return     Number of replacements performed.
///
/// @note       In theory, can be used to check/count how many ocurrences of a string there are under a particular node (e.g. if new_value = old_value),
///             although it is not a very efficient way to do so.
///
/// @note       Multiple element non-recursive overload.
///
inline size_t replaceIn(const std::vector< XMLElement* >& elements, const std::string& old_value, const std::string& new_value, const std::string& element_name = "") {
    size_t count = 0;

    // loop over attributes
    for (auto& element : elements) {
        if (!element) {
            continue;
        }
        // if element_name was given and doesn't match, skip
        if (!element_name.empty() && element_name.compare(element->Name()) != 0) {
            continue;
        }
        // call single element overload with replacement implementation
        count += replaceIn(element, old_value, new_value, false, element_name);
    }

    return count;
}



//------------------------------------------------------------------------------
/// @brief      Replaces text string in attribute values (text) with given value, for attribute of given name (optional).
///
/// @param      element         XML Element.
/// @param[in]  old_value       Attribute text to replace.
/// @param[in]  new_value       New attribute value (replacement).
/// @param[in]  attribute_name  Optional attribute name for filtering. Defaults to empty string, where all attributes are considered.
/// @param[in]  deep            Recursive flag, when True changes are applied to all child nodes.
/// @param[in]  element_name    Optional element name for filtering. Defaults to empty string, wherein all element are considered.
///
/// @return     Number of changes.
///
/// @note       At this stage can't be wrapped around attributesOf(...) because of const return type!
///
/// @note       Single element recursive overload.
///
inline size_t replaceInAttributesOf(XMLElement* element, const std::string& old_value, const std::string& new_value, const std::string& attribute_name = "", bool deep = false, const std::string& element_name = "") {
    if (!element) {
        return 0;
    }

    size_t count = 0;

    // replace in children (recursive)
    if (deep) {
        // @note elementsUnder(...) filters on relevant attribute & element name
        for (const auto& child : elementsUnder(element, false, { attribute_name }, element_name)) {
            count += replaceInAttributesOf(child, old_value, new_value, attribute_name, true, element_name);
        }
    }

    // loop over attributes and parse arguments
    auto attributes = attributesOf(element, attribute_name);
    for (const auto& attribute : attributes) {
        // if attribute value matches, replace value!
        auto value_str = std::string(attribute->Value());
        count += std::replace(&value_str, old_value, new_value);
        element->SetAttribute(attribute->Name(), value_str.data());
    }

    // if (element) {

    //     // if element_name was given and doesn't match, skip
    //     if (!element_name.empty() && element_name.compare(element->Name()) != 0) {
    //         return 0;
    //     }
    //     // loop over attributes
    //     // const XMLAttribute* attribute = element->FirstAttribute();
    //     // while (attribute) {
    //     //     if (!attribute_name.empty() && attribute_name.compare(attribute->Name()) != 0 /* || !attribute->Value() */) {
    //     //         // move to next attribute
    //     //         attribute = attribute->Next();
    //     //         continue;
    //     //     }
    //     //     // if attribute value matches, replace value!
    //     //     auto value_str = std::string(attribute->Value());
    //     //     count += std::replace(&value_str, old_value, new_value);
    //     //     element->SetAttribute(attribute->Name(), value_str.data());
    //     //     // move to next attribute
    //     //     attribute = attribute->Next();
    //     // }
    //     // @note attributesOf(...) filters according to attribute name

    // }

    return count;
}



//------------------------------------------------------------------------------
/// @brief      Replaces text string in attribute values (text) with given value, for attribute of given name (optional).
///
/// @param      element         XML Element.
/// @param[in]  old_value       Attribute text to replace.
/// @param[in]  new_value       New attribute value (replacement)
/// @param[in]  attribute_name  Optional attribute name for filtering. Defaults to empty string, where all attributes are considered.
/// @param[in]  element_name    Optional element name for filtering. Defaults to empty string, wherein all element are considered.
///
/// @return     Number of changes.
///
/// @note       At this stage can't be wrapped around attributesOf(...) because of const return type!
///
/// @note       Multiple element non-recursive overload.
///
inline size_t replaceInAttributesOf(const std::vector< XMLElement* > elements, const std::string& old_value, const std::string& new_value, const std::string& attribute_name = "", const std::string& element_name = "") {
    size_t count = 0;

    // loop over attributes
    for (auto& element : elements) {
        if (!element) {
            continue;
        }
        // if element_name was given and doesn't match, skip
        if (!element_name.empty() && element_name.compare(element->Name()) != 0) {
            continue;
        }
        // call single element overload with replacement implementation
        count += replaceInAttributesOf(element, old_value, new_value, attribute_name, false, element_name);
    }

    return count;
}



//------------------------------------------------------------------------------
/// @brief      Gets the content of a given XML element as a vector of (numerical) values.
///
/// @param[in]  element    XML element
/// @param[in]  separator  Separator/delimitar charater. Defaults to space.
///
/// @tparam     T          Content data type. Text data must be cast-able to numerical type.
///
/// @return     The list at.
///
template< typename T >
std::vector< T > getDataAt(const XMLElement* element, char separator = ' ') {
    std::vector< T > list;
    if (element) {
        auto text = std::string(element->GetText());
        auto vals = tokenize(text, separator);
        for (const auto& val : vals) {
            list.emplace_back(static_cast< T >(atof(val.data())));
        }
    }
    return list;
}



//------------------------------------------------------------------------------
/// @brief      Inputs *numerical* data into a XML Element content.
///
/// @param[in]  element    XML element.
/// @param[in]  data       Data vector/container.
/// @param[in]  separator  Separator/delimitar charater. Defaults to space.
///
/// @tparam     T          Input data type. Must be numerical, otherwise it won't compile.
///
/// @return     True if success, False otherwise.
///
template< typename T, std::enable_if_t< std::is_arithmetic< T >::value, bool > = true >
bool setDataAt(XMLElement* element, const std::vector< T >& data, char separator = ' ') {
    if (element) {
        // construct data string
        std::string text = "";
        for (int idx = 0; idx < data.size(); idx++) {
            text += std::to_string(data[idx]);
            if (idx < data.size() - 1) {
                text += separator;
            }
        }
        // set text on element
        element->SetText(text.data());

        return true;
    }

    return false;
}



//------------------------------------------------------------------------------
/// @brief      Inputs data into a XML Element content.
///
/// @param[in]  element    XML element.
/// @param[in]  data       Data vector/container.
/// @param[in]  separator  Separator/delimitar charater. Defaults to space.
///
/// @tparam     T          Input data type. Can't be numerical/arithmetic.
///
/// @return     True if success, False otherwise.
///
/// @note       Overload for non-numeric arguments (must be convertible to std::string)
///
template < typename T, std::enable_if_t< !std::is_arithmetic< T >::value, bool > = true >
bool setDataAt(XMLElement* element, const std::vector< T >& data, char separator = ' ') {
    if (element) {
        // construct data string
        std::string text = "";
        for (int idx = 0; idx < data.size(); idx++) {
            text += std::string(data[idx]);
            if (idx < data.size() - 1) {
                text += separator;
            }
        }
        // set text on element
        element->SetText(text.data());

        return true;
    }

    return false;
}



//------------------------------------------------------------------------------
/// @brief      Inputs data into a XML Element content.
///
/// @param[in]  element    XML element.
/// @param[in]  data       Data container, as a brace-enclosed list.
/// @param[in]  separator  Separator/delimitar charater. Defaults to space.
///
/// @tparam     T          Input data type. Must be numerical cast-able to std::string.
///
/// @return     True if success, False otherwise.
/// 
/// @note       Overload provided for conveninece, allowing brace-enclosed list as data argument.
///
template< typename T >
bool setDataAt(XMLElement* element, const std::initializer_list< T >& data, char separator = ' ') {
    return setDataAt< T >(element, std::vector< T >(data), separator);
}


}  // namespace extra

}  // namespace tinyxml2


#endif  //  _INCLUDE_UTILS_TINYXML2UTILS_HPP_
