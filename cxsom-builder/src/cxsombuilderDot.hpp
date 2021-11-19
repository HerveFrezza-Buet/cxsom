#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstddef>

#include <cxsom-rules.hpp>

namespace cxsom {
  namespace builder {
    
    enum class LayerKind : unsigned int {None,
					 ContextualAdaptive,
					 ContextualStatic,
					 ExternalAdaptive,
					 ExternalStatic};

    /**
     * Each node of graphviz inherits from this.
     */
    struct Dot {
      std::size_t idf;
      std::string tag;
      std::string shape;     // box, ellipse, ....
      std::string style;     // filled
      std::string fillcolor; // #rrggbb
      std::string title;
      std::string subtitle;

      Dot()                      = default;
      Dot(const Dot&)            = default;
      Dot(Dot&&)                 = default;
      Dot& operator=(const Dot&) = default;
      Dot& operator=(Dot&&)      = default;

      Dot(const std::string& tag,
	  const std::string& shape, 
	  const std::string& style,   
	  const std::string& fillcolor, 
	  const std::string& title,
	  const std::string& subtitle)
	: idf(0), tag(tag), shape(shape), style(style), fillcolor(fillcolor), title(title), subtitle(subtitle) {}

      virtual ~Dot() {}

      static std::string print_edge_label(const cxsom::rules::step& time) {
	std::ostringstream ostr;
	if(std::holds_alternative<unsigned int>(time))                    ostr << ", label=<<font point-size=\"8\"> t = " << std::get<unsigned int>(time) << "</font>>";
	else if(int dt = std::get<rules::offset>(time).value; dt != 0) {
	  if(dt > 0)                                                      ostr << ", label=<<font point-size=\"8\"> t + " <<  dt                          << "</font>>";
	  else                                                            ostr << ", label=<<font point-size=\"8\"> t - " << -dt                          << "</font>>"; 
	}
	return ostr.str();
      }
      
      static void print_edge(std::ostream& os, const cxsom::rules::step& time, LayerKind kind) {
	switch(kind) {
	case LayerKind::ContextualAdaptive : os << " [color=\"#000000\"" << print_edge_label(time) << ", style=solid ]"; break;
	case LayerKind::ContextualStatic   : os << " [color=\"#aaaaaa\"" << print_edge_label(time) << ", style=dashed]"; break;
	case LayerKind::ExternalAdaptive   : os << " [color=\"#5555aa\"" << print_edge_label(time) << ", style=solid ]"; break;
	case LayerKind::ExternalStatic     : os << " [color=\"#5555aa\"" << print_edge_label(time) << ", style=dashed]"; break;
	default:
	  break;
	}
      }
      
      static void indent(std::ostream& os, std::size_t indent_level) {
	os << std::string(2*indent_level, ' ');
      }

      void print_idf(std::ostream& os) {
	os << tag << std::right << std::setw(4) << std::setfill('0')  << idf << std::setfill(' ');
      }
      
      void print_declare(std::ostream& os, std::size_t indent_level) {
	indent(os, indent_level);
	os << std::right << std::setw(5);
	print_idf(os);
	os << " [label=<";
	bool needs_sep = false;
	
	if(title.size() > 0) {
 	  os << "<font point-size=\"10\"><b>" << title << "</b></font>";
	  needs_sep = true;
	}

	if(subtitle.size() > 0) {
	  if(needs_sep) os << "<br/>";
 	  os << "<font point-size=\"8\">" << subtitle << "</font>";
	  needs_sep = true;
	}
	os << '>';

	if(shape.size() > 0) {
	  if(needs_sep) os << ", ";
	  os << "shape=" << shape;
	  needs_sep = true;
	}
	
	if(shape.size() > 0) {
	  if(needs_sep) os << ", ";
	  os << "shape=" << shape;
	  needs_sep = true;
	}
	
	if(fillcolor.size() > 0) {
	  if(needs_sep) os << ", ";
	  os << "fillcolor=\"" << fillcolor << '\"';
	  needs_sep = true;
	}
	
	if(style.size() > 0) {
	  if(needs_sep) os << ", ";
	  os << "style=\"" << style << '\"';
	  needs_sep = true;
	}
	
	os << ']' << std::endl;
      }

      virtual void print_edges(std::ostream&) {}
    };
  }
}

