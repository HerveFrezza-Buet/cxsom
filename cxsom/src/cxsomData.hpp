#pragma once

#include <iostream>
#include <tuple>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <array>

namespace cxsom {
  namespace error {
    struct unknown_type : public std::logic_error {
      using std::logic_error::logic_error;
    };
    
    struct type_mismatch : public std::logic_error {
      using std::logic_error::logic_error;
    };
    
    struct untyped : public std::logic_error {
      using std::logic_error::logic_error;
    };
    
    struct type_parsing : public std::logic_error {
      using std::logic_error::logic_error;
    };
  }


  /* ######## */
  /* #      # */
  /* # Base # */
  /* #      # */
  /* ######## */

  namespace type {
    /**
     * This class is the base for the cxsom dynamical types (i.e. the
     * type of the data handled during computation).
     */
    class Base {
    public:
      Base() = default;
      Base(const Base&) = default;
      Base& operator=(const Base&) = default;
      Base(Base&&) = default;
      Base& operator=(Base&&) = default;
      virtual ~Base() {}

      /**
       * @returns a string (normalized) describing the type.
       */
      virtual std::string name()           const = 0;
      
      /**
       * @returns The size of a value of that type.
       */
      virtual std::size_t byte_length()    const = 0;
      
      /**
       * @returns true for the "Scalar" type, false otherwise.
       */
      virtual bool is_Scalar()                  const {return false;}
      
      /**
       * @returns true is the type is "Array=...", false otherwise.
       */
      virtual unsigned int is_Array()           const {return 0;}
      
      /**
       * @param arg : provide an integer for the array size.
       * @returns true is the type is "Array=arg", false otherwise.
       */
      virtual bool is_Array(unsigned int)       const {return false;}
      
      /**
       * @returns true is the type is "Pos1D", false otherwise.
       */
      virtual bool is_Pos1D()                   const {return false;}
      
      /**
       * @returns true is the type is "Pos2D", false otherwise.
       */
      virtual bool is_Pos2D()                   const {return false;}
      
      /**
       * @returns true is the type is "Map1D<...>=..." or "Map2D<...>=...", false otherwise.
       */
      virtual bool is_Map()                     const {return false;}
      
      /**
       * @param arg : provide the name (string) of the content type.
       * @returns true is the type is "Map1D<arg>=..." or "Map2D<arg>=...", false otherwise.
       */
      virtual bool is_Map(const std::string&)   const {return false;}
      
      /**
       * @returns true is the type is "Map1D<...>=...", false otherwise.
       */
      virtual bool is_Map1D()                   const {return false;}
      
      /**
       * @param arg : provide the name (string) of the content type.
       * @returns true is the type is "Map1D<arg>=...", false otherwise.
       */
      virtual bool is_Map1D(const std::string&) const {return false;}
      
      /**
       * @returns true is the type is "Map2D<...>=...", false otherwise.
       */
      virtual bool is_Map2D()                   const {return false;}
      
      /**
       * @param arg : provide the name (string) of the content type.
       * @returns true is the type is "Map2D<arg>=...", false otherwise.
       */
      virtual bool is_Map2D(const std::string&) const {return false;}
      
      /**
       * @returns 0 if the type is not "Map1D<Array=size>=..." nore "Map2D<Array=size>=...", the array size otherwise.
       */
      virtual unsigned int is_MapOfArray()      const {return 0;}     
      
      /**
       * @returns 0 if the type is not "Map1D<Array=size>=...", the array size otherwise.
       */
      virtual unsigned int is_Map1DOfArray()    const {return 0;} 
      
      /**
       * @returns 0 if the type is not "Map2D<Array=size>=...", the array size otherwise.
       */
      virtual unsigned int is_Map2DOfArray()    const {return 0;}     

      bool operator==(const Base& b) const {return name() == b.name();}
      bool operator!=(const Base& b) const {return name() != b.name();}
    };

    using ref = std::shared_ptr<const Base>;
  }

  namespace data {
    /**
     * This class is the bas for representing the content of a data
     * handled by the simulation.
     */
    class Base {
    public:
      type::ref type;
      Base() = delete;
      Base(type::ref type) : type(type) {}
      Base(const Base&) = default;
      Base& operator=(const Base&) = default;
      Base(Base&&) = default;
      Base& operator=(Base&&) = default;
      virtual ~Base() {}

      virtual char*       first_byte()       = 0;
      virtual const char* first_byte() const = 0;

      std::tuple<double*, double*> data_range() {return {reinterpret_cast<double*>(first_byte()), reinterpret_cast<double*>(first_byte() + type->byte_length())};}
      std::tuple<const double*, const double*> data_range() const {return {reinterpret_cast<const double*>(first_byte()), reinterpret_cast<const double*>(first_byte() + type->byte_length())};}

     
      virtual void write(std::ostream& os)   const {os.write(first_byte(), type->byte_length());}
      virtual void read (std::istream& is)         {is.read (first_byte(), type->byte_length());}
      virtual void write(char* buf)          const {auto begin = first_byte(); std::copy(begin, begin + type->byte_length(), buf);}
      virtual void read (const char* buf)          {std::copy(buf, buf + type->byte_length(), first_byte());}
      virtual bool is_equal(const char* buf) const {auto begin = first_byte(); return std::equal(begin, begin + type->byte_length(), buf);}
      virtual void print(std::ostream& os)   const = 0;
      virtual void operator=(double value)         = 0;
    };
      
    using ref = std::shared_ptr<Base>;
    inline std::ostream& operator<<(std::ostream& os, const Base& b) {
      b.print(os);
      return os;
    }
  }

  
  /* ########## */
  /* #        # */
  /* # Scalar # */
  /* #        # */
  /* ########## */
  
  namespace type {
    class Scalar : public Base {
    public:
      Scalar() = default;
      Scalar(const Scalar&) = default;
      Scalar& operator=(const Scalar&) = default;
      Scalar(Scalar&&) = default;
      Scalar& operator=(Scalar&&) = default;
      virtual ~Scalar() {}
	
      virtual std::string name()        const override {return "Scalar";}
      virtual std::size_t byte_length() const override {return sizeof(double);}
      virtual bool        is_Scalar()   const override {return true;}
    };
  }
  
  namespace data {
    class Scalar : public Base {
    public:
      double value = 0;
      Scalar() = delete;
      Scalar(type::ref type) : Base(type) {}
      Scalar(const Scalar&) = default;
      Scalar& operator=(const Scalar&) = default;
      Scalar(Scalar&&) = default;
      Scalar& operator=(Scalar&&) = default;
      virtual ~Scalar() {}
       
      virtual char*       first_byte()           override {return reinterpret_cast<char*>(&value);}
      virtual const char* first_byte()     const override {return reinterpret_cast<const char*>(&value);}
      virtual void print(std::ostream& os) const override {os << value;}
      virtual void operator=(double v)           override {value = v;}
    };
  }


  
  /* ########## */
  /* #        # */
  /* # Pos 1D # */
  /* #        # */
  /* ########## */
  
  namespace type {
    namespace d1 {
      class Pos : public Base {
      public:
	Pos() = default;
	Pos(const Pos&) = default;
	Pos& operator=(const Pos&) = default;
	Pos(Pos&&) = default;
	Pos& operator=(Pos&&) = default;
	virtual ~Pos() {}
	
	virtual std::string name()        const override {return "Pos1D";}
	virtual std::size_t byte_length() const override {return sizeof(double);}
	virtual bool        is_Pos1D()    const override {return true;}
      };
    }
  }
  
  namespace data {
    namespace d1 {
      class Pos : public Base {
      public:
	double x = 0;
	Pos() = delete;
	Pos(type::ref type) : Base(type) {}
	Pos(const Pos&) = default;
	Pos& operator=(const Pos&) = default;
	Pos(Pos&&) = default;
	Pos& operator=(Pos&&) = default;
	virtual ~Pos() {}
       
	virtual char*       first_byte()           override {return reinterpret_cast<char*>(&x);}
	virtual const char* first_byte()     const override {return reinterpret_cast<const char*>(&x);}
	virtual void print(std::ostream& os) const override {os << '(' << x << ')';}
	virtual void operator=(double v)           override {x = v;}
      };
    }
  }
  
  /* ########## */
  /* #        # */
  /* # Pos 2D # */
  /* #        # */
  /* ########## */
  
  namespace type {
    namespace d2 {
      class Pos : public Base {
      public:
	Pos() = default;
	Pos(const Pos&) = default;
	Pos& operator=(const Pos&) = default;
	Pos(Pos&&) = default;
	Pos& operator=(Pos&&) = default;
	virtual ~Pos() {}
	
	virtual std::string name()        const override {return "Pos2D";}
	virtual std::size_t byte_length() const override {return 2*sizeof(double);}
	virtual bool        is_Pos2D()    const override {return true;}
      };
    }
  }
  
  namespace data {
    namespace d2 {
      class Pos : public Base {
      public:
	std::array<double, 2> xy = {0, 0};
	Pos() = delete;
	Pos(type::ref type) : Base(type) {}
	Pos(const Pos&) = default;
	Pos& operator=(const Pos&) = default;
	Pos(Pos&&) = default;
	Pos& operator=(Pos&&) = default;
	virtual ~Pos() {}
       
	virtual char*       first_byte()           override {return reinterpret_cast<char*>(&xy);}
	virtual const char* first_byte()     const override {return reinterpret_cast<const char*>(&xy);}
	virtual void print(std::ostream& os) const override {os << '(' << xy[0] << ", " << xy[1] << ')';}
	virtual void operator=(double v)           override {xy[0] = v; xy[1] = v;}
      };
    }
  }

  /* ########## */
  /* #        # */
  /* # Arrays # */
  /* #        # */
  /* ########## */

  namespace type {
    class Array : public Base {
    public:
      unsigned int size;
      Array() = delete;
      Array(unsigned int size)
	: Base(), size(size) {}
      Array(const Array&) = default;
      Array& operator=(const Array&) = default;
      Array(Array&&) = default;
      Array& operator=(Array&&) = default;
      virtual ~Array() {}
      
      virtual unsigned int is_Array()           const override {return size;}
      virtual bool is_Array(unsigned int size)  const override {return size == this->size;}
      virtual std::string name()                const override {return std::string("Array=")+std::to_string(size);}
      virtual std::size_t byte_length()         const override {return sizeof(double)*size;}
    };
  }

  namespace data {
    
    class Array : public Base {
    public:
      std::vector<double> content;
	
      Array() = delete;
      Array(type::ref type) : Base(type), content(static_cast<const type::Array*>(type.get())->size, 0.) {}
      Array(const Array&) = default;
      Array& operator=(const Array&) = default;
      Array(Array&&) = default;
      Array& operator=(Array&&) = default;
      virtual ~Array() {}
	
      virtual char*       first_byte()           override {return reinterpret_cast<char*>(std::data(content));}
      virtual const char* first_byte()     const override {return reinterpret_cast<const char*>(std::data(content));}
      virtual void print(std::ostream& os) const override {
	os << '<';
	auto it = content.begin();
	if(it != content.end())    os << *(it++);
	while(it != content.end()) os << ", " << *(it++);
	os << '>';
      }
      virtual void operator=(double v)           override {for(auto& value : content) value = v;}
    };
  }
  

  /* ######## */
  /* #      # */
  /* # Maps # */
  /* #      # */
  /* ######## */

  namespace type {

    /**
     * This is the base for any map-based type.
     */
    class Map : public Base {
    public:
      unsigned int side;            //!< The side of the map (the length is 1D, the square side if 2D).
      unsigned int size;            //!< The number of elements in the map.
      unsigned int content_size;    //!< The number of doubles for one of the elements.
      unsigned int nb_of_doubles;   //!< The total number of doubles required to store the map elements.
      std::string  content_type;
	
      Map() = delete;
      Map(unsigned int side, unsigned int size, unsigned int content_size, const std::string& content_type)
	: Base(), side(side), size(size), content_size(content_size), content_type(content_type) {
	nb_of_doubles = size * content_size;
      }
      Map(const Map&) = default;
      Map& operator=(const Map&) = default;
      Map(Map&&) = default;
      Map& operator=(Map&&) = default;
      virtual ~Map() {}
      
      virtual bool is_Map() const override {return true;}
      virtual std::size_t  byte_length() const override {return sizeof(double)*size*content_size;}

    protected:
      
      std::string build_name(int dim) const {
	std::ostringstream ostr;
	ostr << "Map" << dim << "D<" << content_type << ">=" << side;
	return ostr.str();
      }
    };
    
    namespace d1 {
      template<typename CONTENT>
      class Map : public type::Map {};

      template<>
      class Map<Scalar> : public type::Map {
      public:
	
	Map() = delete;
	Map(unsigned int side) : type::Map(side, side, 1, "Scalar") {}
	Map(const Map&) = default;
	Map& operator=(const Map&) = default;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;
	virtual ~Map() {}
	
	virtual std::string name()                                    const override {return this->build_name(1);}
	virtual bool        is_Map1D()                                const override {return true;}
	virtual bool        is_Map1D(const std::string& content_type) const override {return content_type == "Scalar";}
	virtual bool        is_Map  (const std::string& content_type) const override {return content_type == "Scalar";}
      };

      template<>
      class Map<d1::Pos> : public type::Map {
      public:
	
	Map() = delete;
	Map(unsigned int side) : type::Map(side, side, 1, "Pos1D") {}
	Map(const Map&) = default;
	Map& operator=(const Map&) = default;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;
	virtual ~Map() {}
	
	virtual std::string name()                                    const override {return this->build_name(1);}
	virtual bool        is_Map1D()                                const override {return true;}
	virtual bool        is_Map1D(const std::string& content_type) const override {return content_type == "Pos1D";}
	virtual bool        is_Map  (const std::string& content_type) const override {return content_type == "Pos1D";}
      };

      template<>
      class Map<d2::Pos> : public type::Map {
      public:
	
	Map() = delete;
	Map(unsigned int side) : type::Map(side, side, 2, "Pos2D") {}
	Map(const Map&) = default;
	Map& operator=(const Map&) = default;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;
	virtual ~Map() {}
	
	virtual std::string name()                                    const override {return this->build_name(1);}
	virtual bool        is_Map1D()                                const override {return true;}
	virtual bool        is_Map1D(const std::string& content_type) const override {return content_type == "Pos2D";}
	virtual bool        is_Map  (const std::string& content_type) const override {return content_type == "Pos2D";}
      };

      template<>
      class Map<Array> : public type::Map {
      public:
	
	Map() = delete;
	Map(unsigned int side, unsigned int array_size) : type::Map(side, side, array_size, Array(array_size).name()) {}
	Map(const Map&) = default;
	Map& operator=(const Map&) = default;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;
	virtual ~Map() {}
	
	virtual std::string  name()                                    const override {return this->build_name(1);}
	virtual bool         is_Map1D()                                const override {return true;}
	virtual bool         is_Map1D(const std::string& content_type) const override {return content_type == Array(content_size).name();}
	virtual bool         is_Map  (const std::string& content_type) const override {return content_type == Array(content_size).name();}
	virtual unsigned int is_MapOfArray()                           const override {return content_size;}     
	virtual unsigned int is_Map1DOfArray()                         const override {return content_size;}     
      };
    }
    
    namespace d2 {
      template<typename CONTENT>
      class Map : public type::Map {};

      template<>
      class Map<Scalar> : public type::Map {
      public:
	
	Map() = delete;
	Map(unsigned int side) : type::Map(side, side*side, 1, "Scalar") {}
	Map(const Map&) = default;
	Map& operator=(const Map&) = default;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;
	virtual ~Map() {}
	
	virtual std::string name()                                    const override {return this->build_name(2);}
	virtual bool        is_Map2D()                                const override {return true;}
	virtual bool        is_Map2D(const std::string& content_type) const override {return content_type == "Scalar";}
	virtual bool        is_Map  (const std::string& content_type) const override {return content_type == "Scalar";}
      };

      template<>
      class Map<d1::Pos> : public type::Map {
      public:
	
	Map() = delete;
	Map(unsigned int side) : type::Map(side, side*side, 1, "Pos1D") {}
	Map(const Map&) = default;
	Map& operator=(const Map&) = default;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;
	virtual ~Map() {}
	
	virtual std::string name()                                    const override {return this->build_name(2);}
	virtual bool        is_Map2D()                                const override {return true;}
	virtual bool        is_Map2D(const std::string& content_type) const override {return content_type == "Pos1D";}
	virtual bool        is_Map  (const std::string& content_type) const override {return content_type == "Pos1D";}
      };

      template<>
      class Map<d2::Pos> : public type::Map {
      public:
	
	Map() = delete;
	Map(unsigned int side) : type::Map(side, side*side, 2, "Pos2D") {}
	Map(const Map&) = default;
	Map& operator=(const Map&) = default;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;
	virtual ~Map() {}
	
	virtual std::string name()                                    const override {return this->build_name(2);}
	virtual bool        is_Map2D()                                const override {return true;}
	virtual bool        is_Map2D(const std::string& content_type) const override {return content_type == "Pos2D";}
	virtual bool        is_Map  (const std::string& content_type) const override {return content_type == "Pos2D";}
      };

      template<>
      class Map<Array> : public type::Map {
      public:
	
	Map() = delete;
	Map(unsigned int side, unsigned int array_size) : type::Map(side, side * side, array_size, Array(array_size).name()) {}
	Map(const Map&) = default;
	Map& operator=(const Map&) = default;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;
	virtual ~Map() {}
	
	virtual std::string  name()                                    const override {return this->build_name(2);}
	virtual bool         is_Map2D()                                const override {return true;}
	virtual bool         is_Map2D(const std::string& content_type) const override {return content_type == Array(content_size).name();}
	virtual bool         is_Map  (const std::string& content_type) const override {return content_type == Array(content_size).name();}
	virtual unsigned int is_MapOfArray()                           const override {return content_size;}     
	virtual unsigned int is_Map2DOfArray()                         const override {return content_size;}     
      };
    }
  }
    

  namespace data {
    class Map : public Base {
    public:
      std::vector<double> content;
      unsigned int           side;
	
      Map() = delete;
      Map(type::ref type) : Base(type), content(static_cast<const type::Map*>(type.get())->nb_of_doubles, 0.), side(static_cast<const type::Map*>(type.get())->side) {}
      Map(const Map&) = default;
      Map& operator=(const Map&) = default;
      Map(Map&&) = default;
      Map& operator=(Map&&) = default;
      virtual ~Map() {}
	
      virtual char*       first_byte()           override {return reinterpret_cast<char*>(std::data(content));}
      virtual const char* first_byte()     const override {return reinterpret_cast<const char*>(std::data(content));}
      virtual void print(std::ostream& os) const override {
	os << '[' << side << ", " << content.size() << "] = (";
	if(content.size() <= 4) {
	  auto it = content.begin();
	  if    (it != content.end()) os << *(it++);
	  while (it != content.end()) os << ", " << *(it++);
	  os << ')';
	}
	else 
	  os << content[0] << ", " << content[1] << ", ..., "
	     << content[content.size()-2] << ", " << content[content.size()-1]  << ')';
      }
      virtual void operator=(double v)           override {for(auto& value : content) value = v;}
    };
  }

  namespace type {

    inline void parse_check(std::istream& is, const std::string& token) {
      for(auto c : token) 
	if(char cc = is.get(); cc != c)
	  throw error::type_parsing(token + " expected");
    }
    
    inline void parse_check(std::istream& is, char c) {
      if(char cc = is.get(); cc != c)
	throw error::type_parsing(std::string("char '") + std::string(1, c) + "' expected");
    }
    
    inline ref parse_Scalar(std::istream& is) {
      parse_check(is, "Scalar");
      return ref(new Scalar());
    }

    inline unsigned int parse_DIM(std::istream& is) {
      unsigned int d = 0;
      is >> d;
      if(d == 0)
	throw error::type_parsing("Integer (and >0) dimension expected");
      parse_check(is, 'D');
      return d;
    }

    inline ref parse_POS(std::istream& is) {
      parse_check(is, "Pos");
      switch(parse_DIM(is)) {
      case 1:
	return ref(new d1::Pos());
	break;
      case 2:
	return ref(new d2::Pos());
	break;
      default:
	throw error::type_parsing("Bad dim number");
	break;
      }
    }

    inline unsigned int parse_SIZE(std::istream& is) {
      parse_check(is, '=');
      unsigned int res = 0;
      is >> res;
      if(res == 0) 
	throw error::type_parsing("Integer (and >0) size expected");
      return res;
    }

    inline ref parse_ARRAY(std::istream& is) {
      parse_check(is, "Array");
      auto s = parse_SIZE(is);
      return ref(new Array(s));
    }
    
    inline ref parse_CONTENT(std::istream& is) {
      char c;
      std::string buf;
      is >> c;
      is.putback(c);
      switch(c) {
      case 'S':
	return parse_Scalar(is);
	break;
      case 'P':
	return parse_POS(is);
	break;
      case 'A':
	return parse_ARRAY(is);
	break;
      default:
	throw error::type_parsing("Bad content");
	break;
      }

      return {};
    }
    
    inline ref parse_MAP(std::istream& is) {
      parse_check(is, "Map");
      auto map_dim = parse_DIM(is);
      if(map_dim > 2)
	throw error::type_parsing("Bad dim number (use 1 our 2)");
      parse_check(is, '<');
      auto content_ref = parse_CONTENT(is);
      parse_check(is, '>');
      auto map_side = parse_SIZE(is);
      switch(map_dim) {
      case 1:
	if(content_ref->is_Scalar())
	  return ref(new d1::Map<Scalar>(map_side));
	if(content_ref->is_Pos1D())
	  return ref(new d1::Map<d1::Pos>(map_side));
	if(content_ref->is_Pos2D())
	  return ref(new d1::Map<d2::Pos>(map_side));
	if(auto array_size = content_ref->is_Array(); array_size != 0)
	  return ref(new d1::Map<Array>(map_side, array_size));
	throw error::type_parsing("Bad map content");
	break;
      case 2:
	if(content_ref->is_Scalar())
	  return ref(new d2::Map<Scalar>(map_side));
	if(content_ref->is_Pos1D())
	  return ref(new d2::Map<d1::Pos>(map_side));
	if(content_ref->is_Pos2D())
	  return ref(new d2::Map<d2::Pos>(map_side));
	if(auto array_size = content_ref->is_Array(); array_size != 0)
	  return ref(new d2::Map<Array>(map_side, array_size));
	throw error::type_parsing("Bad map content");
	break;
      default:
	throw error::type_parsing("Bad map dim number");
	break;
      }
    }
    
    inline ref parse_TYPE(std::istream& is) {
      char c;
      std::string buf;
      is >> c;
      is.putback(c);
      switch(c) {
      case 'S':
      case 'P':
      case 'A':
	return parse_CONTENT(is);
	break;
      case 'M':
	return parse_MAP(is);
	break;
      default:
	std::getline(is, buf);
	throw error::type_parsing(std::string("Bad type --> \"") + buf + "\" (incoming char is (char)" + std::to_string((unsigned int)c) + ").");
	break;
      }
      return {};
    }
    
    /**
     * This is the grammar defining the type strings. It is a
     * predictive grammar. 'xxx' means actual xxx characters. Other
     * are non-terminals. There ar no spaces in the grammar (if so,
     * they would have been denoted by ' ').
     * <int> means characters representing an unsigned integer (e.g 246).
     *
     *
     * TYPE    :=   CONTENT 
     *            | MAP
     *
     * CONTENT :=   'Scalar'
     *            | POS
     *            | ARRAY
     *
     * POS     := 'Pos' DIM
     *
     * ARRAY   := 'Array' SIZE
     *
     * MAP     := 'Map' DIM '<' CONTENT '>' SIZE
     *
     * SIZE    := '=' <int>
     *
     * DIM     :=   '1D' 
     *            | '2D'
     *
     */
    inline ref make(const std::string& type_name) {
      std::istringstream istr(type_name);
      return parse_TYPE(istr);
    }
  }

  namespace data {
    inline ref make(type::ref type) {
      if(!type) {
	std::ostringstream ostr;
	ostr << "cxsom::data::make : Cannot build any type from nullptr.";
	throw cxsom::error::unknown_type(ostr.str());
      }
      
      if(type->is_Scalar())     return ref(new Scalar(type));
      if(type->is_Pos1D())      return ref(new d1::Pos(type));
      if(type->is_Pos2D())      return ref(new d2::Pos(type));
      if(type->is_Array() != 0) return ref(new Array(type));
      if(type->is_Map())        return ref(new Map(type));
      
      std::ostringstream ostr;
      ostr << "cxsom::data::make : Cannot build any type from \"" << type->name() << "\".";
      throw cxsom::error::unknown_type(ostr.str());
    }
  }
}
