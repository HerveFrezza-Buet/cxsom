#pragma once

#include <ios>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <memory>
#include <queue>
#include <optional>
#include <limits>
#include <algorithm>

#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

#include <cxsomData.hpp>
#include <cxsomSymbols.hpp>

#include <filesystem>
namespace fs = std::filesystem;

#define cxsom_UINT_LENGTH                  8

#define cxsom_LENGTH_TYPE_IN_FILE          64
#define cxsom_LENGTH_CACHE_SIZE_IN_FILE    cxsom_UINT_LENGTH
#define cxsom_LENGTH_FILE_SIZE_IN_FILE     cxsom_UINT_LENGTH
#define cxsom_LENGTH_HIGHEST_TIME_IN_FILE  cxsom_UINT_LENGTH
#define cxsom_LENGTH_NEXT_FREE_POS_IN_FILE cxsom_UINT_LENGTH

#define cxsom_OFFSET_TYPE_IN_FILE          0
#define cxsom_OFFSET_CACHE_SIZE_IN_FILE    (cxsom_OFFSET_TYPE_IN_FILE          + cxsom_LENGTH_TYPE_IN_FILE         )
#define cxsom_OFFSET_FILE_SIZE_IN_FILE     (cxsom_OFFSET_CACHE_SIZE_IN_FILE    + cxsom_LENGTH_CACHE_SIZE_IN_FILE   )
#define cxsom_OFFSET_HIGHEST_TIME_IN_FILE  (cxsom_OFFSET_FILE_SIZE_IN_FILE     + cxsom_LENGTH_FILE_SIZE_IN_FILE    )
#define cxsom_OFFSET_NEXT_FREE_POS_IN_FILE (cxsom_OFFSET_HIGHEST_TIME_IN_FILE  + cxsom_LENGTH_HIGHEST_TIME_IN_FILE )
#define cxsom_OFFSET_HEADER_IN_FILE        (cxsom_OFFSET_NEXT_FREE_POS_IN_FILE + cxsom_LENGTH_NEXT_FREE_POS_IN_FILE)



//#define cxsomDEBUG_VARFILE
//#define cxsomDEBUG_VARIABLE

namespace cxsom {
  namespace error {
    struct file : public std::logic_error {
      using std::logic_error::logic_error;
    };
    
    struct unknown_variable : public std::logic_error {
      using std::logic_error::logic_error;
    };
  }
  
  namespace data {
    enum class Availability : char {
				    Busy  = 0, //!< The variable content is not determined yet.
				    Ready = 1  //!< The variable content is determined (forever).
    };
    
    inline std::ostream& operator<<(std::ostream& os, Availability a) {
      switch(a) {
      case Availability::Busy : os << "busy" ; break;
      case Availability::Ready: os << "ready"; break;
      }
      
      return os;
    }

    
    enum class FileAvailability : char {
					Busy      = 0, //!< The value is not determined yet, but the data instance lives in the file.
					Ready     = 1, //!< Value is determined (forever), the instance lives in the buffer.
					Forgotten = 2  //!< The instance is in the past, before the history start.
    };
    
    inline std::ostream& operator<<(std::ostream& os, FileAvailability a) {
      switch(a) {
      case FileAvailability::Busy :     os << "busy" ;     break;
      case FileAvailability::Ready:     os << "ready";     break;
      case FileAvailability::Forgotten: os << "forgotten"; break;
      }
      
      return os;
    }
    
    inline Availability availability_of(FileAvailability a) {
      if(a == FileAvailability::Busy) return Availability::Busy;
      return Availability::Ready;
    }

    class File {
    public:
      static std::size_t no_time() {return std::numeric_limits<std::size_t>::max();}
      static std::string string_of_time(std::size_t time) {
	if(time == no_time()) return "no_time";
	return std::to_string(time);
      }

      static std::size_t next_pos(std::size_t current_pos, std::size_t length) {
	std::size_t res = current_pos+1;
	if(res == length) res = 0;
	return res;
      }

      static bool is_past_in_buffer(std::size_t highest_time_minus_at, std::size_t length) {
	return highest_time_minus_at < length ;
      }

      // We suppose that is_past_in_buffer
      static std::size_t pos_in_past(std::size_t current_free_pos, std::size_t highest_time_minus_at, std::size_t length) {
	if(highest_time_minus_at < current_free_pos)
	  return current_free_pos - highest_time_minus_at - 1;
	return length + current_free_pos  - highest_time_minus_at - 1;
	  
      }
      
    private:

      class WithFile {
      private:
	std::fstream& file;
	bool was_closed;

      public:
	WithFile(std::fstream& file,
		 const fs::path& p) : file(file), was_closed(!(file.is_open())) {
	  if(was_closed) {    
	    file.clear();
	    file.open(p.c_str(), std::ios::out | std::ios::in);
	  }
	}
	
	~WithFile() {
	  if(was_closed) 
	    file.close();
	}
      };
	
      mutable std::fstream file;
      const symbol::Variable var_symb;
      type::ref type;
      std::size_t cache_size;
      std::size_t file_size;
      std::size_t data_size;
      std::size_t highest_time;
      std::size_t next_free_pos;
      fs::path var_path;
      bool realized;
      
      
      void read_uint(std::size_t& nb) {
	std::array<char,cxsom_UINT_LENGTH> buf;
	std::size_t byte;
	file.read(std::data(buf), cxsom_UINT_LENGTH);
	
	nb = 0;
	byte = (unsigned char)(buf[0]); nb |= byte;
	byte = (unsigned char)(buf[1]); nb |= (byte <<  8);
	byte = (unsigned char)(buf[2]); nb |= (byte << 16);
	byte = (unsigned char)(buf[3]); nb |= (byte << 24);
	byte = (unsigned char)(buf[4]); nb |= (byte << 32);
	byte = (unsigned char)(buf[5]); nb |= (byte << 40);
	byte = (unsigned char)(buf[6]); nb |= (byte << 48);
	byte = (unsigned char)(buf[7]); nb |= (byte << 56);
      }
      
      void write_uint(std::size_t val) {
	file.put((char)(val & 0xFF)); val >>= 8;
	file.put((char)(val & 0xFF)); val >>= 8;
	file.put((char)(val & 0xFF)); val >>= 8;
	file.put((char)(val & 0xFF)); val >>= 8;
	file.put((char)(val & 0xFF)); val >>= 8;
	file.put((char)(val & 0xFF)); val >>= 8;
	file.put((char)(val & 0xFF)); val >>= 8;
	file.put((char)(val & 0xFF));
      }

      bool readable(std::size_t at) {
	return (highest_time != no_time())
	  &&   (file_size != 0)
	  &&   (at <= highest_time)
	  &&   (at > highest_time - file_size);
      }
				    

      // This seeks at a position (not checked) in the buffer.
      void seekg(std::size_t at) {seekg_htma(highest_time - at);}
      
      // This seeks at a position (not checked) in the buffer.
      void seekg_htma(std::size_t highest_time_minus_at) {
	file.seekg(cxsom_OFFSET_HEADER_IN_FILE + pos_in_past(next_free_pos, highest_time_minus_at, file_size) * data_size);
      }
      
    public:

      File() = delete;
      File(const fs::path& root_path,
	   const symbol::Variable& var_symb)    
	: file(),
	  var_symb(var_symb),
	  var_path(root_path / var_symb.timeline / (var_symb.name + ".var")),
	  realized(false) {
	file.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
	auto d = var_path;
	d.remove_filename();
	if(!fs::exists(d)) fs::create_directories(d);
#ifdef cxsomDEBUG_VARFILE
	std::cout << "[varfile " << var_path << "] Definition." << std::endl;
#endif
      }

      /**
       * Tells wether the file is already there.
       */
      bool exists() const {
	return fs::exists(var_path);
      }

      std::string type_as_string() const {
	if(type)
	  return type->name();
	else
	  return "none";
      }

      type::ref get_type() const {return type;}

      std::size_t get_next_time() const {
	if(highest_time == no_time())
	  return 0;
	return highest_time + 1;
      }

      /**
       * Returns the legnth of the circular buffer.
       */
      std::size_t get_file_size() const {return file_size;}
      
      /**
       * Returns the legnth of the cache as stored in the file.
       */
      std::size_t get_cache_size() const {return cache_size;}

      /**
       * Returns (first, last), the time steps stored in the file (last included). It may return (no_time, no_time) if the file is empty.
       */
      std::pair<std::size_t, std::size_t> get_time_range() const {
	if(highest_time == no_time()) return {no_time(),    no_time()   };
	if(file_size == 0)            return {highest_time, highest_time};
	if(highest_time < file_size)  return {0,            highest_time};
	return {highest_time - file_size + 1, highest_time};
      }

      
      
      
      /**
       * Tells wether the file is realized.
       */
      operator bool() const {return realized;}

      /**
       * This tells which variable is handled.
       */
      operator symbol::Variable() const {return var_symb;}

      /**
       * Checks the file where the variable is supposed to be
       * stored. It creates the file if it does not exist yet, it
       * retrieves information from it if it is already there. Without
       * this call at least once, the File object is not related to
       * any file on the disk.
       *
       * Realization is also required if somebody else has changed the file (data added mainly).
       *
       * @param type If not nullptr, it defines the type of the variable. If not nullprt, it should fit the existing type in the file when the file already exists.
       * @param cache_size The size of the data instance cache. It is updated in the file if not std::nullopt.
       * @param file_size The size of the circular buffer stored in the file. It is updated read if the file exists, so a non std::nullopt in this case is ignored.
       * @param kept_opened If true, the file is kept opened for further use. Be sure you do not have too many (> 1024) files kept opened during your simulation, Linux limits this.
       */
      void realize(type::ref type,
		   std::optional<std::size_t> cache_size,
		   std::optional<std::size_t> file_size,
		   bool kept_opened) {
	if(fs::exists(var_path)) {
	  WithFile with_file(file, var_path);

	  // Checking type
	  std::string type_name;
	  file.seekg(0, std::ios_base::beg);
	  file >> type_name;
	  this->type = type::make(type_name);
	  if(type && (*(this->type) != *type)) {
	    std::ostringstream ostr;
	    ostr << "cxsom::data::File::check : "
		 << "Checking for " << type->name() << " while " << var_path << " contains " << this->type->name() << ".";
	    throw error::type_mismatch(ostr.str());
	  }

	  // Setting/getting cache_size
	  if(cache_size) {
	    this->cache_size = *cache_size;
	    file.seekp(cxsom_OFFSET_CACHE_SIZE_IN_FILE, std::ios_base::beg);
	    write_uint(this->cache_size);
	  }
	  else {
	    file.seekg(cxsom_OFFSET_CACHE_SIZE_IN_FILE, std::ios_base::beg);
	    read_uint(this->cache_size);
	  }


	  // Getting file_size
	  file.seekg(cxsom_OFFSET_FILE_SIZE_IN_FILE, std::ios_base::beg);
	  read_uint(this->file_size);


	  
	  // Getting highest_time
	  file.seekg(cxsom_OFFSET_HIGHEST_TIME_IN_FILE, std::ios_base::beg);
	  read_uint(this->highest_time);


	  // Getting next_free_pos
	  file.seekg(cxsom_OFFSET_NEXT_FREE_POS_IN_FILE, std::ios_base::beg);
	  read_uint(this->next_free_pos);

#ifdef cxsomDEBUG_VARFILE
	  std::cout << "[varfile " << var_path << "] Realize from existing : "
		    << "cache_size=" << this->cache_size << ", "
		    << "file_size=" << this->file_size << ", "
		    << "highest_time=" << string_of_time(this->highest_time) << ", "
		    << "next_free_pos=" << this->next_free_pos << std::endl;
#endif
	}
	else {
	  if(file.is_open()) // May never happen...
	    file.close();
	  
	  if(!type) {
	    std::ostringstream msg;
	    msg << "cxsom::data::File::ckeck: Creating file for " << var_symb << " from scratch requires a type.";
	    throw error::file(msg.str());
	  }
	  if(!cache_size) {
	    std::ostringstream msg;
	    msg << "cxsom::data::File::ckeck: Creating file for " << var_symb << " from scratch requires a cache size.";
	    throw error::file(msg.str());
	  }
	  if(!file_size) {
	    std::ostringstream msg;
	    msg << "cxsom::data::File::ckeck: Creating file for " << var_symb << " from scratch requires a file size.";
	    throw error::file(msg.str());
	  }
	  
	  file.clear();
	  file.open(var_path, std::ios::out); // Creates the file.
	  
	  // We write the type
	  this->type = type;
	  file.seekp(0, std::ios_base::beg);
	  for(std::size_t i = 0; i < cxsom_LENGTH_TYPE_IN_FILE; ++i)
	    file.put(char(0));
	  file.seekp(0, std::ios_base::beg);
	  file << type->name() << '\n' << std::flush;
	  
	  file.seekp(cxsom_OFFSET_CACHE_SIZE_IN_FILE, std::ios_base::beg);
	  
	  this->cache_size = *cache_size;
	  write_uint(this->cache_size);
	  
	  this->file_size = *file_size;
	  write_uint(this->file_size);

	  highest_time = no_time();
	  write_uint(highest_time); 

	  next_free_pos = 0;
	  write_uint(next_free_pos);
	  file.close();
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "[varfile " << var_path << "] Realize from scratch : "
		    << "cache_size=" << this->cache_size << ", "
		    << "file_size=" << this->file_size << ", "
		    << "highest_time=" << string_of_time(this->highest_time) << ", "
		    << "next_free_pos=" << this->next_free_pos << std::endl;
#endif
	}
	
	if(kept_opened) {
	  if(!file.is_open()) {
	    file.clear();
	    file.open(var_path.c_str());
	  }
	}
	else if(file.is_open())
	  file.close(); 
	
	data_size = sizeof(bool) + this->type->byte_length();
	  
	realized = true;
      }

      /**
       * Returns data instance availability before the writing. If the instance is in the buffer (i.e. not forgotten), it is ready after writing.
       */
      FileAvailability write(std::size_t at, data::ref d) {
#ifdef cxsomDEBUG_VARFILE
	std::cout << "[varfile " << var_path << "] writing @" << at << ", datasize = " << data_size << " : " << std::endl;
#endif
	if(!realized) {
	  std::ostringstream ostr;
	  ostr << "Writing " << symbol::Instance(var_symb, at) << " while the file manager for " << var_symb << " is not realized yet." << std::endl;
	  throw error::file(ostr.str());
	}

	if(file_size == 0) {
	  if(highest_time == no_time() || at > highest_time) {
	    WithFile with_file(file, var_path);
	    highest_time = at;
	    file.seekp(cxsom_OFFSET_HIGHEST_TIME_IN_FILE, std::ios_base::beg);  write_uint(highest_time);
	    file << std::flush;
#ifdef cxsomDEBUG_VARFILE
	    std::cout << "[varfile " << var_path << "]@" << at << "    forgotten since file_size=0, highest time updated to " << highest_time << '.' << std::endl;
#endif
	  }
	  else {
#ifdef cxsomDEBUG_VARFILE
	    std::cout << "[varfile " << var_path << "]@" << at << "    forgotten since file_size=0, no highest time update." << std::endl;
#endif
	  }
	  
	  return FileAvailability::Forgotten;
	}

	if(highest_time == no_time()) {
	  // This is the first time something is written in the buffer.
    
	  WithFile with_file(file, var_path);
	  file.seekp(cxsom_OFFSET_HEADER_IN_FILE, std::ios_base::beg);

	  if(at >= file_size) {
	    // We have to allocate the full buffer length.
	    std::size_t nb_bytes = file_size * data_size;
	    for(std::size_t p = 0; p < nb_bytes; ++p) 
	      file.put(0);
	    
	    file.seekp(cxsom_OFFSET_HEADER_IN_FILE, std::ios_base::beg); // We go back to 0
	    next_free_pos = next_pos(0, file_size);
	  }
	  else {
	    // We clear pos [0, ... a-1], i.e. a slots.
	    std::size_t nb_bytes = at * data_size;
	    for(std::size_t p = 0; p < nb_bytes; ++p) 
	      file.put(0);
	    
	    // we are at slot a.
	    next_free_pos = next_pos(at, file_size);
	  }

	  auto slot_pos = file.tellp();
	    
	  // We write the content before marking it as ready, so that
	  // file readers cannot consider a file chunk readable while
	  // it is currently beeing filled.
	  file.put(static_cast<char>(Availability::Busy));
	  d->write(file);
	  file << std::flush;
	  
#ifdef cxsomDEBUG_VARFILE
	  auto end_write_pos = file.tellp();
#endif
	    
	  file.seekp(slot_pos, std::ios_base::beg);
	  file.put(static_cast<char>(Availability::Ready));

	  highest_time = at;
	  file.seekp(cxsom_OFFSET_HIGHEST_TIME_IN_FILE, std::ios_base::beg);  write_uint(highest_time);
	  file.seekp(cxsom_OFFSET_NEXT_FREE_POS_IN_FILE, std::ios_base::beg); write_uint(next_free_pos);
	  file << std::flush;
	  
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "[varfile " << var_path << "]@" << at << "    was busy (written to an empty file): " 
		    << "cache_size=" << this->cache_size << ", "
		    << "file_size=" << this->file_size << ", "
		    << "highest_time=" << string_of_time(this->highest_time) << ", "
		    << "next_free_pos=" << this->next_free_pos << ", "
		    << "write_range=[" << slot_pos << ".." << end_write_pos << ']' << std::endl;
#endif

	  return FileAvailability::Busy;
	}

	if(at <= highest_time) {
	  // We write in the past
	  std::size_t highest_time_minus_at = highest_time - at;
	  if(is_past_in_buffer(highest_time_minus_at, file_size)) {
	    WithFile with_file(file, var_path);
	    seekg_htma(highest_time_minus_at);
	    std::size_t file_pos = file.tellp();
	    if(file.get() != 0) {
#ifdef cxsomDEBUG_VARFILE
	      std::cout << "[varfile " << var_path << "]@" << at << "    was ready (no write here) : " 
			<< "cache_size=" << this->cache_size << ", "
			<< "file_size=" << this->file_size << ", "
			<< "highest_time=" << string_of_time(this->highest_time) << ", "
			<< "next_free_pos=" << this->next_free_pos << std::endl;
#endif
	      return FileAvailability::Ready;
	    }
	    
	    // We write the content before marking it as ready, so that
	    // file readers cannot consider a file chunk readable while
	    // it is currently beeing filled.
	    file.seekp(file_pos + std::streamoff(1), std::ios_base::beg); 
	    d->write(file);
	    file << std::flush;
	  
#ifdef cxsomDEBUG_VARFILE
	    auto end_write_pos = file.tellp();
#endif
	    
	    file.seekp(file_pos, std::ios_base::beg);
	    file.put(static_cast<char>(Availability::Ready));
	    file << std::flush;
#ifdef cxsomDEBUG_VARFILE
	    std::cout << "[varfile " << var_path << "]    was busy (written in the past) : " 
		      << "cache_size=" << this->cache_size << ", "
		      << "file_size=" << this->file_size << ", "
		      << "highest_time=" << string_of_time(this->highest_time) << ", "
		      << "next_free_pos=" << this->next_free_pos << ", "
		      << "write_range=[" << file_pos << ".." << end_write_pos << ']' << std::endl;
#endif
	    return FileAvailability::Busy;
	  }
	  else {
#ifdef cxsomDEBUG_VARFILE
	    std::cout << "[varfile " << var_path << "]    forgotten (written would have been in far past) : " 
		      << "cache_size=" << this->cache_size << ", "
		      << "file_size=" << this->file_size << ", "
		      << "highest_time=" << string_of_time(this->highest_time) << ", "
		      << "next_free_pos=" << this->next_free_pos << std::endl;
#endif
	    return FileAvailability::Forgotten;
	  }
	}

	// We write in the future.
	//
	///////////////

	WithFile with_file(file, var_path);

	// We have to clear the values in the buffer from
	// the last one until at-1.
	std::size_t nb_zeros = at - 1 - highest_time;

#ifdef cxsomDEBUG_VARFILE
	std::cout << "[varfile " << var_path << "]    Nota : " << nb_zeros
		  << " = " << at << " - 1 - " << highest_time << std::endl;
#endif

	if(nb_zeros > file_size) {
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "[varfile " << var_path << "]     Nota : clear all" << std::endl;
#endif

	  // We have to clear the whole buffer.
	  std::size_t nb_bytes = file_size * data_size;
	  file.seekp(cxsom_OFFSET_HEADER_IN_FILE, std::ios_base::beg);
	  for(std::size_t p = 0; p < nb_bytes; ++p) file.put(0);
	  
	  file.seekp(cxsom_OFFSET_HEADER_IN_FILE, std::ios_base::beg); // We go back to 0
	  next_free_pos = next_pos(0, file_size);
	}
	else {
	  // We have to clear the buffer partially.
	  // Next data pos is data_pos = next_free_pos + nb_zeros (modulo file_size).
	  // We clear [next_free_pos, min(file_size, data_pos)[
	  std::size_t data_pos      = next_free_pos + nb_zeros;
	  std::size_t upper_bound   = std::min(file_size, data_pos);
	  std::size_t nb_to_the_end = upper_bound - next_free_pos;
	  std::size_t nb_bytes      = nb_to_the_end * data_size;
	  file.seekp(cxsom_OFFSET_HEADER_IN_FILE + next_free_pos * data_size, std::ios_base::beg);
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "[varfile " << var_path << "]    Nota : clear " << nb_bytes
		    << " bytes first from " << file.tellp() << std::endl;
#endif
	  for(std::size_t p = 0; p < nb_bytes; ++p) file.put(0);
	  
	  // The file is ready to write the data... except if the writing is out of bounds.
	  // This is what we check.
	  
	  if(data_pos >= file_size) {
	    data_pos -= file_size;
	    file.seekp(cxsom_OFFSET_HEADER_IN_FILE, std::ios_base::beg); // We go back to 0
	    // we have to write data_pos zeroes.
	    std::size_t nb_bytes = data_pos * data_size;
#ifdef cxsomDEBUG_VARFILE
	    std::cout << "[varfile " << var_path << "]     Nota : clear "
		      << nb_bytes << " bytes second from " << file.tellp() << std::endl;
#endif
	    for(std::size_t p = 0; p < nb_bytes; ++p) file.put(0);
	  }

	  // The file is ready to writhe the data
	  next_free_pos = next_pos(data_pos, file_size);
	}

	auto slot_pos = file.tellp();

	    
	// We write the content before marking it as ready, so that
	// file readers cannot consider a file chunk readable while
	// it is currently beeing filled.
	file.put(static_cast<char>(Availability::Busy));
	d->write(file);
	file << std::flush;
	
#ifdef cxsomDEBUG_VARFILE
	auto end_write_pos = file.tellp();
#endif
	    
	file.seekp(slot_pos, std::ios_base::beg);
	file.put(static_cast<char>(Availability::Ready));
	    
	highest_time  = at;
	file.seekp(cxsom_OFFSET_HIGHEST_TIME_IN_FILE, std::ios_base::beg);  write_uint(highest_time);
	file.seekp(cxsom_OFFSET_NEXT_FREE_POS_IN_FILE, std::ios_base::beg); write_uint(next_free_pos);
	file << std::flush;
	
#ifdef cxsomDEBUG_VARFILE
	std::cout << "[varfile " << var_path << "]@" << at << "    was busy (written in the future) : " 
		  << "cache_size=" << this->cache_size << ", "
		  << "file_size=" << this->file_size << ", "
		  << "highest_time=" << string_of_time(this->highest_time) << ", "
		  << "next_free_pos=" << this->next_free_pos << ", "
		  << "write_range=[" << slot_pos << ".." << end_write_pos << ']' << std::endl;
#endif
	return FileAvailability::Busy;
      }

      /**
       * Returns data instance availability of the data. d is set only in case of availability being 'Ready'.
       */
      FileAvailability read(std::size_t at, data::ref d) {
#ifdef cxsomDEBUG_VARFILE
	std::cout << "[varfile " << var_path << "] reading @" << at << ": " << std::flush;
#endif
	if(!realized) {
	  std::ostringstream ostr;
	  ostr << "Writing " << symbol::Instance(var_symb, at) << " while the file manager for " << var_symb << " is not realized yet." << std::endl;
	  throw error::file(ostr.str());
	}

	if(file_size == 0) {
	  if(highest_time == no_time() || at > highest_time) {
#ifdef cxsomDEBUG_VARFILE
	    std::cout << "is busy (file_size = 0, but we read in the future)." << std::endl;
#endif
	    return FileAvailability::Busy;
	  }
	  
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "is forgotten (file_size = 0, we read in the past)." << std::endl;
#endif
	  return FileAvailability::Forgotten;
	}

	if(highest_time == no_time()) {
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "is busy (empty file)." << std::endl;
#endif
	  return FileAvailability::Busy;
	}

	if(at > highest_time) {
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "is busy (read in the future)." << std::endl;
#endif
	  return FileAvailability::Busy;
	}

	// We read from the past
	std::size_t highest_time_minus_at = highest_time - at;
	if(is_past_in_buffer(highest_time_minus_at, file_size)) {
	  WithFile with_file(file, var_path);
	  seekg_htma(highest_time_minus_at);
	  if(file.get() == 0) {
#ifdef cxsomDEBUG_VARFILE
	    std::cout << "is busy (in file but not initialized)." << std::endl;
#endif
	    return FileAvailability::Busy;
	  }
	  d->read(file);
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "is ready (in file, already set)." << std::endl;
#endif
	  return FileAvailability::Ready;
	}
	else {
#ifdef cxsomDEBUG_VARFILE
	  std::cout << "is forgotten (in far past)." << std::endl;
#endif
	  return FileAvailability::Forgotten;
	}
      }

      /**
       * This update memory info (highest_time, next_free_pos, ...)
       * from the file. This is used if the file has been modified by
       * somebody else.
       */
      void sync() {
#ifdef cxsomDEBUG_VARFILE
	std::cout << "[varfile " << var_path << "] sync." << std::endl;
#endif
	if(!realized)
	  throw error::file("Syncing unrealized file.");
	
	WithFile with_file(file, var_path);
	// Getting cache_size
	file.seekg(cxsom_OFFSET_CACHE_SIZE_IN_FILE, std::ios_base::beg);
	read_uint(this->cache_size);
	// Getting file_size
	file.seekg(cxsom_OFFSET_FILE_SIZE_IN_FILE, std::ios_base::beg);
	read_uint(this->file_size);
	// Getting highest_time
	file.seekg(cxsom_OFFSET_HIGHEST_TIME_IN_FILE, std::ios_base::beg);
	read_uint(this->highest_time);
	// Getting next_free_pos
	file.seekg(cxsom_OFFSET_NEXT_FREE_POS_IN_FILE, std::ios_base::beg);
	read_uint(this->next_free_pos);
      }




      

#define cxsom_MAX_CONTENT_SIZE 20
      void pretty_print(std::ostream& os,
			unsigned int tl_width,
			unsigned int name_width,
			unsigned int type_width,
			unsigned int file_size_width,
			unsigned int cache_size_width,
			unsigned int inf_time_width,
			unsigned int sup_time_width) {
	os << "\e[1m" // bold
	   << "\e[94m" // light blue
	   << std::setw(tl_width) << std::left << var_symb.timeline << "\e[0m"
	   << "\e[94m" // light blue
	   << ' ' << std::setw(name_width) << std::right << var_symb.name << ": \e[0m"
	   << "\e[95m" // light magenta
	   << std::setw(type_width) << std::right << type_as_string() << ": \e[0m"
	   << "\e[95m\e[1m" // light magenta, bold
	   << std::setw(file_size_width) << std::right << file_size << " \e[0m"
	   << "\e[95m(" << std::setw(cache_size_width) << std::right << cache_size << "): \e[0m"
	   << "\e[90m"; // Dark gray

	auto [inf, sup] = get_time_range();
	if(sup == no_time())
	  os << std::setw(inf_time_width + sup_time_width + 4) << std::right << "[]";
	else if(sup == inf) {
	  std::string strsup = std::to_string(sup);
	  unsigned int nb_ws = inf_time_width + sup_time_width + 4 - 2 - strsup.size();
	  os << std::setw(nb_ws) << ' ';
	  os << "[\e[30m\e[1m" << sup << "\e[0m\e[90m]";
	}
	else
	  os << '['
	     << std::setw(inf_time_width) << std::right << inf << ", \e[30m\e[1m"
	     << std::setw(sup_time_width) << std::right << sup
	     << "\e[0m\e[90m]";
	os << ": \e[0m";

	
	os << "\e[90m"; // Dark gray
	WithFile with_file(file, var_path);
	if(file_size > 0 && inf != no_time()) {
	  if(sup - inf + 1 > cxsom_MAX_CONTENT_SIZE) {
	    os << "...";
	    inf = sup - cxsom_MAX_CONTENT_SIZE + 4;
	  }
	  for(auto at = inf; at <= sup; ++at) {
	    seekg(at);
	    if(file.get() != 0)
	      os << "\u2714";
	    else
	      os << "\u25E6";
	  }
	}
	os << "\e[0m";
	

	os << std::endl;
      }

    };




    
    class Variable;

    /**
     * An instance is a variable, in a timeline, at a time step. It
     * can be read simultaneously by many readers, while writing into
     * it is thread safe. The instance is accessible via a shared
     * pointer, it is thus kept alive as long as some process holds a
     * reference on it.
     */
    class Instance;
    using instance_ref  = std::shared_ptr<Instance>;
    using instance_wref = std::weak_ptr<Instance>;
    class Instance {
    private:

      mutable std::atomic<unsigned int> nb_readers         = 0;
      mutable std::atomic<bool>         writing            = false;
      mutable std::mutex                read_mutex;
      mutable std::condition_variable   nobody_reads;
      mutable std::mutex                write_mutex;
      mutable std::condition_variable   no_writing_in_progress;
      
      Availability status      = Availability::Busy;
      std::size_t at           = 0;
      std::size_t datation     = 0;
      ref          content     = nullptr;
      Variable*    owner       = nullptr;

      friend class Variable;

      Instance(Availability status, std::size_t at, data::ref content, Variable* owner);
      Instance()                           = default;
      Instance(const Instance&)            = default;
      Instance& operator=(const Instance&) = default;
      
    public:

      ~Instance();
      
      template<typename SYNC_FUNC>
      void sync(const SYNC_FUNC& sync);

      template<typename READING_FUNC>
      void get(const READING_FUNC& read) const;
      
      template<typename WRITING_FUNC>
      void set(const WRITING_FUNC& write);

      operator Availability() const {return status;}
      
    };


    class Variable  {
    private:

      friend class Instance;
      File file;
      std::mutex mutex;
      
      // This enables to retreive the instances if they have already
      // been extracted from the file, without reading the file (i.e. a
      // kind of cache).
      std::map<std::size_t, instance_ref> cached_instances;
      

      // Tries to remove the nb oldest unused instance in the
      // cache. It returns how many instances have actually been
      // removed.
      std::size_t remove_oldest(std::size_t nb) {
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[var " << (symbol::Variable)file << "] : remove_oldest >>> attempting to flush " << nb << " instances from the cache." << std::endl;
#endif
	std::vector<std::map<std::size_t, instance_ref>::iterator> to_remove;
	auto out_to_remove = std::back_inserter(to_remove);
	for(auto it = cached_instances.begin(); it != cached_instances.end() && to_remove.size() < nb; ++it)
	  if(it->second.use_count() == 1) *(out_to_remove++) = it;
	for(auto it : to_remove) cached_instances.erase(it);
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[var " << (symbol::Variable)file << "] : remove_oldest <<< " << to_remove.size() << " actually flushed." << std::endl;
#endif
	return to_remove.size();
      }
      
      instance_ref instance(std::size_t at) {
	if(auto iter = cached_instances.find(at); iter != cached_instances.end()) {
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[var " << (symbol::Variable)file << "] retrieveing cached instance @" << at
		  << " (availability = " << iter->second->status << ")." << std::endl;
#endif
	  return iter->second;
	}
	
	// If we have not returned, we have to build the instance.
	
	auto d         = data::make(file.get_type());
	Availability s = availability_of(file.read(at, d));
	auto res = std::shared_ptr<Instance>(new Instance(s, at, d, this));
	std::size_t bound = std::max(file.get_cache_size(), std::size_t(1));
	auto next_size = cached_instances.size() + 1;
	if(next_size > bound) remove_oldest(next_size - bound);
	cached_instances.try_emplace(at, res);
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[var " << (symbol::Variable)file << "] making instance @" << at << " (availability = " << s << ")." << std::endl;
#endif
	return res;  
      }

       Availability sync(std::size_t at, data::ref d) {
	std::lock_guard<std::mutex> lock(mutex);
	file.sync();
	auto res = file.read(at, d);
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[var " << (symbol::Variable)file << "] @" << at << ": sync (read) : got " << res << '.' << std::endl;
#endif
	return (Availability)res;
      }

      
      void declare_ready(std::size_t at, data::ref d) {
	std::lock_guard<std::mutex> lock(mutex);
	file.write(at, d);
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[var " << (symbol::Variable)file << "] @" << at << ": declare ready (write)." << std::endl;
#endif
      }
      
    public:
      Variable()                           = delete;
      Variable(const Variable&)            = delete;
      Variable(Variable&&)                 = delete;
      Variable& operator=(const Variable&) = delete;
      Variable& operator=(Variable&&)      = delete;

      Variable(const fs::path& root_path,
	       const symbol::Variable& var_symb,
	       type::ref type,
	       std::optional<std::size_t> cache_size,
	       std::optional<std::size_t> file_size,
	       bool kept_opened)
	: file(root_path, var_symb), mutex(), cached_instances() {
	file.realize(type, cache_size, file_size, kept_opened);
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[var " << (symbol::Variable)file << "] created." << std::endl;
#endif
      }

      std::size_t history_length() {
	std::lock_guard<std::mutex> lock(mutex);
	file.sync();
	return file.get_next_time();
      }

      void clean_up() {
	std::lock_guard<std::mutex> lock(mutex);
	remove_oldest(cached_instances.size());
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[var " << (symbol::Variable)file << "] cleaned up." << std::endl;
#endif
      }

      instance_ref operator[](std::size_t at) {
	std::lock_guard<std::mutex> lock(mutex);
	return instance(at);
      }

      auto get_type() const {return file.get_type();}
      
    };

    

      
    inline Instance::Instance(Availability status, std::size_t at, data::ref content, Variable* owner)
      : status(status), at(at), datation(0), content(content), owner(owner) {
#ifdef cxsomDEBUG_VARIABLE
      std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") : created with status " << status << std::endl;
#endif
    }
    
    inline Instance::~Instance() {
#ifdef cxsomDEBUG_VARIABLE
      std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") : deleted (status " << status << ')' << std::endl;
#endif
    }

    
    template<typename SYNC_FUNC>
    void Instance::sync(const SYNC_FUNC& sync) {
      {
	std::unique_lock<std::mutex> lock(write_mutex);
	while(writing) no_writing_in_progress.wait(lock);
	++nb_readers;
      }

  
      status = owner->sync(at, content);
      sync(status);
	
      {
	std::unique_lock<std::mutex> lock(read_mutex);
	if(--nb_readers == 0) nobody_reads.notify_all();
      }
    }
    
      
    template<typename READING_FUNC>
    void Instance::get(const READING_FUNC& read) const {
      {
	std::unique_lock<std::mutex> lock(write_mutex);
	while(writing) no_writing_in_progress.wait(lock);
	++nb_readers;
      }

#ifdef cxsomDEBUG_VARIABLE
      std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") get >>> status was " << status << std::endl;
#endif
      const auto& d = *content; read(status, datation, d);
#ifdef cxsomDEBUG_VARIABLE
      std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") get <<< status is  " << status << std::endl;
#endif	
      {
	std::unique_lock<std::mutex> lock(read_mutex);
	if(--nb_readers == 0) nobody_reads.notify_all();
      }
    }
    
    template<typename WRITING_FUNC>
    void Instance::set(const WRITING_FUNC& write) {
      writing = true;
      {
	std::unique_lock<std::mutex> lock(read_mutex);
	while(nb_readers != 0) nobody_reads.wait(lock);
      }
	
#ifdef cxsomDEBUG_VARIABLE
      std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") set >>> status was " << status << std::endl;
#endif
      auto prev_status = status;
      auto& d = *content; write(status, datation, d);
      if(prev_status == Availability::Busy && status == Availability::Ready) {
#ifdef cxsomDEBUG_VARIABLE
	std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") set : data setting says 'ready'." << std::endl;
#endif
	owner->declare_ready(at, content);
      }
#ifdef cxsomDEBUG_VARIABLE
      else if(prev_status == Availability::Ready)
	std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") set : We are already 'ready', no computation involved." << std::endl;
      else
	std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") set : data setting says 'busy'." << std::endl;
      std::cout << "[instance " << (symbol::Variable)(owner->file) << "]@" << at << " (" << this << ") set <<< status is " << status << std::endl;
#endif
	
      {
	std::unique_lock<std::mutex> lock(write_mutex);
	writing = false;
	no_writing_in_progress.notify_all();
      }
    }     
    
    class Center {
    private:
      std::mutex mutex;
      fs::path root_dir;
      std::map<symbol::Variable, Variable> variables; 

      fs::path var_path(const symbol::Variable& var_symb) {
	return root_dir / var_symb.timeline / (var_symb.name + ".var");
      }
      
      void touch_var(const symbol::Variable& var_symb) {
	if(auto it = variables.find(var_symb); it == variables.end()) {
	  auto p = var_path(var_symb);
	  if(!fs::exists(p))  {
	    std::ostringstream ostr;
	    ostr << "cxsom::data::Center::touch_var(" << var_symb << "): variable doesn't exist yet.";
	    throw error::unknown_variable(ostr.str());
	  }
	  variables.try_emplace(var_symb,
				root_dir, var_symb,
				nullptr, std::nullopt, std::nullopt, false);
	}
      }

      Variable& get_var(const symbol::Variable& var_symb) {
	if(auto it = variables.find(var_symb); it == variables.end()) {
	  auto p = var_path(var_symb);
	  if(!fs::exists(p))  {
	    std::ostringstream ostr;
	    ostr << "cxsom::data::Center::get_var(" << var_symb << "): variable doesn't exist yet.";
	    throw error::unknown_variable(ostr.str());
	  }
	  return variables.try_emplace(var_symb,
				       root_dir, var_symb,
				       nullptr, std::nullopt, std::nullopt, false).first->second;
	}
	else
	  return it->second;
      }
      
    public:

      Center() = delete;
      Center(const Center&) = delete;
      Center& operator=(const Center&) = delete;

      Center(const fs::path& root_dir)
	: root_dir(root_dir) {
	if(!fs::exists(root_dir))
	  fs::create_directory(root_dir);
      }

      void clear() {
	std::lock_guard<std::mutex> lock(mutex);
	variables.clear();
      }

      instance_ref operator[](const symbol::Instance& var_inst) {
	std::lock_guard<std::mutex> lock(mutex);
	if(auto it = variables.find(var_inst); it != variables.end())
	  return it->second[var_inst];
	else {
	  std::ostringstream ostr;
	  ostr << "cxsom::data::Center::operator[" << var_inst << "]: variable doesn't exist yet.";
	  throw error::unknown_variable(ostr.str());
	}
      }

      type::ref type_of(const symbol::Variable& var_symb) {
	std::lock_guard<std::mutex> lock(mutex);
	return get_var(var_symb).get_type();
      }

      std::size_t history_length(const symbol::Variable& var_symb) {
	std::lock_guard<std::mutex> lock(mutex);
	return get_var(var_symb).history_length();
      }

      /**
       * The data center gets informed about all the variables
       * currently in the root directory and their type.
       */
      void check_all() {
	auto prefix_length = cxsom::symbol::parse::root_dir_length(root_dir);
	for(auto& elem: fs::recursive_directory_iterator(root_dir))
	  if(auto p = elem.path(); p.extension() == ".var")
	    touch_var(cxsom::symbol::parse::split_varpath(prefix_length, p));	    
      }
      
      /**
       * This makes the variable exist or retreive the existing
       * one. It fails if, in case of retrieval, the type is not the
       * type that we check.
       */
      void check(const symbol::Variable& var_symb,
		 type::ref type,
		 std::optional<std::size_t> cache_size,
		 std::optional<std::size_t> file_size,
		 bool kept_opened) {
	std::lock_guard<std::mutex> lock(mutex);
	if(auto it = variables.find(var_symb); it == variables.end()) {
	  auto p = var_path(var_symb);
	  auto d = p;
	  d.remove_filename();
	  if(!fs::exists(d)) fs::create_directories(d);

	  variables.try_emplace(var_symb,
				root_dir, var_symb,
				type,
				cache_size, file_size, kept_opened);
	}
	else if(auto vtype = it->second.get_type(); *type != *vtype) {
	  std::ostringstream ostr;
	  ostr << "cxsom::data::Center::check : "
	       << "Checking for " << type->name() << " while variable " << it->first << " contains " << vtype->name() << ".";
	  throw error::type_mismatch(ostr.str());
	}
      }
      
      void check(const symbol::Variable& var_symb, type::ref type) {
	check(var_symb, type, std::nullopt, std::nullopt, false);
      }
      
      void check(const symbol::Variable& var_symb, type::ref type, bool kept_opened) {
	check(var_symb, type, std::nullopt, std::nullopt, kept_opened);
      }
    };
  }
}
