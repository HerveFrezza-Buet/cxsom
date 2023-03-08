help:
	@echo
	@echo "Read the README.txt file for instructions."
	@echo
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations."
	@echo
	@echo "make input-setup                                   <-- sets up the input generator (sets the image and sends the rules)."
	@echo "make send-input-rules                              <-- sets up the input rules only (in case of restart, for example)."
	@echo "make feed WALLTIME=500                             <-- feed inputs."
	@echo "make show-inputs                                   <-- shows current inputs."
	@echo
	@echo "make show-train-archi                              <-- displays the train architecture."
	@echo "make send-train-rules SAVE_PERIOD=10               <-- sends training rules."
	@echo "make show-weights-history                          <-- displays the saved weights."
	@echo "make show-rgb-mapping                              <-- displays the color map encoding by the RGB map."
	@echo
	@echo "make re-train WALLTIME=3000 SAVE_PERIOD=10         <-- restarts from scratch everything."
	@echo
	@echo "make show-test-archi                               <-- displays the test architecture."
	@echo "make prediction-inputs-setup                       <-- builds up the inputs for building up the predicted image."
	@echo "make send-test-rules WEIGHTS_AT=100                <-- sends testing rules (for saved weights at 100)."
	@echo "make clear-test                                    <-- remove the test variables"
	@echo "                                                       (clear or restart the processor first)."
	@echo
	@echo

# Adapt this path if needed.
include /usr/share/cxsom/cxsom-makefile

.PHONY: send-input-rules
send-input-rules:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- input

.PHONY: input-setup
input-setup:
	@make --quiet send-input-rules
	@python3 build-eye-input.py `cat .cxsom-rootdir-config`
	@cxsom-ping `cat .cxsom-hostname-config` `cat .cxsom-port-config`

.PHONY: feed
feed:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- walltime ${WALLTIME}

.PHONY: show-inputs
show-inputs:
	@python3 show-inputs.py `cat .cxsom-rootdir-config`

.PHONY: show-train-archi
show-train-archi:
	@cxsom-builder-example-003-002-experiment graph-full train -- train 100
	@dot -Tpdf train.dot -o train.pdf
	@sfdp -Tpdf -Goverlap=prism train-updates.dot -o train-updates.pdf
	@sfdp -Tpdf -Goverlap=prism train-inits.dot -o train-inits.pdf
	@rm -f train.dot train-updates.dot train-inits.dot
	@evince train.pdf train-updates.pdf train-inits.pdf &

.PHONY: send-train-rules
send-train-rules:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- train ${SAVE_PERIOD}
	@cxsom-ping `cat .cxsom-hostname-config` `cat .cxsom-port-config`

.PHONY: show-weights-history
show-weights-history:
	@python3 show-weights-history.py `cat .cxsom-rootdir-config`

.PHONY: show-rgb-mapping
show-rgb-mapping:
	@python3 show-rgb-mapping.py `cat .cxsom-rootdir-config`


.PHONY: re-train
re-train:
	@make --quiet cxsom-clear-rootdir cxsom-kill-processor cxsom-launch-processor cxsom-is-processor-running
	@make --quiet input-setup send-train-rules feed

.PHONY: show-test-archi
show-test-archi:
	@cxsom-builder-example-003-002-experiment graph test -- test 100
	@dot -Tpdf test.dot -o test.pdf
	@dot -Tpdf test-updates.dot -o test-updates.pdf
	@dot -Tpdf test-inits.dot -o test-inits.pdf
	@rm -f test.dot test-updates.dot test-inits.dot
	@evince test.pdf test-updates.pdf test-inits.pdf &

.PHONY: prediction-inputs-setup
prediction-inputs-setup:
	@python3 build-prediction-inputs.py `cat .cxsom-rootdir-config`

.PHONY: send-test-rules
send-test-rules:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- test ${WEIGHTS_AT}
	@cxsom-ping `cat .cxsom-hostname-config` `cat .cxsom-port-config`

.PHONY: clear-test
clear-test:
	@rm -rf `cat .cxsom-rootdir-config`/test-*
	@rm -rf `cat .cxsom-rootdir-config`/prediction-output/*


