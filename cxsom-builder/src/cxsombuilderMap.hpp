#pragma once

#include <string>
#include <sstream>
#include <functional>
#include <optional>
#include <list>
#include <vector>
#include <iterator>
#include <tuple>
#include <variant>

#include <cxsombuilderMain.hpp>

namespace cxsom {
  namespace builder {

    using match_func          = std::function<rules::update (const rules::kwd::data&, const rules::kwd::data&)>;
    using learn_func          = std::function<rules::update (const rules::kwd::data&, const rules::kwd::data&, const rules::kwd::data&)>;
    using argmax_func         = std::function<rules::update (const rules::kwd::data&)>;
    using toward_argmax_func  = std::function<rules::update (const rules::kwd::data&, const rules::kwd::data&)>;
    using global_merge_func   = std::function<rules::update (const rules::kwd::data&, const rules::kwd::data&)>;
    using context_merge_func  = std::function<rules::update (const std::vector<kwd::data>&)>;
    using external_merge_func = std::function<rules::update (const std::vector<kwd::data>&)>;

    using layer_input = std::variant<ref_variable, ref_map>;

    
    struct Map : public Dot {

      std::string map_name;
      std::size_t map_dim;
      std::size_t map_size;
      std::string relaxation_timeline;
      std::string weights_timeline;
      std::string output_timeline;
      std::size_t cache_size;
      std::size_t file_size;
      bool        kept_opened;
      mutable rules::kwd::parameters p_external;
      mutable rules::kwd::parameters p_contextual;
      mutable rules::kwd::parameters p_global;
      

      std::size_t bmu_file_size; //!< This can be used to set the filesize of the output variable.

      argmax_func         argmax         = fx::conv_argmax;
      toward_argmax_func  toward_argmax  = fx::toward_conv_argmax;
      global_merge_func   global_merge   = fx::merge;
      context_merge_func  context_merge  = fx::context_merge;
      external_merge_func external_merge = fx::average;

      static kwd::data dated(kwd::data d, rules::step s) {d.id.date = s; return d;}
      
      ref_variable output_BMU() const {
	std::ostringstream type;
	type << "Pos" << map_dim << 'D';
	return variable(output_timeline, name(map_name) / "BMU", type.str(), cache_size, bmu_file_size, kept_opened);
      }
      ref_variable _BMU() const {
	std::ostringstream type;
	type << "Pos" << map_dim << 'D';
	return variable(relaxation_timeline, name(map_name) / "BMU", type.str(), cache_size, 0, kept_opened);
      }
      
      ref_variable _Ac() const {
	std::ostringstream type;
	type << "Map" << map_dim << "D<Scalar>=" << map_size;
	return variable(relaxation_timeline, name(map_name) / "Ac", type.str(), cache_size, 0, kept_opened);
      }
      
      ref_variable _Ae() const {
	std::ostringstream type;
	type << "Map" << map_dim << "D<Scalar>=" << map_size;
	return variable(relaxation_timeline, name(map_name) / "Ae", type.str(), cache_size, 0, kept_opened);
      }
      
      ref_variable _A() const {
	std::ostringstream type;
	type << "Map" << map_dim << "D<Scalar>=" << map_size;
	return variable(relaxation_timeline, name(map_name) / "A", type.str(), cache_size, 0, kept_opened);
      }
      
      struct Layer {
	Map* owner;
	LayerKind kind;
	layer_input xi;
	ref_dot dot_input;
	std::string A_name;
	match_func  match;
	rules::kwd::parameters p_match;
	rules::step at_input_read;
	rules::step at_weight_read;
	bool contextual;

	Layer(Map* owner,
	      layer_input                   xi,               // The input
	      const std::string&            suffix,           // e or c
	      std::size_t                   rank,             // rank in the map layers, starting from 0
	      const match_func&             match,            // The match function
	      const rules::kwd::parameters& p_match,          // The match parameters
	      rules::step                   at_input_read,    // The time where input is get
	      rules::step                   at_weight_read)   // The time where the weight is get
	  : owner(owner),
	    kind(LayerKind::None),
	    xi(xi), dot_input(),
	    A_name(std::string("A") + suffix + "-" + std::to_string(rank)),
	    match(match),
	    p_match(p_match),
	    at_input_read(at_input_read), at_weight_read(at_weight_read),
	    contextual(false) {
	  if(std::holds_alternative<ref_variable>(xi)) dot_input = std::get<ref_variable>(xi);
	  else                                         dot_input = std::get<ref_map>(xi);
	}
	
	virtual ~Layer() {}

	// Gets the activity variable
	ref_variable _A() const {
	  std::ostringstream type;
	  type << "Map" << owner->map_dim << "D<Scalar>=" << owner->map_size;
	  return variable(owner->relaxation_timeline, name(owner->map_name) / A_name, type.str(), owner->cache_size, 0, owner->kept_opened);
	}

	// returns the weight variable.
	virtual ref_variable _W() const = 0;

	// Defines variables created by the layer.
	virtual void definitions() const {
	  _A()->definition();
	}

	// Defines variables created by the layer.
	virtual void expand_relax_definitions(const AnalysisContext& analysis) const {
	  analysis(_A())->definition();
	}

	// Defines variables created by the layer.
	virtual void frozen_definitions(const AnalysisContext& analysis) const {
	  analysis(_A())->definition();
	}

	

	// Tells which is the input.
	ref_variable _xi() const {
	  if(std::holds_alternative<ref_variable>(xi)) return std::get<ref_variable>(xi);
	  else if(contextual)                          return std::get<ref_map>(xi)->_BMU();
	  else                                         return std::get<ref_map>(xi)->output_BMU();
	}

	virtual void internals_random_at(unsigned int at) {
	  auto A = _A();
	  kwd::at(kwd::var(A->timeline, A->varname), at) << fx::random();
	}
	
	virtual void updates() const {
	  auto W  = _W();
	  auto A  = _A();
	  auto xi = _xi();
	  
	  kwd::var(A->timeline, A->varname) << match(dated(kwd::var(xi->timeline, xi->varname), at_input_read),
						     dated(kwd::var(W->timeline,  W->varname ), at_weight_read)) | p_match;
	}
	
	virtual void expand_relax_updates(const AnalysisContext& analysis) const {
	  auto W  = analysis(_W());
	  auto A  = analysis(_A());
	  auto xi = analysis(_xi());
	  if(contextual && std::holds_alternative<rules::offset>(at_input_read) && std::get<rules::offset>(at_input_read).value == 0) {
	    // If the layer is contextual, and if the input is a pattern with a current BMU.
	    kwd::var(A->timeline, A->varname) << match(kwd::prev(kwd::var(xi->timeline, xi->varname)),
						       kwd::at(kwd::var(W->timeline,  W->varname ), 0)) | p_match;

	    // As the previous generic rule involves a previous time instant, it has to be sepecialized for the 0 timestep.
	    kwd::at(kwd::var(A->timeline, A->varname), 0) << fx::clear() | kwd::use("value", 1.0);
	  }
	  else
	    kwd::var(A->timeline, A->varname) << match(kwd::at(kwd::var(xi->timeline, xi->varname), 0),
						       kwd::at(kwd::var(W->timeline,  W->varname),  0)) | p_match;

	  kwd::at(kwd::var(W->timeline,  W->varname ), 0) << fx::copy(kwd::at(kwd::var(_W()->timeline, _W()->varname), analysis.at));
	}
	
	virtual void frozen_updates(const AnalysisContext& analysis) const {
	  auto W  = analysis(_W());
	  auto A  = analysis(_A());
	  auto xi = analysis(_xi());
	  
	  kwd::var(A->timeline, A->varname) << match(dated(kwd::var(xi->timeline, xi->varname), at_input_read),
						     kwd::at(kwd::var(W->timeline,  W->varname), 0)) | p_match;
	  kwd::at(kwd::var(W->timeline,  W->varname ), 0) << fx::copy(kwd::at(kwd::var(_W()->timeline, _W()->varname), analysis.at));
	}
	
      };

      struct StaticLayer : public Layer {
	ref_variable weight;
	StaticLayer(Map* owner,
		    layer_input                   xi,   
		    const std::string&            suffix,          
		    std::size_t                   rank,             
		    const match_func&             match,           
		    const rules::kwd::parameters& p_match,          
		    rules::offset                 at_input_read,    // A relative time here !
		    unsigned int                  at_weight_read,   // An absolute time here !
		    ref_variable                  weight)           // The external weight.
	  : Layer(owner, xi, suffix, rank, match, p_match, at_input_read, at_weight_read),
	    weight(weight) {}
	virtual ~StaticLayer() {}
       
	virtual ref_variable _W() const override {return weight;}
      };

      struct AdaptiveLayer : public Layer {
	learn_func  learn;
	rules::kwd::parameters p_learn;
	rules::step at_weight_write;
	AdaptiveLayer(Map* owner,
		      layer_input                   xi,   
		      const std::string&            suffix,          
		      std::size_t                   rank,             
		      const match_func&             match,           
		      const rules::kwd::parameters& p_match,          
		      rules::offset                 at_input_read,    // A relative time here !
		      rules::offset                 at_weight_read,   // A relative time here !
		      const learn_func&             learn,            // The learning function
		      const rules::kwd::parameters& p_learn,          // The learning parameters
		      rules::offset                 at_weight_write)  // The relative time for writing the weights
	: Layer(owner, xi, suffix, rank, match, p_match, at_input_read, at_weight_read),
	  learn(learn), p_learn(p_learn), at_weight_write(at_weight_write) {}
	virtual ~AdaptiveLayer() {}
	
	virtual void updates() const override {
	  auto W   = _W();
	  auto xi  = _xi();
	  auto BMU = owner->_BMU();

	  this->Layer::updates();
	  
	  dated(kwd::var(W->timeline, W->varname), at_weight_write)
	    << learn(dated(kwd::var(xi->timeline, xi->varname), at_input_read),
		     dated(kwd::var(W->timeline,  W->varname ), at_weight_read),
		     kwd::var(BMU->timeline, BMU->varname)) | p_learn;
	}	
      };

      struct LocalAdaptiveLayer : public AdaptiveLayer {
	std::string W_name;
	LocalAdaptiveLayer(Map* owner,
			   layer_input                   xi,   
			   const std::string&            suffix,          
			   std::size_t                   rank,             
			   const match_func&             match,           
			   const rules::kwd::parameters& p_match,          
			   rules::offset                 at_input_read,  
			   rules::offset                 at_weight_read,  
			   const learn_func&             learn, 
			   const rules::kwd::parameters& p_learn,   
			   rules::offset                 at_weight_write)
	: AdaptiveLayer(owner, xi, suffix, rank, match, p_match, at_input_read, at_weight_read, learn, p_learn, at_weight_write),
	  W_name(std::string("W") + suffix + "-" + std::to_string(rank)) {}
	virtual ~LocalAdaptiveLayer() {}

	
	virtual ref_variable _W() const override {
	  std::ostringstream type;
	  type << "Map" << owner->map_dim << "D<" << _xi()->type << ">=" << owner->map_size;
	  return variable(owner->weights_timeline, name(owner->map_name) / W_name, type.str(), owner->cache_size, owner->file_size, owner->kept_opened);
	}
	
	virtual void internals_random_at(unsigned int at) override {
	  this->Layer::internals_random_at(at);
	  auto W = _W();
	  kwd::at(kwd::var(W->timeline, W->varname), at) << fx::random();
	}

	// Defines variables created by the layer.
	virtual void definitions() const override {
	  this->Layer::definitions();
	  _W()->definition();
	}
	
	// Defines variables created by the layer.
	virtual void expand_relax_definitions(const AnalysisContext& analysis) const override {
	  this->Layer::expand_relax_definitions(analysis);
	  auto W = analysis(_W());
	  W->file_size = 1;
	  W->definition();
	}  
	
	// Defines variables created by the layer.
	virtual void frozen_definitions(const AnalysisContext& analysis) const override {
	  this->Layer::frozen_definitions(analysis);
	  auto W = analysis(_W());
	  W->file_size = 1;
	  W->definition();
	}  
      };

      struct RemoteAdaptiveLayer : public AdaptiveLayer {
	ref_variable weight;
	RemoteAdaptiveLayer(Map* owner,
			    layer_input                   xi,   
			    const std::string&            suffix,          
			    std::size_t                   rank,             
			    const match_func&             match,           
			    const rules::kwd::parameters& p_match,          
			    rules::offset                 at_input_read,  
			    rules::offset                 at_weight_read,  
			    const learn_func&             learn, 
			    const rules::kwd::parameters& p_learn,   
			    rules::offset                 at_weight_write,
			    ref_variable                  weight)
	: AdaptiveLayer(owner, xi, suffix, rank, match, p_match, at_input_read, at_weight_read, learn, p_learn, at_weight_write),
	  weight(weight) {}
	virtual ~RemoteAdaptiveLayer() {}

	
	virtual ref_variable _W() const override {return weight;}
      };

      using layer_ref = std::shared_ptr<Layer>;
      std::list<layer_ref>   external_layers;
      std::list<layer_ref> contextual_layers;
      
      mutable ref_variable Ae;
      mutable ref_variable Ac;
      mutable ref_variable Ae_single;
      mutable ref_variable Ac_single;
      mutable ref_variable A;
      
      Map(const std::string& map_name, std::size_t map_dim, std::size_t map_size, std::size_t cache_size, std::size_t file_size, bool kept_opened)
	: Dot("m", "ellipse", "filled", "#ff88aa", map_name, {}),
	  map_name(map_name), map_dim(map_dim), map_size(map_size), cache_size(cache_size), file_size(file_size), kept_opened(kept_opened), bmu_file_size(0) {}

      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The variable corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
	@param at_weight_read  The offset for reading the weights.
	@param at_weight_write The offset for writing the weights.
      */
      Layer* external(ref_variable input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn,
		      rules::offset at_weight_read, rules::offset at_weight_write) {
	auto layer = new RemoteAdaptiveLayer(this, input, "e", external_layers.size(),
					     match, p_match, at_input_read, at_weight_read,
					     learn, p_learn, at_weight_write, weight);
	layer->kind = LayerKind::ExternalAdaptive;
	external_layers.emplace_back(layer);
	return layer;
      }
      
      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The variable corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* external(ref_variable input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return external(input, match, p_match, at_input_read, weight, learn, p_learn, timestep::previous(), timestep::current());
      }
      
      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The variable corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* external(ref_variable input,  const match_func& match, const rules::kwd::parameters& p_match,
		      ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return external(input, match, p_match, timestep::current(), weight, learn, p_learn);
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The variable corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading iand archi.pdf nput.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
	@param at_weight_read  The offset for reading the weights.
	@param at_weight_write The offset for writing the weights.
      */
      Layer* external(ref_variable input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      const learn_func& learn, const rules::kwd::parameters& p_learn,
		      rules::offset at_weight_read, rules::offset at_weight_write) {
	auto layer = new LocalAdaptiveLayer(this, input, "e", external_layers.size(),
					    match, p_match, at_input_read, at_weight_read,
					    learn, p_learn, at_weight_write);
	layer->kind = LayerKind::ExternalAdaptive;
	external_layers.emplace_back(layer);
	return layer;
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The variable corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* external(ref_variable input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return external(input, match, p_match, at_input_read, learn, p_learn, timestep::previous(), timestep::current());
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The variable corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* external(ref_variable input,  const match_func& match, const rules::kwd::parameters& p_match,
		      const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return external(input, match, p_match, timestep::current(), learn, p_learn);
      }

      /**
	Adds an static layer (using remotely defined weights).
	@param input           The variable corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param at_weight_read  The time for reading the weights.
      */
      Layer* external(ref_variable input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      ref_variable weight, unsigned int at_weight_read) {
	auto layer = new StaticLayer(this, input, "e", external_layers.size(), match, p_match, at_input_read, at_weight_read, weight);
	layer->kind = LayerKind::ExternalStatic;
	external_layers.emplace_back(layer);
	return layer;
      }

      /**
	Adds an static layer (using remotely defined weights).
	@param input           The variable corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param weight          The weight variable, not defined by the layer itself here.
	@param at_weight_read  The time for reading the weights.
      */
      Layer* external(ref_variable input,  const match_func& match, const rules::kwd::parameters& p_match, 
		      ref_variable weight, unsigned int  at_weight_read) {
	return external(input, match, p_match, timestep::current(), weight, at_weight_read);
      }





      

      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
	@param at_weight_read  The offset for reading the weights.
	@param at_weight_write The offset for writing the weights.
      */
      Layer* external(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn,
		      rules::offset at_weight_read, rules::offset at_weight_write) {
	auto layer = new RemoteAdaptiveLayer(this, input, "e", external_layers.size(),
					     match, p_match, at_input_read, at_weight_read,
					     learn, p_learn, at_weight_write, weight);
	layer->kind = LayerKind::ExternalAdaptive;
	external_layers.emplace_back(layer);
	return layer;
      }
      
      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* external(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return external(input, match, p_match, at_input_read, weight, learn, p_learn, timestep::previous(), timestep::current());
      }
      
      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* external(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match,
		      ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return external(input, match, p_match, timestep::current(), weight, learn, p_learn);
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
	@param at_weight_read  The offset for reading the weights.
	@param at_weight_write The offset for writing the weights.
      */
      Layer* external(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      const learn_func& learn, const rules::kwd::parameters& p_learn,
		      rules::offset at_weight_read, rules::offset at_weight_write) {
	auto layer = new LocalAdaptiveLayer(this, input, "e", external_layers.size(),
					    match, p_match, at_input_read, at_weight_read,
					    learn, p_learn, at_weight_write);
	layer->kind = LayerKind::ExternalAdaptive;
	external_layers.emplace_back(layer);
	return layer;
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* external(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return external(input, match, p_match, at_input_read, learn, p_learn, timestep::previous(), timestep::current());
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* external(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match,
		      const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return external(input, match, p_match, timestep::current(), learn, p_learn);
      }

      /**
	Adds an static layer (using remotely defined weights).
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param at_weight_read  The time for reading the weights.
      */
      Layer* external(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
		      ref_variable weight, unsigned int at_weight_read) {
	auto layer = new StaticLayer(this, input, "e", external_layers.size(), match, p_match, at_input_read, at_weight_read, weight);
	layer->kind = LayerKind::ExternalStatic;
	external_layers.emplace_back(layer);
	return layer;
      }

      /**
	Adds an static layer (using remotely defined weights).
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param weight          The weight variable, not defined by the layer itself here.
	@param at_weight_read  The time for reading the weights.
      */
      Layer* external(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, 
		      ref_variable weight, unsigned int  at_weight_read) {
	return external(input, match, p_match, timestep::current(), weight, at_weight_read);
      }










      
      
      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
	@param at_weight_read  The offset for reading the weights.
	@param at_weight_write The offset for writing the weights.
      */
      Layer* contextual(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
			ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn,
			rules::offset at_weight_read, rules::offset at_weight_write) {
	auto layer = new RemoteAdaptiveLayer(this, input, "c", contextual_layers.size(),
					     match, p_match, at_input_read, at_weight_read,
					     learn, p_learn, at_weight_write, weight);
	layer->kind = LayerKind::ContextualAdaptive;
	layer->contextual = true;
	contextual_layers.emplace_back(layer);
	return layer;
      }
      
      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* contextual(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
			ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return contextual(input, match, p_match, at_input_read, weight, learn, p_learn, timestep::previous(), timestep::current());
      }
      
      /**
	Adds an adaptive layer, using remotely defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.and archi.pdf 
	@param weight          The weight variable, not defined by the layer itself here.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* contextual(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match,
			ref_variable weight, const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return contextual(input, match, p_match, timestep::current(), weight, learn, p_learn);
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
	@param at_weight_read  The offset for reading the weights.
	@param at_weight_write The offset for writing the weights.
      */
      Layer* contextual(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
			const learn_func& learn, const rules::kwd::parameters& p_learn,
			rules::offset at_weight_read, rules::offset at_weight_write) {
	auto layer = new LocalAdaptiveLayer(this, input, "c", contextual_layers.size(),
					    match, p_match, at_input_read, at_weight_read,
					    learn, p_learn, at_weight_write);
	layer->kind = LayerKind::ContextualAdaptive;
	layer->contextual = true;
	contextual_layers.emplace_back(layer);
	return layer;
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* contextual(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
			const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return contextual(input, match, p_match, at_input_read, learn, p_learn, timestep::previous(), timestep::current());
      }

      /**
	Adds an adaptive layer, using internally defined weights.
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param learn           The learning function.
	@param p_learn         The learning parameters.
      */
      Layer* contextual(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match,
			const learn_func& learn, const rules::kwd::parameters& p_learn) {
	return contextual(input, match, p_match, timestep::current(), learn, p_learn);
      }

      /**
	Adds an static layer (using remotely defined weights).
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param at_input_read   The offset time for reading input.
	@param weight          The weight variable, not defined by the layer itself here.
	@param at_weight_read  The time for reading the weights.
      */
      Layer* contextual(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, rules::offset at_input_read,
			ref_variable weight, unsigned int at_weight_read) {
	auto layer = new StaticLayer(this, input, "c", contextual_layers.size(), match, p_match, at_input_read, at_weight_read, weight);
	layer->kind = LayerKind::ContextualStatic;
	layer->contextual = true;
	contextual_layers.emplace_back(layer);
	return layer;
      }

      /**
	Adds an static layer (using remotely defined weights).
	@param input           The map corresponding to the input.
	@param match           The match function.
	@param p_match         The match parameters.
	@param weight          The weight variable, not defined by the layer itself here.
	@param at_weight_read  The time for reading the weights.
      */
      Layer* contextual(ref_map input,  const match_func& match, const rules::kwd::parameters& p_match, 
			ref_variable weight, unsigned int  at_weight_read) {
	return contextual(input, match, p_match, timestep::current(), weight, at_weight_read);
      }

    private:

      void acts() const {

	Ae        = nullptr;
	Ac        = nullptr;
	Ae_single = nullptr;
	Ac_single = nullptr;
	A         = nullptr;
	
	switch(external_layers.size()) {
	case 0:
	  break;
	case 1:
	  Ae_single = (*(external_layers.begin()))->_A();
	  break;
	default:
	  Ae = _Ae();
	  break;
	}
	
	switch(contextual_layers.size()) {
	case 0:
	  break;
	case 1:
	  Ac_single = (*(contextual_layers.begin()))->_A();
	  break;
	default:
	  Ac = _Ac();
	  break;
	}

	if((Ae || Ae_single) && (Ac || Ac_single))
	  A = _A();
      }
      
    public:

      void definitions() const {
	_BMU()->definition();
	output_BMU()->definition();
	
	for(auto& ext : external_layers) ext->definitions();
	for(auto& ctx : contextual_layers) ctx->definitions();

	acts();

	if(Ae) Ae->definition();
	if(Ac) Ac->definition();
	if(A)  A->definition();
      }

      void expand_relax_definitions(const AnalysisContext& analysis) const {
	analysis(_BMU())->definition();
	
	for(auto& ext : external_layers) ext->expand_relax_definitions(analysis);
	for(auto& ctx : contextual_layers) ctx->expand_relax_definitions(analysis);

	acts();

	if(Ae) analysis(Ae)->definition();
	if(Ac) analysis(Ac)->definition();
	if(A ) analysis(A )->definition();
      }

      void frozen_definitions(const AnalysisContext& analysis) const {
	analysis(_BMU())->definition();
	analysis(output_BMU())->definition();
	
	for(auto& ext : external_layers) ext->frozen_definitions(analysis);
	for(auto& ctx : contextual_layers) ctx->frozen_definitions(analysis);

	acts();

	if(Ae) analysis(Ae)->definition();
	if(Ac) analysis(Ac)->definition();
	if(A ) analysis(A )->definition();
      }

      void internals_random_at(unsigned int at) {
	for(auto& ext : external_layers)   ext->internals_random_at(at);
	for(auto& ctx : contextual_layers) ctx->internals_random_at(at);

	if(external_layers.size() > 1) {
	  auto Ae = _Ae();
	  kwd::at(kwd::var(Ae->timeline, Ae->varname), at) << fx::random();
	}
	
	if(contextual_layers.size() > 1) {
	  auto Ac = _Ac();
	  kwd::at(kwd::var(Ac->timeline, Ac->varname), at) << fx::random();
	}
	
	if(external_layers.size() > 0 && contextual_layers.size() > 0) {
	  auto A = _A();
	  kwd::at(kwd::var(A->timeline, A->varname), at) << fx::random();
	}

	auto BMU = _BMU();
	kwd::at(kwd::var(BMU->timeline, BMU->varname), at) << fx::random();
	BMU = output_BMU();
	kwd::at(kwd::var(BMU->timeline, BMU->varname), at) << fx::random();
      }
      
      void updates() const {
	ref_variable BMU    = _BMU();
	ref_variable output = output_BMU();
	

	// std::cout << "####" << std::endl;
	// if(Ac) std::cout << "Ac = " << kwd::var(Ac->timeline, Ac->varname) << std::endl;
	// else   std::cout << "Ac = nil" << std::endl;
	// if(Ae) std::cout << "Ae = " << kwd::var(Ae->timeline, Ae->varname) << std::endl;
	// else   std::cout << "Ae = nil" << std::endl;
	// if(A ) std::cout << "A  = " << kwd::var(A->timeline, A->varname) << std::endl;
	// else   std::cout << "A  = nil" << std::endl;
	
	// Let us merge the externals.

	if(Ae) {
	  std::vector<kwd::data> args;
	  auto out_args = std::back_inserter(args);
	  for(auto& ext : external_layers) {
	    ext->updates();
	    auto a = ext->_A();
	    *(out_args++) = kwd::var(a->timeline, a->varname);
	  }
	  kwd::var(Ae->timeline, Ae->varname) << external_merge(args) | p_external;
	}
	else if(Ae_single) {
	  (*(external_layers.begin()))->updates();
	  Ae = Ae_single;
	}
		
	// Let us merge the contextuals.

	if(Ac) {
	  std::vector<kwd::data> args;
	  auto out_args = std::back_inserter(args);
	  for(auto& ctx : contextual_layers) {
	    ctx->updates();
	    auto a = ctx->_A();
	    *(out_args++) = kwd::var(a->timeline, a->varname);
	  }
	  kwd::var(Ac->timeline, Ac->varname) << external_merge(args) | p_contextual;
	}
	else if(Ac_single) {
	  (*(contextual_layers.begin()))->updates();
	  Ac = Ac_single;
	}

	// Let us merge both.
	
	if(A) // We have both, let us merge them.
	  kwd::var(A->timeline, A->varname) << global_merge(kwd::var(Ae->timeline, Ae->varname), kwd::var(Ac->timeline, Ac->varname)) | p_global;
	else if(Ae)
	  A = Ae;
	else if(Ac)
	  A = Ac;

	// Let us compute the BMU.
	if(A) {
	  if(Ac) {
	    if(Ae)
	      (kwd::var(BMU->timeline, BMU->varname) <= argmax(kwd::var(Ae->timeline, Ae->varname))) | p_global;
	    kwd::var(BMU->timeline, BMU->varname) << toward_argmax(kwd::var(A->timeline, A->varname), kwd::var(BMU->timeline, BMU->varname)) | p_global;
	  }
	  else
	    kwd::var(BMU->timeline, BMU->varname) << argmax(kwd::var(A->timeline, A->varname)) | p_global;
	
	  kwd::var(output->timeline, output->varname) << fx::copy(kwd::var(BMU->timeline, BMU->varname)) | p_global;
	}
      }

      void expand_relax_updates(const AnalysisContext& analysis) const {
	double walltime = (double)(analysis.file_size) - 1;
	p_external   | kwd::use("walltime", walltime);
	p_contextual | kwd::use("walltime", walltime);
	p_global     | kwd::use("walltime", walltime);
	
	ref_variable BMU    = analysis(_BMU());
		
	// Let us merge the externals.

	if(Ae) {
	  std::vector<kwd::data> args;
	  auto out_args = std::back_inserter(args);
	  for(auto& ext : external_layers) {
	    ext->p_match | kwd::use("walltime", walltime);
	    ext->expand_relax_updates(analysis);
	    auto a = analysis(ext->_A());
	    *(out_args++) = kwd::var(a->timeline, a->varname);
	  }
	  auto AAe = analysis(Ae);
	  kwd::var(AAe->timeline, AAe->varname) << external_merge(args) | p_external;
	}
	else if(Ae_single) {
	  auto& ext = (*(external_layers.begin()));
	  ext->p_match | kwd::use("walltime", walltime);
	  ext->expand_relax_updates(analysis);
	  Ae = Ae_single;
	}
		
	// Let us merge the contextuals.

	if(Ac) {
	  std::vector<kwd::data> args;
	  auto out_args = std::back_inserter(args);
	  for(auto& ctx : contextual_layers) {
	    ctx->p_match | kwd::use("walltime", walltime);
	    ctx->expand_relax_updates(analysis);
	    auto a = analysis(ctx->_A());
	    *(out_args++) = kwd::var(a->timeline, a->varname);
	  }
	  auto AAc = analysis(Ac);
	  kwd::var(AAc->timeline, AAc->varname) << external_merge(args) | p_contextual;
	}
	else if(Ac_single) {
	  auto& ctx = (*(contextual_layers.begin()));
	  ctx->p_match | kwd::use("walltime", walltime);
	  ctx->expand_relax_updates(analysis);
	  Ac = Ac_single;
	}

	// Let us merge both.
	
	if(A) {// We have both, let us merge them.
	  auto AA  = analysis(A);
	  auto AAe = analysis(Ae);
	  auto AAc = analysis(Ac);
	  kwd::var(AA->timeline, AA->varname) << global_merge(kwd::var(AAe->timeline, AAe->varname), kwd::var(AAc->timeline, AAc->varname)) | p_global;
	}
	else if(Ae)
	  A = Ae;
	else if(Ac)
	  A = Ac;

	// Let us compute the BMU.
	if(A) {
	  auto AA  = analysis(A);
	  if(Ac) 
	    kwd::var(BMU->timeline, BMU->varname) << toward_argmax(kwd::var(AA->timeline, AA->varname), kwd::prev(kwd::var(BMU->timeline, BMU->varname))) | p_global;
	  else
	    kwd::var(BMU->timeline, BMU->varname) << argmax(kwd::var(AA->timeline, AA->varname)) | p_global;
	
	  // kwd::var(output->timeline, output->varname) << fx::copy(kwd::var(BMU->timeline, BMU->varname)) | p_global;
	}
      }
      
      void frozen_updates(const AnalysisContext& analysis) const {
	double walltime = (double)(analysis.file_size) - 1;
	p_external   | kwd::use("walltime", walltime);
	p_contextual | kwd::use("walltime", walltime);
	p_global     | kwd::use("walltime", walltime);
	
	ref_variable BMU    = analysis(_BMU());
	ref_variable output = analysis(output_BMU());

	if(Ae) {
	  std::vector<kwd::data> args;
	  auto out_args = std::back_inserter(args);
	  for(auto& ext : external_layers) {
	    ext->p_match | kwd::use("walltime", walltime);
	    ext->frozen_updates(analysis);
	    auto a = analysis(ext->_A());
	    *(out_args++) = kwd::var(a->timeline, a->varname);
	  }
	  auto AAe = analysis(Ae);
	  kwd::var(AAe->timeline, AAe->varname) << external_merge(args) | p_external;
	}
	else if(Ae_single) {
	  auto& ext = (*(external_layers.begin()));
	  ext->p_match | kwd::use("walltime", walltime);
	  ext->frozen_updates(analysis);
	  Ae = Ae_single;
	}
		
	// Let us merge the contextuals.

	if(Ac) {
	  std::vector<kwd::data> args;
	  auto out_args = std::back_inserter(args);
	  for(auto& ctx : contextual_layers) {
	    ctx->p_match | kwd::use("walltime", walltime);
	    ctx->frozen_updates(analysis);
	    auto a = analysis(ctx->_A());
	    *(out_args++) = kwd::var(a->timeline, a->varname);
	  }
	  auto AAc = analysis(Ac);
	  kwd::var(AAc->timeline, AAc->varname) << external_merge(args) | p_contextual;
	}
	else if(Ac_single) {
	  auto& ctx = (*(contextual_layers.begin()));
	  ctx->p_match | kwd::use("walltime", walltime);
	  ctx->frozen_updates(analysis);
	  Ac = Ac_single;
	}

	// Let us merge both.
	
	if(A) {// We have both, let us merge them.
	  auto AA  = analysis(A);
	  auto AAe = analysis(Ae);
	  auto AAc = analysis(Ac);
	  kwd::var(AA->timeline, AA->varname) << global_merge(kwd::var(AAe->timeline, AAe->varname), kwd::var(AAc->timeline, AAc->varname)) | p_global;
	}
	else if(Ae)
	  A = Ae;
	else if(Ac)
	  A = Ac;

	// Let us compute the BMU.
	if(A) {
	  auto AA  = analysis(A);
	  if(Ac) {
	    if(Ae) {
	      auto AAe = analysis(Ae);
	      (kwd::var(BMU->timeline, BMU->varname) <= argmax(kwd::var(AAe->timeline, AAe->varname))) | p_global;
	    }
	    kwd::var(BMU->timeline, BMU->varname) << toward_argmax(kwd::var(AA->timeline, AA->varname), kwd::var(BMU->timeline, BMU->varname)) | p_global;
	  }
	  else
	    kwd::var(BMU->timeline, BMU->varname) << argmax(kwd::var(AA->timeline, AA->varname)) | p_global;
	
	  kwd::var(output->timeline, output->varname) << fx::copy(kwd::var(BMU->timeline, BMU->varname)) | p_global;
	}
      }


      



      

      void operator=(const std::tuple<kwd::parameters, kwd::parameters, kwd::parameters>& params) {
	std::tie(p_external, p_contextual, p_global) = params;
      }
    };

    inline auto map1D(const std::string& map_name, std::size_t map_size, std::size_t cache_size, std::size_t file_size, bool kept_opened) {return std::make_shared<Map>(map_name, 1, map_size, cache_size, file_size, kept_opened);}
    inline auto map2D(const std::string& map_name, std::size_t map_size, std::size_t cache_size, std::size_t file_size, bool kept_opened) {return std::make_shared<Map>(map_name, 2, map_size, cache_size, file_size, kept_opened);}

  }
}
