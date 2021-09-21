/*----------------------------------------------------------------------------*/

/*!
\file   property_container.h
\author Taylor Guy
\date   2021
\par    email: taylor.a.guy.mkb\@gmail.com
\brief
    This file contains the prototypes of the following function(s) for:
        Wrapper Class for Templated Serialization Container Objects

        ------------------------------------------------------------------------
        Example of creating and adding a property container to a class member:
        ------------------------------------------------------------------------

        @code
            //Given the example struct in the "How to" of property.h
        std::vector<example*>& _ex = 
            properties::add<PSL::vector<example, ...>>("Example")

            // PSL::vector<example, ...> is equivalent to 
            // property_container<example, std::vector, ...>

            // The ellipses of the above template are for overriding the STL or
            // user-defined container's defaulted template parameters.
        @endcode

        Corresponding object loaded/saved in JSON file:
        @code
            "Example":
            [
                { "Name" : 1 },
                ...             // Extra objects stored in the vector.
            ]
        @endcode
*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Guard                                                                      */
/*----------------------------------------------------------------------------*/
#pragma once

#ifndef PSL_PROPERTY_CONTAINER_H
#define PSL_PROPERTY_CONTAINER_H    //!< Header Protection

/*----------------------------------------------------------------------------*/
/* Include                                                                    */
/*----------------------------------------------------------------------------*/

#include "property.h"

    // Currently support only for these containers.
#include <vector>
#include <deque>
#include <list>

/*----------------------------------------------------------------------------*/
/* Namespace                                                                  */
/*----------------------------------------------------------------------------*/

    //! Property Serialization Library Namespace
namespace PSL
{

/*----------------------------------------------------------------------------*/
/* Defines and Typedefs                                                       */
/*----------------------------------------------------------------------------*/

        //! Short vector container property call.
    template<class type, class... args>
        using vector = property_container<type, std::vector, args...>;

        //! Short deque container property call.
    template<class type, class... args>
        using deque = property_container<type, std::deque, args...>;

        //! Short list container property call.
    template<class type, class... args>
        using list = property_container<type, std::list, args...>;

/* Forward Declarations ------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Public Functions                                                           */
/*----------------------------------------------------------------------------*/

/* Global Objects ------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
    /*!
     * @class   PSL::property_container
     *
     * @tparam  type        Type to be contained. Must be default constructable
     *                          and derived from the properties class.
     * @tparam  container   Container type.
     * @tparam  args        Extra container template arguments.
     *
     * @brief   A class for container type automated de/serialization.
     *
     * @brief   <b>Notes:</b>
     *              - The _container holds pointers to "types". It is this
     *                  type that is referred to when loading/saving with JSON.
     *              - Below is the formula for adding a container type to a
     *                  de/serializable object using the same types as described
     *                  in the property_container class:
     * 
     *          @note Ex:  std::container<type*, args...>& _ex 
     *                      = properties::add<PSL::container<type, args...>>
     *                          ("Name");
     *          @note Ex: std::container<type*, args...>& _ex
     *                      = properties::add<
     *                          property_container<type, container, args...>>
     *                              ("Name");
     */
/*----------------------------------------------------------------------------*/
        template<class type, template<class...> class container, class... args>
    class property_container : public property_abs
    {
    public:

            //! Shorthand for internal container type.
        using p_container = container<type*, args...>;

    private:

            //! Stores objects that can de/serialize with JSON.
        p_container _container;

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for clearing _container and growing it to the 
     *          specified size.
     * @param   arg_size  New size of container.
     */
  /*--------------------------------------------------------------------------*/
        void grow(size_t arg_size)
        {
            PROPERTY_CONTAINER::clear(*this);
            p_container l_container(arg_size, nullptr);
            _container.swap(l_container);
        }
    
    public:
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Default Constructor
     * @param   argcp_name  Name of corresponding container stored in JSON file.
     */
  /*--------------------------------------------------------------------------*/
        property_container(string argcp_name = nullptr) :
            property_abs(argcp_name), _container()
        {
                // Check that we can treat given type as de/serializable type.
            static_assert(std::is_base_of<properties, type>::value &&
                          std::is_default_constructible<type>::value, 
                "Given <type> does not meet requirements for "
                "property_container use. "
                "<type> MUST be default constructable. "
                "<type> MUST derive from properties.");
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Copy Constructor
     * @param   argcr_obj   Property to copy.
     */
  /*--------------------------------------------------------------------------*/
        property_container(const PROPERTY_CONTAINER& argcr_obj) :
            property_abs(argcr_obj), _container()
        { 
            assign(argcr_obj); // Deep copy of container.
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Destructor
     */
  /*--------------------------------------------------------------------------*/
        virtual ~property_container()
        { 
            PROPERTY_CONTAINER::clear(*this); // Deallocates allocated pointers.
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for cloning property_container objects.
     * @return  Pointer to newly allocated property_container.
     */
  /*--------------------------------------------------------------------------*/
        property_abs* clone() const
        {
            return new PROPERTY_CONTAINER(*this);
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for deep copying values from another 
     *          property_container.
     * @param   argcr_obj  Reference to object to clone values from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void assign(const property_abs& argcr_obj)
        {
            const p_container& _other = *argcr_obj.cast<PROPERTY_CONTAINER>();
            grow(_other.size()); // Clear and grow to appropriate size.

            for_both(_container, _other,
                [](auto arg_it1, auto arg_it2)
                {
                    if (*arg_it2 != nullptr)
                        *arg_it1 = dynamic_cast<type*>((*arg_it2)->clone()); 
                });
        }

/* Handle Functions ----------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for loading saved values into container. Assumes 
     *          the passed object need only be treated as the object for the 
     *          internal container to parse.
     * @param   argcr_obj   JSON object to load from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void load(const JSON& argcr_obj)
        {
            if (!property_abs::_name)
                std::runtime_error l_err("No name found for this property!");

            std::vector<JSON> l_json = argcr_obj.get<std::vector<JSON>>();
            grow(l_json.size());

            for_both(_container, l_json,
                [](auto arg_it1, auto arg_it2)
                {
                    (*arg_it1) = new type();
                    (*arg_it1)->load(*arg_it2);
                });
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for saving container properties to a JSON object.
     * @return  JSON object of property name and properties.
     */
  /*--------------------------------------------------------------------------*/
        virtual JSON save() const
        {
            JSON l_obj;
            std::string l_name(type().get_name());

            if (property_abs::_name)
            {
                JSON l_parent = JSON::array();

                for (auto it : _container)
                    l_parent.push_back(it->save().front());

                l_obj[property_abs::_name] = l_parent;
            }
            else
                for (auto it : _container)
                    l_obj.push_back(it->save().front());

            return l_obj;
        }

/* Get Functions -------------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for explicitly returning the property member variable.
     * @return  Reference to property member variable.
     */
  /*--------------------------------------------------------------------------*/
        p_container& get_container()
        {
            return _container;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for explicitly returning the property member variable.
     * @return  Reference to constant property member variable.
     */
  /*--------------------------------------------------------------------------*/
        const p_container& get_container() const
        {
            return _container;
        }

/* Conversion Functions ------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for implicitly returning the property member variable.
     * @return  Reference to property member variable.
     */
  /*--------------------------------------------------------------------------*/
        operator p_container& ()
        {
            return _container;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for implicitly returning the property member variable.
     * @return  Reference to constant property member variable.
     */
  /*--------------------------------------------------------------------------*/
        operator const p_container& () const
        {
            return _container;
        }

/* Override Functions --------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Static function for clearing any container of pointers to
     *          properties-derived type. Ensures memory is properly released.
     * @param   argr_container  Container to clear.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        static void clear(p_container& argr_container)
        {
            PSL::for_each(argr_container,
                [](auto arg_it) 
                { 
                    if (arg_it) 
                    {
                        delete arg_it;
                        arg_it = nullptr;
                    }
                });
            argr_container.clear();
        }

/* Overload Functions --------------------------------------------------------*/

    };

/*----------------------------------------------------------------------------*/
    /*!
     * @class   PSL::property_array
     *
     * @tparam  type    Type to be contained. Must be default constructable
     *                          and derived from the properties class.
     * @tparam  size    Size of internal array.
     *
     * @brief   A class for array type automated de/serialization.
     *
     * @brief   <b>Notes:</b>
     *              - The _container holds pointers to "types". It is this 
     *                  type that is referred to when loading/saving with JSON.
     *              - Below is the formula for adding an array type to a
     *                  de/serializable object using the same types as described
     *                  in the property_array class:
     * 
     *          @note Ex:  std::array<type*, size>& _ex 
     *                      = properties::add<PSL::array<type, size>>("Name");
     *          @note Ex:  std::array<type*, size>& _ex
     *                      = properties::add<property_array<type, size>>
     *                          ("Name");
     */
/*----------------------------------------------------------------------------*/
        template<class type, size_t size>
    class property_array : public property_abs
    {
    public:

            //! Shorthand for internal array type.
        using p_container = std::array<type*, size>;

    private:

            //! Stores objects that can de/serialize with JSON.
        p_container _container;
    
    public:
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Default Constructor
     * @param   argcp_name  Name of corresponding container stored in JSON file.
     */
  /*--------------------------------------------------------------------------*/
        property_array(string argcp_name = nullptr) :
            property_abs(argcp_name), _container()
        {
                // Check that we can treat given type as de/serializable type.
            static_assert(std::is_base_of<properties, type>::value &&
                          std::is_default_constructible<type>::value, 
                "Given <type> does not meet requirements for "
                "property_container use. "
                "<type> MUST be default constructable. "
                "<type> MUST derive from properties.");
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Copy Constructor
     * @param   argcr_obj   Property to copy.
     */
  /*--------------------------------------------------------------------------*/
        property_array(const PROPERTY_ARRAY& argcr_obj) :
            property_abs(argcr_obj), _container()
        { 
            assign(argcr_obj); // Deep copy of container.
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Destructor
     */
  /*--------------------------------------------------------------------------*/
        virtual ~property_array()
        { 
            PROPERTY_ARRAY::clear(*this); // Deallocates allocated pointers.
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for cloning property_array objects.
     * @return  Pointer to newly allocated property_array.
     */
  /*--------------------------------------------------------------------------*/
        property_abs* clone() const
        {
            return new PROPERTY_ARRAY(*this);
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for deep copying values from another property_array.
     * @param   argcr_obj  Reference to object to clone values from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void assign(const property_abs& argcr_obj)
        {
            const p_container& _other = *argcr_obj.cast<PROPERTY_ARRAY>();
            PROPERTY_ARRAY::clear(_container);

            for_both(_container, _other,
                [](auto arg_it1, auto arg_it2)
                { 
                    if(*arg_it2 != nullptr)
                        *arg_it1 = dynamic_cast<type*>((*arg_it2)->clone()); 
                });
        }

/* Handle Functions ----------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for loading saved values into container. Assumes 
     *          the passed object need only be treated as the object for the 
     *          internal container to parse.
     * @param   argcr_obj   JSON object to load from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void load(const JSON& argcr_obj)
        {
            if (!property_abs::_name)
                std::runtime_error l_err("No name found for this property!");

            std::array<JSON, size> l_json 
                = argcr_obj.get<std::array<JSON, size>>();
            PROPERTY_ARRAY::clear(_container);

            for_both(_container, l_json,
                [](auto arg_it1, auto arg_it2)
                {
                    (*arg_it1) = new type();
                    (*arg_it1)->load(*arg_it2);
                });
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for saving array properties to a JSON object.
     * @return  JSON object of property name and properties.
     */
  /*--------------------------------------------------------------------------*/
        virtual JSON save() const
        {
            JSON l_obj;
            std::string l_name(type().get_name());

            if (property_abs::_name)
            {
                JSON l_parent = JSON::array();

                for (auto it : _container)
                    l_parent.push_back(it->save().front());

                l_obj[property_abs::_name] = l_parent;
            }
            else
                for (auto it : _container)
                    l_obj.push_back(it->save().front());

            return l_obj;
        }

/* Get Functions -------------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for explicitly returning the property member variable.
     * @return  Reference to property member variable.
     */
  /*--------------------------------------------------------------------------*/
        p_container& get_container()
        {
            return _container;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for explicitly returning the property member variable.
     * @return  Reference to constant property member variable.
     */
  /*--------------------------------------------------------------------------*/
        const p_container& get_container() const
        {
            return _container;
        }

/* Conversion Functions ------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for implicitly returning the property member variable.
     * @return  Reference to property member variable.
     */
  /*--------------------------------------------------------------------------*/
        operator p_container& ()
        {
            return _container;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for implicitly returning the property member variable.
     * @return  Reference to constant property member variable.
     */
  /*--------------------------------------------------------------------------*/
        operator const p_container& () const
        {
            return _container;
        }

/* Override Functions --------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Static function for clearing arrays of pointers to
     *          properties-derived type. Ensures memory is properly released.
     * @param   argr_container  Array to clear.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        static void clear(p_container& argr_container)
        {
            PSL::for_each(argr_container,
                [](auto arg_it) 
                { 
                    if (arg_it) 
                    {
                        delete arg_it;
                        arg_it = nullptr;
                    }
                });
        }

/* Overload Functions --------------------------------------------------------*/

    };
}

#endif