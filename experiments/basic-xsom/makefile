help:
	@echo
	@echo "Read the README.md file for instructions."
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo
	@echo "make clean                                         <-- removes useless generated files"
	@echo "make xsom-figs                                     <-- generate figures for main rules"
	@echo "make send-rules                                    <-- send rules to the processor"
	@echo
	@echo



# Adapt this path if needed. This adds pre-defined cxsom targets to your makefile.
include /usr/share/cxsom/cxsom-makefile


.PHONY: clean
clean:
	@rm -f *~ *.dot *.pdf

# This is how we compile rules.
xsom: xsom.cpp
	g++ -o xsom `pkg-config --cflags cxsom-builder` xsom.cpp `pkg-config --libs cxsom-builder`


.PHONY: xsom-figs
xsom-figs: xsom
	@./xsom graph xsom 
	@dot -Tpdf xsom-inits.dot -o xsom-inits.pdf
	@dot -Tpdf xsom-updates.dot -o xsom-updates.pdf
	@dot -Tpdf xsom-archi.dot -o xsom-archi.pdf
	@rm xsom-inits.dot xsom-updates.dot xsom-archi.dot
	@echo
	@echo
	@echo "xsom-inits.pdf, xsom-updates.pdf and xsom-archi.pdf have been generated."
	@echo

.PHONY: send-rules
send-rules: xsom
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config`
