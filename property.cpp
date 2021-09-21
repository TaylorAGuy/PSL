/*----------------------------------------------------------------------------*/

/*!
\file   property.cpp
\author Taylor Guy
\date   2021
\par    email: taylor.a.guy.mkb\@gmail.com
\brief
    This file contains the implementations of the following function(s) for:
        Wrapper Class for Templated Serialization Objects
*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include                                                                    */
/*----------------------------------------------------------------------------*/

#include "property.h"

#include <iomanip>

/* Forward Declarations ------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines and Typedefs                                                       */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Namespace                                                                  */
/*----------------------------------------------------------------------------*/

    //! Property Serialization Library Namespace
namespace PSL
{
/* Local Objects -------------------------------------------------------------*/

/* Global Objects ------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Public Functions                                                           */
/*----------------------------------------------------------------------------*/

        //! Function for merging two JSON objects.
    JSON& merge_JSON(JSON& argr_parent, const JSON& argcr_add)
    {
        // Identical property names will be overwritten by argcr_add.
        for (auto l_it = argcr_add.begin(); l_it != argcr_add.end(); ++l_it)
            argr_parent[l_it.key()] = l_it.value();

        return argr_parent;
    }

        //! Function for converting a JSON file into a JSON object.
    JSON load_JSON(const std::string& argcr_file)
    {
        try
        {
            std::ifstream l_in(argcr_file);

            if (!l_in.is_open() || !l_in.good())
                std::runtime_error l_err("File for deserialization not found!");

            return JSON::parse(l_in);
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            std::runtime_error l_err("File for deserialization has an error!");
        }

        return JSON();
    }

        //! Function for saving a JSON object to a JSON file.
    void save_JSON(const std::string& argcr_file, const JSON& argcr_obj)
    {
        if (argcr_obj.empty())
        {
            std::cerr << "No file serialized: JSON object empty." << std::endl;
            return;
        }

        try
        {
            std::ofstream l_out(argcr_file);

            if (!l_out.is_open() || !l_out.good())
                std::runtime_error l_err("Error in opening serialization file!");

            l_out << std::setw(4) << argcr_obj;
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            std::runtime_error l_err("File for serialization has an error!");
        }
    }

}

/*----------------------------------------------------------------------------*/
/* Public Functions - property_abstract                                       */
/*----------------------------------------------------------------------------*/

/* Constructors and Destructors ----------------------------------------------*/

    //! Default Constructor
PSL::property_abstract::property_abstract(string argcp_name) :
    _name(argcp_name)
{}
    //! Copy Constructor
PSL::property_abstract::property_abstract(const property_abs& argcr_obj) :
    _name(argcr_obj._name)
{}
    //! Destructor
PSL::property_abstract::~property_abstract()
{}

/* Get Functions -------------------------------------------------------------*/
    
    //! Function for retrieving the corresponding JSON name.
PSL::string PSL::property_abstract::get_name() const
{
    return _name;
}

/*----------------------------------------------------------------------------*/
/* Private Functions - properties                                             */
/*----------------------------------------------------------------------------*/

    //! Function that adds an individual property to the internal properties 
    //! storage or overwrites those that already exist under the same name. 
    //! Deletes the old property in this case.
PSL::property_abs* PSL::properties::add(PSL::property_abs* argp_obj)
{
    remove(argp_obj->get_name());
    _properties[argp_obj->get_name()] = argp_obj;

    return argp_obj;
}

/*----------------------------------------------------------------------------*/
/* Public Functions - properties                                              */
/*----------------------------------------------------------------------------*/

/* Constructors and Destructors ----------------------------------------------*/

    //! Default Constructor
PSL::properties::properties(string argcp_name) :
    property_abs(argcp_name), _properties()
{}

    //! Copy Constructor
PSL::properties::properties(const properties& argcr_obj) :
    property_abs(argcr_obj), _properties()
{
    for (auto it : argcr_obj._properties)
        _properties.insert({ it.first, it.second->clone() });
}

    //! Destructor
PSL::properties::~properties()
{
    for (auto it : _properties)
        delete it.second;

    _properties.clear();
}

    //! Function for cloning property<type> objects.
PSL::property_abs* PSL::properties::clone() const
{
    return new properties(*this);
}

    //! Abstract Function for polymorphic assigning.
void PSL::properties::assign(const property_abs& argcr_obj)
{
    for_each(argcr_obj.cast<properties>()->_properties,
        [this](auto arg_it)
        {
            auto l_it = this->_properties.find(arg_it.first);

            if (l_it != this->_properties.end())
                this->_properties[arg_it.first]->assign(*arg_it.second);
        });
}

/* Handle Functions ----------------------------------------------------------*/

    //! Function for loading saved value into properties map. Assumes the passed 
    //! object need only be treated as the object for the internal properties to 
    //! parse.
void PSL::properties::load(const JSON& argcr_obj)
{
    try
    {
        for (auto l_it : argcr_obj.items())
        {
            auto l_pit = _properties.find(l_it.key());
        
            if (l_pit != _properties.end())
                l_pit->second->load(l_it.value());
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::runtime_error l_err("File for deserialization has an error with type!");
    }
    
}

    //! Function for saving properties map to a JSON object.
PSL::JSON PSL::properties::save() const
{
    JSON l_obj;
    
    if (property_abs::_name)
    {
        JSON l_parent;

        for (auto it : _properties)
            merge_JSON(l_parent, it.second->save());

        l_obj[property_abs::_name] = l_parent;
    }
    else
        for (auto it : _properties)
            merge_JSON(l_obj, it.second->save());

    return l_obj;
}

/* Remove Functions ----------------------------------------------------------*/

    //! Function for deallocating and removing a property derived type. 
void PSL::properties::remove(string argcr_name)
{
    auto l_it = _properties.find(argcr_name);

    if (l_it != _properties.end())
    {
        delete l_it->second;
        l_it->second = nullptr;
        _properties.erase(l_it);
    }
}
