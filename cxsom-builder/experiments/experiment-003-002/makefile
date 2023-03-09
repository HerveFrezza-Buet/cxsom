help:
	@echo
	@echo "Read the README.txt file for instructions."
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations."
	@echo
	@echo "# Inputs mode"
	@echo 
	@echo "make inputs-setup IMAGE_SIDE=100                   <-- sets up inputs."
	@echo "make show-samples                                  <-- shows the image check/test samples"
	@echo "make clear-samples                                 <-- deletes the samples."
	@echo
	@echo "# Train mode"
	@echo
	@echo "make show-train-rules                              <-- Shows how training is computed."
	@echo "make train-setup SAVE_PERIOD=100 IMAGE_SIDE=100    <-- sets up training rules, saving weights periodically."
	@echo "make feed-train-inputs WALLTIME=50000              <-- feed inputs for training."
	@echo "make show-weights-history       	                  <-- displays the saved weights."
	@echo "make show-rgb-mapping                              <-- displays the color map encoding by the RGB map."
	@echo "make clear-training                                <-- clears training variables (training cannot be continued further)."
	@echo "make clear-saved-weights                           <-- Danger zone ! You will loose the training result."
	@echo
	@echo "# Predict mode"
	@echo
	@echo "make show-predict-rules                            <-- Shows how prediction is computed."
	@echo "make predict WEIGHTS_AT=500 IMAGE_SIDE=100         <-- sends testing rules (for saved weights at 500)."
	@echo "                                                       (clear or restart the processor first)."
	@echo "make show-predictions                              <-- Shows the rgb predictions."
	@echo "make clear-predictions                             <-- clears prediction."
	@echo

# Adapt this path if needed.
include /usr/share/cxsom/cxsom-makefile


.PHONY: inputs-setup
inputs-setup:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- input ${IMAGE_SIDE}
	@python3 build-eye-input.py `cat .cxsom-rootdir-config`
	@python3 build-img-coords.py `cat .cxsom-rootdir-config` img ${IMAGE_SIDE}
	@make --quiet cxsom-ping-processor

.PHONY: show-samples
show-samples:
	@python3 show-samples.py `cat .cxsom-rootdir-config` img img

.PHONY: clear-samples
clear-samples:
	@rm -rf `cat .cxsom-rootdir-config`/img

.PHONY: train-setup
train-setup:
	@@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- train ${SAVE_PERIOD} ${IMAGE_SIDE}


.PHONY: show-train-rules
show-train-rules:
	@cxsom-builder-example-003-002-experiment graph-full train -- train 100 100
	@dot -Tpdf train.dot -o train.pdf
	@sfdp -Tpdf -Goverlap=prism train-updates.dot -o train-updates.pdf
	@sfdp -Tpdf -Goverlap=prism train-inits.dot -o train-inits.pdf
	@rm -f train.dot train-updates.dot train-inits.dot
	@evince train.pdf train-updates.pdf train-inits.pdf &

.PHONY: feed-train-inputs
feed-train-inputs:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- walltime ${WALLTIME}
	@make --quiet cxsom-ping-processor

.PHONY: show-weights-history
show-weights-history:
	@python3 show-weights-history.py `cat .cxsom-rootdir-config`

.PHONY: show-rgb-mapping
show-rgb-mapping:
	@python3 show-rgb-mapping.py `cat .cxsom-rootdir-config`

.PHONY: clear-training
clear-training:
	@rm -rf `cat .cxsom-rootdir-config`/train-*

.PHONY: clear-saved-weights
clear-saved-weights:
	@rm -rf `cat .cxsom-rootdir-config`/saved

.PHONY: show-predict-rules
show-predict-rules:
	@cxsom-builder-example-003-002-experiment graph predict -- predict 100
	@dot -Tpdf predict.dot -o predict.pdf
	@dot -Tpdf predict-updates.dot -o predict-updates.pdf
	@dot -Tpdf predict-inits.dot -o predict-inits.pdf
	@rm -f predict.dot predict-updates.dot predict-inits.dot
	@evince predict.pdf predict-updates.pdf predict-inits.pdf &

.PHONY: clear-predict
clear-predict:
	@rm -rf `cat .cxsom-rootdir-config`/predict-*

.PHONY: predict
predict:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- predict ${WEIGHTS_AT} ${IMAGE_SIDE}


.PHONY: show-predictions
show-predictions:
	@python3 show-samples.py `cat .cxsom-rootdir-config` img predict-out
