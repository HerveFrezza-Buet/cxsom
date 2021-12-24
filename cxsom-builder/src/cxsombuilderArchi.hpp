#pragma once

#include <cstddef>
#include <memory>
#include <set>
#include <map>
#include <utility>
#include <iostream>

#include <cxsombuilderForward.hpp>
#include <cxsombuilderMain.hpp>
#include <cxsombuilderMap.hpp>
#include <tuple>

namespace cxsom {
  namespace builder {
    struct Architecture {
      std::size_t next_free_idf = 0;

      std::set<ref_map>                                  maps;
      std::set<ref_dot>                                  vertices;
      std::map<std::tuple<ref_dot, ref_dot, LayerKind>, rules::step> edges;


      struct WriteDot {
	Architecture* owner;
	WriteDot(Architecture* owner) : owner(owner) {}
      };

      struct Timelines {
	std::string weights;
	std::string relaxation;
	std::string output;
	Timelines(const std::string& weights, const std::string& relaxation, const std::string& output)
	  : weights(weights), relaxation(relaxation), output(output) {}
      };
	
      WriteDot write_dot;
      Timelines timelines;
      std::optional<std::string> relax_count; //!< Set this optional to the name of the convergence count variable for adding the counting rule.

      Architecture() : write_dot(this), timelines("wgt", "rlx", "out") {}

      void operator<<(ref_map m) {
	vertices.insert(m);
	maps.insert(m);

	for(auto& ext : m->external_layers) {
	  ref_dot d1 = ext->dot_input;
	  vertices.insert(d1);
	  ref_dot d2 = m;
	  edges[{d1, d2, ext->kind}] = ext->at_input_read;
	}
	
	for(auto& ctx : m->contextual_layers) {
	  ref_dot d1 = ctx->dot_input;
	  ref_dot d2 = m;
	  edges[{d1, d2, ctx->kind}] = ctx->at_input_read;
	}

	m->weights_timeline    = timelines.weights;
	m->relaxation_timeline = timelines.relaxation;
	m->output_timeline     = timelines.output;
      }

      void realize() const {
	for(auto m : maps) m->definitions();
	for(auto m : maps) m->updates();
	
	if(relax_count) {
	  std::vector<kwd::data> bmus;
	  for(auto m : maps) {
	    auto BMU = m->_BMU();
	    bmus.push_back(kwd::var(BMU->timeline, BMU->varname));
	  }
	  kwd::var(timelines.relaxation, *relax_count) << fx::converge(bmus);
	}
      }
    };

    inline auto architecture() {return std::make_shared<Architecture>();}
    inline ref_architecture operator<<(ref_architecture a, ref_map m) {
      *a << m;
      return a;
    }
    
    inline std::ostream& operator<<(std::ostream& os, Architecture::WriteDot& write_dot) {
      auto that = write_dot.owner;
      that->next_free_idf = 0;
      for(auto v : that->vertices) v->idf = (that->next_free_idf)++;

      os << "digraph All {" << std::endl
	 << std::endl;

      for(auto v : that->vertices) v->print_declare(os, 1);
      os << std::endl;

      for(auto& abt : that->edges) {
	auto [a, b, k] = abt.first;
	Dot::indent(os, 1);
	a->print_idf(os);
	os << " -> ";
	b->print_idf(os);
	Dot::print_edge(os, abt.second, k);	
	os << std::endl;
      }
      os << std::endl;

      
      os << '}' << std::endl;
      return os;
    }
  }
}
