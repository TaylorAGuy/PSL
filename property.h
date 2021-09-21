/*----------------------------------------------------------------------------*/

/*!
\file   property.h
\author Taylor Guy
\date   2021
\par    email: taylor.a.guy.mkb\@gmail.com
\brief
    This file contains the prototypes of the following function(s) for:
        Wrapper Class for Templated Serialization Objects

        ------------------------------------------------------------------------
        Example of derived properties class or struct:
        ------------------------------------------------------------------------
        
        @code
            using namespace PSL;

                // Can be copied as a template.
            struct example : public properties 
            {
                    // Example of de/serializable property.
                int& _ex = properties::add<int>("Name", 0);
                    // The add() functions will always return a reference to
                    // the internal type of the property so as to bypass the
                    // need for the property wrapper to override interactive 
                    // functionality for the type. Properties and 
                    // properties-derived types will return themselves.
        
                    // Required Constructors - Default and Copy
                    // (May require specialized implementation by user)
                example() : properties("Example") {}
                example(const example& argcr_obj) : properties(argcr_obj) 
                { 
                    properties::assign(argcr_obj); 
                }
        
                    // Required Override
                    // (May require specialized implementation by user)
                property_abs* clone() const { return new example(*this); }
        
                    // Optional Overrides
                    // (Require specialized implementation by user)
                ~example();
                void load(const JSON&);
                JSON save() const;
            };
        @endcode

        Corresponding object loaded/saved in JSON file:
        @code
            "Example": { "Name" : 1 }
        @endcode

        ------------------------------------------------------------------------
        Example of creating an individual object and loading/saving said object:
        ------------------------------------------------------------------------
        
        @code
                // Takes given path to .json file and converts to JSON object.
            JSON l_json = load_JSON(FILEPATH);
        
                // Supports objects created on stack or heap.
            example l_ex;       // l_ex._ex == 0
                // Read saved value in .json file.
            l_ex.load(l_json);  // l_ex._ex == 1
        
                // Modify the value within the example class through its
                // reference to the property created at construction.
            l_ex._ex = 2;       // l_ex._ex == 2
        
                // Takes given path to .json file (Either a new or existing
                // file) and serializes the JSON object created by the save
                // member function.
            save_JSON(FILEPATH, l_ex.save());
        @endcode
*/

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Guard                                                                      */
/*----------------------------------------------------------------------------*/
#pragma once

#ifndef PSL_PROPERTY_H
#define PSL_PROPERTY_H      //!< Header Protection

/*----------------------------------------------------------------------------*/
/* Include                                                                    */
/*----------------------------------------------------------------------------*/

#include <unordered_map>    // Used by "properties" class

#include <fstream>
#include <iostream>

#include "json.hpp"

/*----------------------------------------------------------------------------*/
/* Namespace                                                                  */
/*----------------------------------------------------------------------------*/

    //! Property Serialization Library Namespace
namespace PSL
{
/* Forward Declarations ------------------------------------------------------*/

    class property_abstract;

        template<typename type>
    class property;

        template<class type, template<class...> class container, class... args>
    class property_container;

        template<class type, std::size_t size>
    class property_array;

    class properties;

/*----------------------------------------------------------------------------*/
/* Defines and Typedefs                                                       */
/*----------------------------------------------------------------------------*/

    using string     = const char*;          //!< Short string call.
    using JSON       = nlohmann::json;       //!< Short JSON call.
    using property_abs = property_abstract;  //!< Short abstract property call.

    //! Short template for templated container property call.
#define PROPERTY_CONTAINER_TEMPLATE \
    template<class type, template<class...> class container, class... args>
    //! Short templated container property call.
#define PROPERTY_CONTAINER property_container<type, container, args...>

    //! Short array property call.
        template<class type, std::size_t size>
    using array = property_array<type, size>;

    //! Short template for templated array property call.
#define PROPERTY_ARRAY_TEMPLATE \
        template<class type, std::size_t size>
    //! Short templated array property call.
#define PROPERTY_ARRAY property_array<type, size>

/*----------------------------------------------------------------------------*/
/* Public Functions                                                           */
/*----------------------------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for merging two JSON objects.
     * @param   argr_parent JSON object to be modified.
     * @param   argcr_add   JSON object containing values to be added to the 
                            parent.
     * @return              Modified parent JSON.
     */
  /*--------------------------------------------------------------------------*/
    JSON& merge_JSON(JSON& argr_parent, const JSON& argcr_add);
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for converting a JSON file into a JSON object.
     * @param   argcr_file  File path to open JSON file.
     * @return              JSON object parsed from JSON file.
     */
  /*--------------------------------------------------------------------------*/
    JSON load_JSON(const std::string& argcr_file);
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for saving a JSON object to a JSON file.
     * @param   argcr_file  File path to create or overwrite JSON file.
     * @param   argcr_obj   JSON object containing values to be saved.
     */
  /*--------------------------------------------------------------------------*/
    void save_JSON(const std::string& argcr_file, const JSON& argcr_obj);

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Loops and applies behavior over two containers.
     * @param   arg_C   Container to iterate.
     * @param   arg_CO  Second Container to iterate.
     * @param   arg_fn  Binary function to apply to both containers' iterators.
     */
  /*--------------------------------------------------------------------------*/
    template<class container, class other_container, class binary_fn>
    void for_both(container& arg_C, other_container& arg_CO, binary_fn arg_fn)
    {
        for (auto l_it = std::make_pair(arg_C.begin(), arg_CO.begin());
             l_it.first != arg_C.end() && l_it.second != arg_CO.end();
             ++l_it.first, ++l_it.second)
        {
            arg_fn(l_it.first, l_it.second);
        }
    }

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Loops and applies behavior over one container.
     * @param   arg_C   Container to iterate.
     * @param   arg_fn  Unary function to apply to container iterator.
     */
  /*--------------------------------------------------------------------------*/
        template<class container, class unary_function>
    void for_each(container& arg_C, unary_function arg_fn)
    {
        for (auto l_it = arg_C.begin(); l_it != arg_C.end(); ++l_it)
            arg_fn(*l_it);
    }

/* Global Objects ------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
    /*!
     * @class   PSL::property_abstract
     *
     * @brief   An abstract class for automated de/serialization.
     *
     * @brief   <b>Notes:</b>
     *              - The _name variable acts as the key for finding the
     *                  associated value in a JSON file.
     */
/*----------------------------------------------------------------------------*/
    class property_abstract
    {
    protected:

        string _name;   //!< Name of corresponding value stored in JSON file.

    public:

/* Constructors and Destructors ----------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Default Constructor
     * @param   argcp_name  Name of corresponding value stored in JSON file.
     */
  /*--------------------------------------------------------------------------*/
        property_abstract(string argcp_name);
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Copy Constructor
     * @param   argcr_obj   Property to copy.
     */
  /*--------------------------------------------------------------------------*/
        property_abstract(const property_abs& argcr_obj);
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Destructor
     */
  /*--------------------------------------------------------------------------*/
        virtual ~property_abstract();
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Abstract Function for polymorphic cloning.
     * @return  Pointer to newly allocated property.
     */
  /*--------------------------------------------------------------------------*/
        virtual property_abs* clone() const = 0;

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Abstract Function for polymorphic assigning.
     * @param   argcr_obj  Reference to object to assign values from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void assign(const property_abs& argcr_obj) = 0;

/* Handle Functions ----------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Abstract Function for polymorphic loading.
     * @param   arg_obj     JSON object to parse values from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void load(const JSON& arg_obj) = 0;
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Abstract Function for polymorphic saving.
     * @return  JSON object of saved values.
     */
  /*--------------------------------------------------------------------------*/
        virtual JSON save() const = 0;

/* Get Functions -------------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for retrieving the corresponding JSON name.
     * @return  PSL::string of property name.
     */
  /*--------------------------------------------------------------------------*/
        string get_name() const;

/* Helper Structs ------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
    /*!
     * @tparam  type    Type to be defined.
     *
     * @brief   A templated struct for easy type casting.
     */
/*----------------------------------------------------------------------------*/
            template<class type>
        struct cast_return 
        { 
                //! Typedef for specialized return.
            typedef property<type> type;    
        };

    //! DEFINE for casting return type.
#define PSL_CAST_RETURN(x) typename cast_return<x>::type
        
/*----------------------------------------------------------------------------*/
    /*!
     * @brief   A specialized templated struct for easy type casting.
     */
/*----------------------------------------------------------------------------*/
            template<>
        struct cast_return<properties> 
        { 
                //! Typedef for specialized return.
            typedef properties type;
        };

/*----------------------------------------------------------------------------*/
    /*!
     * @tparam  type        Type to be contained.
     * @tparam  container   Container type.
     * @tparam  args        Extra container template arguments.
     *
     * @brief   A specialized templated struct for easy type casting of 
     *          property containers.
     */
/*----------------------------------------------------------------------------*/
            PROPERTY_CONTAINER_TEMPLATE
        struct cast_return<PROPERTY_CONTAINER>
        {
                //! Typedef for specialized return.
            typedef PROPERTY_CONTAINER type;
        };

/*----------------------------------------------------------------------------*/
    /*!
     * @tparam  type    Type to be contained.
     * @tparam  size    Size of array.
     *
     * @brief   A specialized templated struct for easy type casting of 
     *          property arrays.
     */
/*----------------------------------------------------------------------------*/
            PROPERTY_ARRAY_TEMPLATE
        struct cast_return<PROPERTY_ARRAY>
        {
                //! Typedef for specialized return.
            typedef PROPERTY_ARRAY type;
        };

/* Cast Functions ------------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for casting property_abstract to specified property 
                type.
     * @return  Pointer to casted property<type>.
     */
  /*--------------------------------------------------------------------------*/
            template<typename type>
        PSL_CAST_RETURN(type)* cast()
            { return dynamic_cast<PSL_CAST_RETURN(type)*>(this); }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for casting property_abstract to specified property 
                type.
     * @return  Pointer to constant casted property<type>.
     */
  /*--------------------------------------------------------------------------*/
            template<typename type> 
        const PSL_CAST_RETURN(type)* cast() const
            { return dynamic_cast<const PSL_CAST_RETURN(type)*>(this); }

#undef PSL_CAST_RETURN // UNDEFINE for casting return type.

    };

/*----------------------------------------------------------------------------*/
    /*!
     * @class   PSL::property
     * 
     * @tparam  type    Type to be defined.
     *
     * @brief   A class for basic type automated de/serialization.
     *
     * @brief   <b>Notes:</b>
     *              - The _value holds a variable of "type". It is this variable
     *                  that is referred to when loading/saving with JSON.
     */
/*----------------------------------------------------------------------------*/
        template<typename type>
    class property : public property_abs
    {
        type _value;    //!< Value for de/serialization.

    public:

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Default Constructor
     * @param   argcp_name  Name of corresponding value stored in JSON file.
     */
  /*--------------------------------------------------------------------------*/
        property(string argcp_name) :
            property_abs(argcp_name), _value()
        {}
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Initializing Constructor
     * @param   argcp_name      Name of corresponding value stored in JSON file.
     * @param   argcr_default   Default value in the case value is not stored 
     *                          in a JSON file.
     */
  /*--------------------------------------------------------------------------*/
        property(string argcp_name, const type& argcr_default) :
            property_abs(argcp_name), _value(argcr_default)
        {}
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Copy Constructor
     * @param   argcr_obj   Property to copy.
     */
  /*--------------------------------------------------------------------------*/
        property(const property<type>& argcr_obj) :
            property_abs(argcr_obj), _value(argcr_obj._value)
        {}
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Destructor
     */
  /*--------------------------------------------------------------------------*/
        virtual ~property()
        {}
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for cloning property<type> objects.
     * @return  Pointer to newly allocated property.
     */
  /*--------------------------------------------------------------------------*/
        property_abs* clone() const
        {
            return new property<type>(*this);
        }

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for copying values from another property.
     * @param   argcr_obj  Reference to object to assign values from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void assign(const property_abs& argcr_obj)
        {
            _value = argcr_obj.cast<type>()->_value;
        }

/* Handle Functions ----------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for loading saved value into property member variable.
     *          Assumes the passed object need only be converted to desired 
     *          type.
     * @param   argcr_obj   JSON object to load from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void load(const JSON& argcr_obj)
        {
            if (!property_abs::_name)
                std::runtime_error l_err("No name found for this property!");
            _value = argcr_obj.get<type>();
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for saving property member value to a JSON object.
     * @return  JSON object of property name and property member variable.
     */
  /*--------------------------------------------------------------------------*/
        virtual JSON save() const
        {
            return JSON{ { property_abs::_name, _value } };
        }

/* Get Functions -------------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for explicitly returning the property member variable.
     * @return  Reference to property member variable.
     */
  /*--------------------------------------------------------------------------*/
        type& get_value()
        {
            return _value;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for explicitly returning the property member variable.
     * @return  Reference to constant property member variable.
     */
  /*--------------------------------------------------------------------------*/
        const type& get_value() const
        {
            return _value;
        }

/* Conversion Functions ------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for implicitly returning the property member variable.
     * @return  Reference to property member variable.
     */
  /*--------------------------------------------------------------------------*/
        operator type& ()
        {
            return _value;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for implicitly returning the property member variable.
     * @return  Reference to constant property member variable.
     */
  /*--------------------------------------------------------------------------*/
        operator const type& () const
        {
            return _value;
        }

/* Overload Functions --------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for assigning one property member value to another
     *          of the same type. This does not update the name of the property 
     *          which is only changed at construction.
     * @param   argcr_obj   Property to copy new value from.
     * @return              Reference to updated property.
     */
  /*--------------------------------------------------------------------------*/
        property<type>& operator=(const property<type>& argcr_obj)
        {
            _value = argcr_obj._value;
            return *this;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for assigning property member variable to new value.
     * @param   argcr_val   New value for property member variable.
     * @return              Reference to updated property.
     */
  /*--------------------------------------------------------------------------*/
        property<type>& operator=(const type& argcr_val)
        {
            _value = argcr_val;
            return *this;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for testing equivalence between two property member
     *          variables.
     * @param   argcr_obj   Property to test against.
     * @return              Whether both property member variables are equal.
     */
  /*--------------------------------------------------------------------------*/
        bool operator==(const property<type>& argcr_obj) const
        {
            if (&argcr_obj == this) return true;
            return _value == argcr_obj._value;
        }
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for testing non-equivalence between two property member
     *          variables.
     * @param   argcr_obj   Property to test against.
     * @return              Whether both property member variables aren't equal.
     */
  /*--------------------------------------------------------------------------*/
        bool operator!=(const property<type>& argcr_obj) const
        {
            if (&argcr_obj == this) return false;
            return _value != argcr_obj._value;
        }

    };

/*----------------------------------------------------------------------------*/
    /*!
     * @class   PSL::properties
     *
     * @brief   A class for deriving and automated de/serialization.
     *
     * @brief   <b>Notes:</b>
     *              - The _properties value maps and manages de/serialization 
     *                  for every property belonging to the same JSON object.
     *
     *              - For ease of implementation of the property classes,
     *                  user-defined types needing de/serialization should 
     *                  publicly derive from this class. Use the "example" class 
     *                  provided at the top of the file as a template for 
     *                  inheritance.
     *              - Anything that derives from this class MUST override the 
     *                  following:
     *                      - property_abs* clone() const
     *                          - It is recommended to allocate via the object's
     *                              copy constructor and use the 
     *                              properties::assign function in the 
     *                              constructor's body to safely copy and    
     *                              reference the property members of the class.
     *              - To add a new property for de/serialization to the base 
     *                  class, declare a reference of the desired type which is 
     *                  immediately initialize using the public add() functions.
     * 
     *          @note Ex:  int& _val_1     = properties::add<int>("Name", 0);
     *          @note Ex:  int& _val_2     = properties::add<int>("Name");
     *          @note Ex:  example& _val_3 
     *                      = properties::add<example>(example());
     *          @note Ex:  std::vector<example*>& _val_4 
     *                      = properties::add<PSL::vector<example>>("Name");
     * 
     */
/*----------------------------------------------------------------------------*/
    class properties : public property_abs
    {
    protected:

            //! Map for managing derived property classes.
        std::unordered_map<std::string, property_abs*> _properties;

    private:

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function that adds an individual property to the internal 
     *          properties storage or overwrites those that already exist 
     *          under the same name. Deletes the old property in this case.
     * @param   argp_obj    Pointer to object to add. Must be allocated outside 
     *                      function.
     * @return              Pointer to added object.
     */
  /*--------------------------------------------------------------------------*/
        property_abs* add(property_abs* argp_obj);

    public:
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Default Constructor
     * @param   argcp_name  Name of corresponding value stored in JSON file.
     */
  /*--------------------------------------------------------------------------*/
        properties(string argcp_name = nullptr);
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Copy Constructor
     * @param   argcr_obj   Property to copy.
     */
  /*--------------------------------------------------------------------------*/
        properties(const properties& argcr_obj);
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Destructor
     */
  /*--------------------------------------------------------------------------*/
        virtual ~properties();

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for cloning property<type> objects.
     * @return  Pointer to newly allocated property.
     */
  /*--------------------------------------------------------------------------*/
        virtual property_abs* clone() const;

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for copying values from another property.
     * @param   argcr_obj  Reference to object to assign values from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void assign(const property_abs& argcr_obj);

/* Handle Functions ----------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for loading saved value into properties map. Assumes 
     *          the passed object need only be treated as the object for the 
     *          internal properties to parse.
     * @param   argcr_obj   JSON object to load from.
     * @return
     */
  /*--------------------------------------------------------------------------*/
        virtual void load(const JSON& argcr_obj);
  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for saving properties map to a JSON object.
     * @return  JSON object of property name and properties.
     */
  /*--------------------------------------------------------------------------*/
        virtual JSON save() const;

/* Add Functions -------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
    /*!
     * @tparam  type    Type to be defined.
     *
     * @brief   A templated struct for easy type casting.
     */
/*----------------------------------------------------------------------------*/
            template<typename type>
        struct add_return
        {
            typedef type return_type;        //!< Add function return type.
            typedef type cast_type;          //!< Add function cast type.
            typedef property<type> add_type; //!< Add function allocation type.
        };

/*----------------------------------------------------------------------------*/
    /*!
     * @brief   A templated struct for easy type casting.
     */
/*----------------------------------------------------------------------------*/
            template<>
        struct add_return<properties>
        {
            typedef properties return_type; //!< Add function return type.
            typedef properties cast_type;   //!< Add function cast type.
            typedef properties add_type;    //!< Add function allocation type.
        };

/*----------------------------------------------------------------------------*/
    /*!
     * @tparam  type        Type to be contained.
     * @tparam  container   Container type.
     * @tparam  args        Extra container template arguments.
     * 
     * @brief   A templated struct for easy type casting.
     */
/*----------------------------------------------------------------------------*/
            PROPERTY_CONTAINER_TEMPLATE
        struct add_return<PROPERTY_CONTAINER>
        {
                //! Add function return type.
            typedef container<type*, args...> return_type; 
                //! Add function cast type.
            typedef PROPERTY_CONTAINER        cast_type;
                //! Add function allocation type.
            typedef PROPERTY_CONTAINER        add_type;    
        };

/*----------------------------------------------------------------------------*/
    /*!
     * @tparam  type    Type to be contained.
     * @tparam  size    Size of array.
     * 
     * @brief   A templated struct for easy type casting.
     */
/*----------------------------------------------------------------------------*/
            PROPERTY_ARRAY_TEMPLATE
        struct add_return<PROPERTY_ARRAY>
        {
                //! Add function return type.
            typedef std::array<type*, size> return_type; 
                //! Add function cast type.
            typedef PROPERTY_ARRAY        cast_type;
                //! Add function allocation type.
            typedef PROPERTY_ARRAY        add_type;
        };

    //! DEFINE for add() function return type.
#define PSL_RETURN(x) typename add_return<x>::return_type
    //! DEFINE for add() function cast type.
#define PSL_CAST(x) typename add_return<x>::cast_type
    //! DEFINE for add() function allocation type.
#define PSL_ADD(x, ...) typename add_return<x>::add_type(__VA_ARGS__)
    //! DEFINE shorthand add() code.
#define PSL_ADD_N_CAST(x, ...)\
    *add(new PSL_ADD(x, __VA_ARGS__))->cast<PSL_CAST(x)>()

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for allocating, constructing and adding a property. 
     * @param   arg_name        Name of the stored value in JSON.
     * @param   argcr_default   Default value for wrapped type.
     * @return                  Reference to the wrapped type.
     */
  /*--------------------------------------------------------------------------*/
            template<typename type>
        PSL_RETURN(type)& add(string arg_name, const type& argcr_default)
            { return PSL_ADD_N_CAST(type, arg_name, argcr_default); }

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for allocating, constructing and adding a property. 
     * @param   arg_name        Name of the stored value in JSON.
     * @return                  Reference to the wrapped type.
     */
  /*--------------------------------------------------------------------------*/
            template<typename type>
        PSL_RETURN(type)& add(string arg_name)
            { return PSL_ADD_N_CAST(type, arg_name); }

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for allocating, constructing and adding a properties 
     *          derived object. Clones the passed object and adds to the
     *          properties map.
     * @param   argcr_obj   Reference to derived type for cloning into map.
     * @return              Reference to the newly added derived type.
     */
  /*--------------------------------------------------------------------------*/
            template<typename type>
        type& add(const properties& argcr_obj)
        {
            return *dynamic_cast<type*>(add(argcr_obj.clone()));
        }

#undef PSL_RETURN       // UNDEFINE for add() function return type.
#undef PSL_CAST         // UNDEFINE for add() function cast type.
#undef PSL_ADD          // UNDEFINE for add() function allocation type.
#undef PSL_ADD_N_CAST   // UNDEFINE shorthand add() code.

/* Remove Functions ----------------------------------------------------------*/

  /*--------------------------------------------------------------------------*/
    /*!
     * @brief   Function for deallocating and removing a property derived type. 
     * @param   argcr_name  Name of the stored property in the map.
     */
  /*--------------------------------------------------------------------------*/
        void remove(string argcr_name);

    };
}
#endif