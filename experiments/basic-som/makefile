help:
	@echo
	@echo "Read the README.md file for instructions."
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo
	@echo "make clean                                         <-- removes useless generated files"
	@echo "make som-figs                                      <-- generate figures for main rules"
	@echo "make send-rules                                    <-- send rules to the processor"
	@echo
	@echo "make feed-square                                   <-- add new inputs taken in a square"
	@echo "make feed-crown                                    <-- add new inputs taken in a crown"
	@echo "make feed-triangle                                 <-- add new inputs taken in a triangle"
	@echo
	@echo "make view-last-weights                             <-- shows the last weights (from wgt)"
	@echo "make view-weights-history                          <-- shows the weights      (from save)"
	@echo
	@echo



# Adapt this path if needed. This adds pre-defined cxsom targets to your makefile.
include /usr/share/cxsom/cxsom-makefile


.PHONY: clean
clean:
	@rm -f *~ *.dot *.pdf som monitoring.data monitoring.md

# This is how we compile rules.
som: som.cpp
	g++ -o som `pkg-config --cflags cxsom-builder` som.cpp `pkg-config --libs cxsom-builder`


.PHONY: som-figs
som-figs: som
	@./som graph som 
	@dot -Tpdf som-inits.dot -o som-inits.pdf
	@dot -Tpdf som-updates.dot -o som-updates.pdf
	@dot -Tpdf som-archi.dot -o som-archi.pdf
	@rm som-inits.dot som-updates.dot som-archi.dot
	@echo
	@echo
	@echo "som-inits.pdf, som-updates.pdf and som-archi.pdf have been generated."
	@echo

.PHONY: send-rules
send-rules: som
	@./som send `cat .cxsom-hostname-config` `cat .cxsom-port-config`

.PHONY: feed-square
feed-square:
	@python feed.py square `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config`

.PHONY: feed-crown
feed-crown:
	@python feed.py crown `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` 

.PHONY: feed-triangle
feed-triangle:
	@python feed.py triangle `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` 

.PHONY: view-last-weights
view-last-weights:
	@python view.py `cat .cxsom-rootdir-config` wgt &

.PHONY: view-weights-history
view-weights-history:    
	@python view.py `cat .cxsom-rootdir-config` save &
