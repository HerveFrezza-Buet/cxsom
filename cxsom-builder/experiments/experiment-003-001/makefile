help:
	@echo
	@echo "Read the README.txt file for instructions."
	@echo
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo
	@echo "make main-figs                                     <-- generate figures for main rules"
	@echo "make relax-figs                                    <-- generate figures for expanded relaxation rules"
	@echo "make frozen-figs                                   <-- generate figures for frozen rules"
	@echo
	@echo "make send-main-rules DURATION=<nb-steps>           <-- sends the main rules"
	@echo "make send-relax-rules TIMESTEP=<time>              <-- sends the relax rules"
	@echo "make declare-frozen-inputs                         <-- sends the rules for frozen inputs declaration"
	@echo "make send-frozen-rules TIMESTEP=<time>             <-- sends the frozen rules"
	@echo
	@echo "make feed-main-inputs SHAPE=<shape>                <-- feeds X and Y"
	@echo "make feed-relax-inputs TIMESTEP=<time>...          <-- feeds the X,Y pair for relaxation at <time>"
	@echo "     ... U=<u in [0, 1]> SHAPE=<shape>..."
	@echo "     ... [XBMU=<xbmu> YBMU=<ybmu>]"
	@echo "make feed-frozen-inputs SHAPE=<shape>              <-- feeds the X,Y pairs for frozen tests"
	@echo
	@echo "make clear-relaxation                              <--+ "
	@echo "make clear-frozen                                  <--+-- clears temporary variables"
	@echo "make clear-frozen-inputs                           <--+ "
	@echo
	@echo "make view-weights                                  <-- shows the weight evolution."
	@echo "make view-relaxation TIMESTEP=<time>               <-- shows the relaxation expansion."
	@echo "make view-frozen-inputs                            <-- shows the frozen input samples."
	@echo "make view-frozen TIMESTEP=<time> [FRAME=<id>]      <-- shows the frozen analysis."
	@echo
	@echo "make frames EVERY=50 NEXT_FRAME=0                   <-- compute the movie frames from frozen tests, 1 frame each EVERY step, (re)starting at NEXT_FRAME"
	@echo "make movie                                          <-- makes a movie from the frames."
	@echo

# Adapt this path if needed.
include /usr/share/cxsom/cxsom-makefile

RULES         = cxsom-builder-example-003-001-experiment
RELAX_PREFIX  = zrlx
FROZEN_PREFIX = zfrz
TEST_PREFIX   = test

.PHONY: main-figs
main-figs:
	@${RULES} graph main -- main 0
	@dot -Tpdf main-inits.dot -o main-inits.pdf
	@dot -Tpdf main-updates.dot -o main-updates.pdf
	@dot -Tpdf archi.dot -o archi.pdf
	@rm main-inits.dot main-updates.dot archi.dot
	@echo
	@echo
	@echo "main-inits.pdf, main-updates.pdf and archi.pdf have been generated."
	@echo

.PHONY: relax-figs
relax-figs:
	@${RULES} graph relax -- relax $(RELAX_PREFIX) 1234
	@dot -Tpdf relax-inits.dot -o relax-inits.pdf
	@dot -Tpdf relax-updates.dot -o relax-updates.pdf
	@rm relax-inits.dot relax-updates.dot 
	@echo
	@echo
	@echo "relax-inits.pdf and relax-updates.pdf have been generated."
	@echo

.PHONY: frozen-figs
frozen-figs:
	@${RULES} graph frozen -- frozen SHARED LOCAL 1234
	@dot -Tpdf frozen-inits.dot -o frozen-inits.pdf
	@dot -Tpdf frozen-updates.dot -o frozen-updates.pdf
	@rm frozen-inits.dot frozen-updates.dot 
	@echo
	@echo
	@echo "frozen-inits.pdf and frozen-updates.pdf have been generated."
	@echo

.PHONY: send-main-rules
send-main-rules:
	@${RULES} send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- main $(DURATION)

.PHONY: feed-main-inputs
feed-main-inputs:
	@python3 feed-main.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` ${SHAPE}

.PHONY: view-weights
view-weights:
	@python3 view-weights.py `cat .cxsom-rootdir-config` 

.PHONY: send-relax-rules
send-relax-rules:
	@${RULES} send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- relax $(RELAX_PREFIX)-`printf %08d ${TIMESTEP}` ${TIMESTEP}

.PHONY: feed-relax-inputs
feed-relax-inputs:
	@python3 feed-relax.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` $(RELAX_PREFIX)-`printf %08d ${TIMESTEP}` ${U} ${SHAPE} ${XBMU} ${YBMU}

.PHONY: view-relaxation
view-relaxation:
	@python3 view-relaxation.py `cat .cxsom-rootdir-config` ${TIMESTEP}

.PHONY: clear-relaxation
clear-relaxation:
	@rm -rf `cat .cxsom-rootdir-config`/zrlx-*

.PHONY: declare-frozen-inputs
declare-frozen-inputs:
	@${RULES} send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- test $(TEST_PREFIX) $(SHAPE)


.PHONY: send-frozen-rules
send-frozen-rules:
	@${RULES} send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- frozen $(TEST_PREFIX) $(FROZEN_PREFIX)-`printf %08d ${TIMESTEP}` ${TIMESTEP}

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
	@rm -rf `cat .cxsom-rootdir-config`/test-in/*

.PHONY: clear-frozen
clear-frozen:
	@rm -rf `cat .cxsom-rootdir-config`/zfrz-*

./PHONY: frames
frames:
	@python3 make-frames.py `cat .cxsom-rootdir-config` $(TEST_PREFIX) $(FROZEN_PREFIX) $(EVERY) $(NEXT_FRAME)

./PHONY: movie
movie:
	@rm -f movie-expe-003-001.ogg
	@ffmpeg -i frame-%06d.png -b:v 2M movie-expe-003-001.ogg
