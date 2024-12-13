help:
	@echo
	@echo "Read the README.md file for instructions."
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo
	@echo "make clean                                         <-- removes useless generated files"
	@echo "make minmax-figs                                   <-- generate figures for main rules"
	@echo "make send-rules                                    <-- send rules to the processor"
	@echo
	@echo



# Adapt this path if needed. This adds pre-defined cxsom targets to your makefile.
include /usr/share/cxsom/cxsom-makefile


.PHONY: clean
clean:
	@rm -f *~ *.dot *.pdf

# This is how we compile rules.
minmax: minmax.cpp
	g++ -o minmax `pkg-config --cflags cxsom-builder` minmax.cpp `pkg-config --libs cxsom-builder`


.PHONY: minmax-figs
minmax-figs: minmax
	@./minmax graph minmax 
	@dot -Tpdf minmax-inits.dot -o minmax-inits.pdf
	@dot -Tpdf minmax-updates.dot -o minmax-updates.pdf
	@rm minmax-inits.dot minmax-updates.dot
	@echo
	@echo
	@echo "minmax-inits.pdf and minmax-updates.pdf have been generated."
	@echo

.PHONY: send-rules
send-rules: minmax
	@./minmax send `cat .cxsom-hostname-config` `cat .cxsom-port-config`
