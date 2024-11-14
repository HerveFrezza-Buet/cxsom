help:
	@echo
	@echo "Read the README.md file for instructions."
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo "make clean                                         <-- removes useless generated files"
	@echo
	@echo "Main mode"
	@echo "---------"
	@echo
	@echo "make main-figs                                     <-- generate figures for main rules"
	@echo "make send-main-rules TRACE=2500                    <-- send the rules (defining the trace size)."
	@echo "make feed-main SHAPE=banana                        <-- feed inputs."
	@echo "make view-weights"
	@echo
	@echo "Relax mode"
	@echo "----------"
	@echo
	@echo "make relax-figs"	
	@echo "make send-relax-rules  TIMESTEP=100"
	@echo "make feed-relax-inputs TIMESTEP=100 U=.5 SHAPE=banana XBMU=.5 YBMU=.5"	
	@echo "make view-relaxation   TIMESTEP=100"
	@echo "make clear-relaxation                             <-- removes all rlx timelines from rootdir."



# Adapt this path if needed. This adds pre-defined cxsom targets to your makefile.
include /usr/share/cxsom/cxsom-makefile


.PHONY: clean
clean:
	@rm -f *~ *.dot *.pdf

# This is how we compile rules.
xsom: xsom.cpp
	g++ -o xsom `pkg-config --cflags cxsom-builder` xsom.cpp `pkg-config --libs cxsom-builder`


.PHONY: main-figs
main-figs: xsom
	@./xsom graph main -- main 0
	@dot -Tpdf main-inits.dot -o main-inits.pdf
	@dot -Tpdf main-updates.dot -o main-updates.pdf
	@dot -Tpdf archi.dot -o archi.pdf
	@rm main-inits.dot main-updates.dot archi.dot
	@echo
	@echo
	@echo "main-inits.pdf, main-updates.pdf and archi.pdf have been generated."
	@echo

.PHONY: send-main-rules
send-main-rules:
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- main $(TRACE)

.PHONY: feed-main
feed-main:
	@python3 feed-main.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` ${SHAPE}

.PHONY: view-weights
view-weights:
	@python3 view-weights.py `cat .cxsom-rootdir-config` 

RELAX_PREFIX  = zrlx

.PHONY: relax-figs
relax-figs:
	@./xsom graph relax -- relax $(RELAX_PREFIX) 1234
	@dot -Tpdf relax-inits.dot -o relax-inits.pdf
	@dot -Tpdf relax-updates.dot -o relax-updates.pdf
	@rm relax-inits.dot relax-updates.dot 
	@echo
	@echo
	@echo "relax-inits.pdf and relax-updates.pdf have been generated."
	@echo


.PHONY: send-relax-rules
send-relax-rules:
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- relax $(RELAX_PREFIX)-`printf %08d ${TIMESTEP}` ${TIMESTEP}

.PHONY: feed-relax-inputs
feed-relax-inputs:
	@python3 feed-relax.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` $(RELAX_PREFIX)-`printf %08d ${TIMESTEP}` ${U} ${SHAPE} ${XBMU} ${YBMU}

.PHONY: view-relaxation
view-relaxation:
	@python3 view-relaxation.py `cat .cxsom-rootdir-config` ${TIMESTEP}

.PHONY: clear-relaxation
clear-relaxation:
	@rm -rf `cat .cxsom-rootdir-config`/zrlx-*
