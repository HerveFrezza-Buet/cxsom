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
	@echo
	@echo "Frozen mode"
	@echo "-----------"
	@echo
	@echo "make frozen-figs"
	@echo "make declare-frozen-inputs"       
	@echo "make send-frozen-rules TIMESTEP=100"
	@echo "make feed-frozen-inputs SHAPE=banana"
	@echo "make view-frozen-inputs"
	@echo "make view-frozen TIMESTEP=<time> [FRAME=<id>]     <-- this shows the analysis."
	@echo "make clear-frozen"
	@echo "make clear-frozen-inputs"
	@echo
	@echo "Making a movie"
	@echo "--------------"
	@echo
	@echo "make frames EVERY=50 NEXT_FRAME=0                 <-- compute the movie frames from frozen tests, 1 frame each EVERY step, (re)starting at NEXT_FRAME"
	@echo "make movie                                        <-- makes a movie from the frames."
	@echo



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

FROZEN_PREFIX = zfrz
TEST_PREFIX   = ztest

.PHONY: frozen-figs
frozen-figs:
	@./xsom graph frozen -- frozen SHARED LOCAL 1234
	@dot -Tpdf frozen-inits.dot -o frozen-inits.pdf
	@dot -Tpdf frozen-updates.dot -o frozen-updates.pdf
	@rm frozen-inits.dot frozen-updates.dot 
	@echo
	@echo
	@echo "frozen-inits.pdf and frozen-updates.pdf have been generated."
	@echo

.PHONY: declare-frozen-inputs
declare-frozen-inputs:
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- test $(TEST_PREFIX)


.PHONY: send-frozen-rules
send-frozen-rules:
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- frozen $(TEST_PREFIX) $(FROZEN_PREFIX)-`printf %08d ${TIMESTEP}` ${TIMESTEP}

.PHONY: feed-frozen-inputs
feed-frozen-inputs:
	@python3 feed-frozen.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` $(TEST_PREFIX) $(SHAPE)

.PHONY: view-frozen-inputs
view-frozen-inputs:
	@python3 view-frozen-inputs.py `cat .cxsom-rootdir-config` $(TEST_PREFIX)

.PHONY: view-frozen
view-frozen:
	@python3 view-frozen.py `cat .cxsom-rootdir-config` $(TEST_PREFIX) $(FROZEN_PREFIX)-`printf %08d ${TIMESTEP}` ${TIMESTEP} ${FRAME}

.PHONY: clear-frozen-inputs
clear-frozen-inputs:
	@rm -rf `cat .cxsom-rootdir-config`/$(TEST_PREFIX)-in/*

.PHONY: clear-frozen
clear-frozen:
	@rm -rf `cat .cxsom-rootdir-config`/$(FROZEN_PREFIX)-*

./PHONY: frames
frames:
	@python3 make-frames.py `cat .cxsom-rootdir-config` $(TEST_PREFIX) $(FROZEN_PREFIX) $(EVERY) $(NEXT_FRAME)

./PHONY: movie
movie:
	@rm -f movie-expe-003-001.ogg
	@ffmpeg -i frame-%06d.png -b:v 2M movie-expe-003-001.ogg
